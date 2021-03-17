/*
 Copyright (c) 2021, The Cinder Project, All rights reserved.

 This code is intended for use with the Cinder C++ library: http://libcinder.org

 Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 the following conditions are met:

	* Redistributions of source code must retain the above copyright notice, this list of conditions and
	   the following disclaimer.
	* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
	   the following disclaimer in the documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
*/

#include "cinder/Log.h"
#include "cinder/graphics/DrawContext.h"
#include "cinder/graphics/Texture.h"
#include "cinder/app/RendererGx.h"

#if defined( IMGUI_DEGUG )
#include "cinder/CinderImGui.h"
#endif

#include <functional>
#include <algorithm>

using namespace std;

namespace cinder { namespace graphics {

DrawContext::DrawContext() 
    : mIndexBufferSize( 0 ),
	mVertexBufferSize( 0 ),
	mConstantsBufferSize( 0 ),
	mVertexIndex( 0 ),
	mConstantIndex( 0 ),
	mConstantCount( 1 ),
	mColor( ColorAf::white() ),
	mTransformValid( false ),
	mStateValid( false ),
	mViewportValid( false ),
	mScissorValid( false ),
	mResourcesValid( false ),
	mGeomBuffersValid( false ),
	mConstantBufferValid( false ),
	mSRBsValid( false ),
	mPSOsValid( false ),
	mBindlessResources( true ),
	mVerifyDeviceFeatures( true ),
	mGeomBuffersImmutable( true ),
	mConstantBufferImmutable( true ),
	mTextureIndex( 0 ),
	mTextureCount( 1 ),
	// TODO/NOTES: Currently unbounded arrays of Texture seem to be disabled by default in the HLSL compiler as 
	// they are known to cause issues with graphics / frame capture tools. Instead of depending on those
	// it might be a good idea to lower the maximum number of textures and ensure that commands stop being 
	// merged once that maximum is reached. Currently no such check exists.
	// see: https://docs.microsoft.com/en-us/windows/win32/direct3d12/resource-binding-in-hlsl#resource-types-and-arrays
	// https://docs.microsoft.com/en-us/windows/win32/direct3d12/dynamic-indexing-using-hlsl-5-1
	// https://alextardif.com/Bindless.html
	// https://github.com/TheRealMJP/DeferredTexturing
	// http://roar11.com/2019/06/vulkan-textures-unbound/
	mMaxBindlessTextures( 64 )
{
	mModelMatrixStack.push_back( mat4() );
	mViewMatrixStack.push_back( mat4() );
	mProjectionMatrixStack.push_back( mat4() );

	mVertices.resize( 64 );
	mIndices.resize( 16 );
	mConstants.resize( 1 );
	mTextures.resize( mMaxBindlessTextures );

	mVertex = mVertices.data();
	mIndex = mIndices.data();
}

DrawContext::Pipeline DrawContext::initializePipelineState( RenderDevice* device, const State &state )
{
	gx::ShaderMacroHelper bindlessMacro;
	bindlessMacro.AddShaderMacro( "BINDLESS_RESOURCES", 1 );
	bindlessMacro.AddShaderMacro( "NUM_TEXTURES", mMaxBindlessTextures );

	string vertexShader = R"( #line 94

		struct Constant {
			float4x4 transform;
		};

		StructuredBuffer<Constant> constantBuffer;
 
		struct VSInput {
			float3 position : ATTRIB0;
			float2 uv		: ATTRIB1;
			float4 color    : ATTRIB2;
			uint constant	: ATTRIB3;
			uint textureId	: ATTRIB4;
		};

		struct PSInput { 
			float4 position : SV_POSITION; 
			float4 color    : COLOR0; 
			float2 uv		: TEX_COORD;
			uint textureId	: TEX_ARRAY_INDEX;
		};
 
		void main( in VSInput vsIn, out PSInput psIn ) 
		{
			const float4x4 transform = constantBuffer[vsIn.constant].transform;
			psIn.position  = mul( float4( vsIn.position, 1.0f ), transform );
			psIn.color     = vsIn.color;
			psIn.uv		 = vsIn.uv;
			psIn.textureId = vsIn.textureId;
		}
	)";

	string pixelShader = R"( #line 127

		#ifdef BINDLESS_RESOURCES
			Texture2D    rTexture[NUM_TEXTURES];
		#else
			Texture2D    rTexture;
		#endif
			SamplerState rTexture_sampler;

			struct PSInput { 
				float4 position : SV_POSITION; 
				float4 color    : COLOR0; 
				float2 uv		: TEX_COORD;
				uint textureId	: TEX_ARRAY_INDEX;
			};

			struct PSOutput { 
				float4 color : SV_TARGET; 
			};
                
			void main( in PSInput psIn, out PSOutput psOut ) 
			{
		#ifdef BINDLESS_RESOURCES
				psOut.color = rTexture[psIn.textureId].Sample( rTexture_sampler, psIn.uv ) * psIn.color;
		#else
				psOut.color = rTexture.Sample( rTexture_sampler, psIn.uv ) * psIn.color;
		#endif
			}
    )";

	Pipeline pipeline;
	pipeline.pso = gx::createGraphicsPipelineState( device, gx::GraphicsPipelineDesc()
		.name( "DrawContext Color Pipeline" )
		.inputLayout( {
			// Attribute 0 - vertex position
			gx::LayoutElement{ 0, 0, 3, gx::VT_FLOAT32, false },
			// Attribute 1 - vertex uv
			gx::LayoutElement{ 1, 0, 2, gx::VT_FLOAT32, false },
			// Attribute 0 - vertex color
			gx::LayoutElement{ 2, 0, 4, gx::VT_FLOAT32, false },
			// Attribute 0 - primitive constants index
			gx::LayoutElement{ 3, 0, 1, gx::VT_UINT32, false },
			// Attribute 0 - primitive texture index
			gx::LayoutElement{ 4, 0, 1, gx::VT_UINT32, false },
			} )
			.vertexShader( gx::createShader( gx::ShaderCreateInfo()
				.name( "DrawContext Color VS" )
				.sourceLanguage( gx::SHADER_SOURCE_LANGUAGE_HLSL )
				.shaderType( gx::SHADER_TYPE_VERTEX )
				.useCombinedTextureSamplers( true )
				.source( vertexShader )
			) )
		.pixelShader( gx::createShader( gx::ShaderCreateInfo()
			.name( "DrawContext Color PS" )
			.sourceLanguage( gx::SHADER_SOURCE_LANGUAGE_HLSL )
			.shaderType( gx::SHADER_TYPE_PIXEL )
			.useCombinedTextureSamplers( true )
			.macros( mBindlessResources ? static_cast<const ShaderMacro*>( bindlessMacro ) : nullptr )
			.source( pixelShader )
		) )
		.variables( {
			{ gx::SHADER_TYPE_PIXEL, "rTexture", gx::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC },
			{ gx::SHADER_TYPE_VERTEX, "constantBuffer", gx::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC }
			} )
		.immutableSamplers( { { gx::SHADER_TYPE_PIXEL, "rTexture", Diligent::SamplerDesc() } } )
		.depthStencilDesc( DepthStencilStateDesc()
			.depthEnable( state.depthEnable )
			.depthWriteEnable( state.depthWriteEnable )
			.depthFunc( state.depthFunc )
		)
		.rasterizerStateDesc( RasterizerStateDesc()
			.scissorEnable( true )
			.fillMode( state.fillMode )
			.cullMode( state.cullMode )
		)
		.primitiveTopology( state.primitiveTopology )
		.blendStateDesc( BlendStateDesc()
			.alphaToCoverageEnable( state.alphaToCoverageEnable )
			.renderTarget( 0, RenderTargetBlendDesc()
				.blendEnable( state.blendEnable )
				.srcBlend( state.srcBlend )
				.destBlend( state.destBlend )
				.blendOp( state.blendOp )
				.srcBlendAlpha( state.srcBlendAlpha )
				.destBlendAlpha( state.destBlendAlpha )
				.blendOpAlpha( state.blendOpAlpha )
			)
		) );
	pipeline.pso->CreateShaderResourceBinding( &pipeline.srb, true );

	return pipeline;
}

namespace {
	// from Boost hash: 
	// https://github.com/boostorg/container_hash/blob/master/include/boost/container_hash/hash.hpp#L311
	template <typename SizeT>
	inline void hash_combine( SizeT& seed, SizeT value )
	{
		seed ^= value + 0x9e3779b9 + ( seed << 6 ) + ( seed >> 2 );
	}

	template<typename T>
	size_t hash_value( const T &t )
	{
		return std::hash<T>{}( t );
	}
} // anonymous namespace

size_t DrawContext::State::hash() const
{
	std::size_t seed = 0;
	hash_combine( seed, hash_value( fillMode ) );
	hash_combine( seed, hash_value( cullMode ) );
	hash_combine( seed, hash_value( depthEnable ) );
	hash_combine( seed, hash_value( depthWriteEnable ) );
	hash_combine( seed, hash_value( depthFunc ) );
	hash_combine( seed, hash_value( stencilEnable ) );
	hash_combine( seed, hash_value( stencilReadMask ) );
	hash_combine( seed, hash_value( stencilWriteMask ) );
	hash_combine( seed, hash_value( primitiveTopology ) );
	hash_combine( seed, hash_value( alphaToCoverageEnable ) );
	hash_combine( seed, hash_value( blendEnable ) );
	hash_combine( seed, hash_value( srcBlend ) );
	hash_combine( seed, hash_value( destBlend ) );
	hash_combine( seed, hash_value( blendOp ) );
	hash_combine( seed, hash_value( srcBlendAlpha ) );
	hash_combine( seed, hash_value( destBlendAlpha ) );
	hash_combine( seed, hash_value( blendOpAlpha ) );
	return seed;
}

DrawContext::DrawScope::DrawScope( DrawContext* context, uint32_t indexCount, uint32_t vertexCount )
	: mContext( context ), mIndexCount( indexCount ), mVertexCount( vertexCount )
{
}

DrawContext::DrawScope::~DrawScope()
{
	mContext->mVertex += mVertexCount;
	mContext->mVertexIndex += mVertexCount;
	mContext->mIndex += mIndexCount;

	Command &command = mContext->mCommands.back();
	command.indexCount += mIndexCount;
}

DrawContext::Vertex* DrawContext::DrawScope::getVertices() const
{
	return mContext->mVertex;
}

DrawContext::Index*  DrawContext::DrawScope::getIndices() const
{
	return mContext->mIndex;
}

DrawContext::Index   DrawContext::DrawScope::getIndexOffset() const
{
	return mContext->mVertexIndex;
}

void DrawContext::commit()
{
	Command cmd;
	cmd.vertexOffset = static_cast<uint32_t>( mVertex - mVertices.data() );
	cmd.indexOffset = static_cast<uint32_t>( mIndex - mIndices.data() );
	cmd.indexCount = 0;
	auto scissor = getScissor();
	cmd.scissor = glm::vec4( scissor.first.x, scissor.first.y, scissor.second.x, scissor.second.y );
	auto viewport = getViewport();
	cmd.viewport = glm::vec4( viewport.first.x, viewport.first.y, viewport.second.x, viewport.second.y );
	cmd.state = mState;
	cmd.resources.textureIndex = mTextureIndex;

	mCommands.push_back( cmd );
}

void DrawContext::invalidateTransform()
{
	mTransformValid = false;
	mConstantBufferValid = false;
}

void DrawContext::invalidateResources()
{
	mResourcesValid = false;
	mSRBsValid = false;
}

void DrawContext::invalidateState()
{
	mStateValid = false;
	mPSOsValid = false;
}

void DrawContext::invalidateViewport()
{
	mViewportValid = false;
}

void DrawContext::invalidateScissor()
{
	mScissorValid = false;
}

void DrawContext::submit( bool flushAfterSubmit )
{
	submit( app::getRenderDevice(), app::getImmediateContext(), flushAfterSubmit );
}

void DrawContext::submit( RenderDevice* device, DeviceContext* context, bool flushAfterSubmit )
{
	if( mCommands.empty() ) {
		return;
	}
	// Remove empty trailing command
	if( mCommands.back().indexCount == 0 ) {
		mCommands.pop_back();
	}
	// Verify device features if not done previously
	if( mVerifyDeviceFeatures ) {
		mBindlessResources = device->GetDeviceCaps().Features.BindlessResources;
		mVerifyDeviceFeatures = false;
	}
	// make sure the base texture is initialized
	if( ! mBaseTexture ) {
		uint32_t white = 0xffffffff;
		TextureSubResData subData[] ={ { &white, sizeof( uint32_t ) } };
		TextureData data ={ subData, 1 };
		TextureRef texture;
		device->CreateTexture( TextureDesc()
			.size( ivec2( 1 ) )
			.type( RESOURCE_DIM_TEX_2D )
			.usage( USAGE_IMMUTABLE )
			.bindFlags( BIND_SHADER_RESOURCE )
			.format( TEX_FORMAT_RGBA8_UNORM ),
			&data, &texture );
		mBaseTexture = texture->GetDefaultView( gx::TEXTURE_VIEW_SHADER_RESOURCE );
		mTextures[0] = mBaseTexture;
		for( size_t i = mTextureCount; i < mTextures.size(); ++i ) {
			mTextures[i] = mBaseTexture;
		}
	}

	// update vertex and index buffer

	// NOTES: The following considers the data to be immutable if the same data is submitted multiple times or dynamic 
	// otherwise. USAGE_DYNAMIC buffers are discarded by DE at the end of the frame and need to be rewritten every frames. 
	// So this will default to USAGE_DYNAMIC and revert back to USAGE_IMMUTTABLE if not updated frequently. This will 
	// result in potentially wasting a second initialization and upload if the DrawContent is intended to be static. 
	// This could be made smarter by keeping track of the update frequency but this logic should be a good start already.

	const bool geomImmutable = mGeomBuffersValid;
	if( ! mGeomBuffersValid || geomImmutable != mGeomBuffersImmutable ) {
		// Check vertex buffer size and grow if needed or re-initialized if its type changed
		uint32_t vertexCount = static_cast<uint32_t>( mVertex - mVertices.data() );
		if( ! mVertexBuffer || mVertexBufferSize < vertexCount || geomImmutable != mGeomBuffersImmutable ) {
			while( mVertexBufferSize < vertexCount ) {
				mVertexBufferSize = mVertexBufferSize == 0 ? vertexCount : mVertexBufferSize * 2;
			}
			mVertexBuffer.Release();
			BufferData data = { mVertices.data(), vertexCount * sizeof( Vertex ) };
			device->CreateBuffer( BufferDesc()
				.name( "DrawContext vertex buffer" )
				.usage( geomImmutable ? USAGE_IMMUTABLE : USAGE_DYNAMIC )
				.bindFlags( BIND_VERTEX_BUFFER )
				.cpuAccessFlags( geomImmutable ? CPU_ACCESS_NONE : CPU_ACCESS_WRITE )
				.sizeInBytes( geomImmutable ? vertexCount * sizeof( Vertex ) : mVertexBufferSize * sizeof( Vertex ) ),
				geomImmutable ? &data : nullptr, &mVertexBuffer );
		}
		// Check index buffer size and grow if needed or re-initialized if its type changed
		uint32_t indexCount = static_cast<uint32_t>( mIndex - mIndices.data() );
		if( ! mIndexBuffer || mIndexBufferSize < indexCount || geomImmutable != mGeomBuffersImmutable ) {
			while( mIndexBufferSize < indexCount ) {
				mIndexBufferSize = mIndexBufferSize == 0 ? indexCount : mIndexBufferSize * 2;
			}
			mIndexBuffer.Release();
			BufferData data = { mIndices.data(), indexCount * sizeof( Index ) };
			device->CreateBuffer( BufferDesc()
				.name( "DrawContext index buffer" )
				.usage( geomImmutable ? USAGE_IMMUTABLE : USAGE_DYNAMIC )
				.bindFlags( BIND_INDEX_BUFFER )
				.cpuAccessFlags( geomImmutable ? CPU_ACCESS_NONE : CPU_ACCESS_WRITE )
				.sizeInBytes( geomImmutable ? indexCount * sizeof( Index ) : mIndexBufferSize * sizeof( Index ) ),
				geomImmutable ? &data : nullptr, &mIndexBuffer );
		}

		// Copy vertex and index
		if( ! geomImmutable ) {
			MapHelper<Vertex> vertices( context, mVertexBuffer, MAP_WRITE, MAP_FLAG_DISCARD );
			MapHelper<Index> indices( context, mIndexBuffer, MAP_WRITE, MAP_FLAG_DISCARD );
			memcpy( vertices, mVertices.data(), vertexCount * sizeof( Vertex ) );
			memcpy( indices, mIndices.data(), indexCount * sizeof( Index ) );
		}

		mGeomBuffersImmutable = geomImmutable;
		mGeomBuffersValid = true;
	}
	// update the constant buffer
	const bool constantsImmutable = mConstantBufferValid;
	if( ! mConstantBufferValid || constantsImmutable != mConstantBufferImmutable ) {
		// Check constants buffer size and grow if needed or re-initialized if its type changed
		if( ! mConstantsBuffer || mConstantsBufferSize < mConstantCount || constantsImmutable != mConstantBufferImmutable ) {
			while( mConstantsBufferSize < mConstantCount ) {
				mConstantsBufferSize = mConstantsBufferSize == 0 ? mConstantCount : mConstantsBufferSize * 2;
			}
			mConstantsBuffer.Release();
			BufferData data = { mConstants.data(), mConstantCount * sizeof( Constants ) };
			device->CreateBuffer( BufferDesc()
				.name( "DrawContext constants buffer" )
				.usage( constantsImmutable ? USAGE_IMMUTABLE : USAGE_DYNAMIC )
				.bindFlags( BIND_SHADER_RESOURCE )
				.mode( BUFFER_MODE_STRUCTURED )
				.cpuAccessFlags( constantsImmutable ? CPU_ACCESS_NONE : CPU_ACCESS_WRITE )
				.sizeInBytes( mConstantCount * sizeof( Constants ) )
				.elementByteStride( sizeof( Constants ) ),
				constantsImmutable ? &data : nullptr, &mConstantsBuffer );
			mConstantsBufferSRV = mConstantsBuffer->GetDefaultView( BUFFER_VIEW_SHADER_RESOURCE );
		}
		{
			for( const auto &namedTransform : mTransforms ) {
				const Transform& transform = namedTransform.second;
				if( transform.mActive ) {
					mConstants[transform.mTargetIndex].transform = glm::transpose( transform.mParentTransform * transform.mTransform );
				}
			}
		}
		// Copy constant data
		if( ! constantsImmutable ) {
			MapHelper<Constants> constants( context, mConstantsBuffer, MAP_WRITE, MAP_FLAG_DISCARD );
			memcpy( constants, mConstants.data(), mConstantCount * sizeof( Constants ) );
		}

		mConstantBufferValid = true;
		mConstantBufferImmutable = constantsImmutable;
	}

	// Bind the index buffer once now, index offsets are provided on a draw call basis 
	context->SetIndexBuffer( mIndexBuffer, 0, gx::RESOURCE_STATE_TRANSITION_MODE_TRANSITION );

	// Make sure pipelines and srbs are initialized
	if( ! mPSOsValid ) {
		for( size_t i = 0; i < mCommands.size(); i++ ) {
			Command &command = mCommands[i];
			size_t stateHash = command.state.hash();
			if( i == 0 || stateHash != mCommands[i - 1].state.hash() ) {
				if( ! mPipelines.count( stateHash ) ) {
					mPipelines.insert( { stateHash, initializePipelineState( device, command.state ) } );
				}
			}
		}
		mPSOsValid = true;
	}

	// Merge subsequent commands
	// TODO: Only merges two subsequent commands, should probably switch to a double for-loop to be 
	//		 able to merge more than two in a row.
	for( size_t i = 1; i < mCommands.size(); i++ ) {
		// when bindless resources are available commands that differ only 
		// by their texture index can be merged into a single command
		if( mBindlessResources &&
			mCommands[i].state.hash() == mCommands[i-1].state.hash() &&
			mCommands[i].viewport == mCommands[i-1].viewport &&
			mCommands[i].scissor == mCommands[i-1].scissor &&
			mCommands[i].resources.textureIndex != mCommands[i-1].resources.textureIndex ) {
			// merge and shift left
			mCommands[i-1].indexCount += mCommands[i].indexCount;
			mCommands.erase( mCommands.begin() + i );
		}
	}
	
	// Submit commands list
	for( size_t i = 0; i < mCommands.size(); i++ ) {
		Command &command = mCommands[i];

		const Viewport viewport( command.viewport.x, command.viewport.y, command.viewport.z, command.viewport.w );
		context->SetViewports( 1, &viewport, 0, 0 );
		const gx::Rect scissor( command.scissor.x, command.scissor.y, command.scissor.z, command.scissor.w );
		context->SetScissorRects( 1, &scissor, 0, 0 );

		size_t stateHash = command.state.hash();
		if( i == 0 || stateHash != mCommands[i-1].state.hash() ) {
			Pipeline &pipeline = mPipelines[stateHash];
			pipeline.srb->GetVariableByName( SHADER_TYPE_VERTEX, "constantBuffer" )->Set( mConstantsBufferSRV );
			if( ! mBindlessResources ) {
				pipeline.srb->GetVariableByName( SHADER_TYPE_PIXEL, "rTexture" )->Set( getTextureAt( command.resources.textureIndex ) );
			}
			else {
				pipeline.srb->GetVariableByName( SHADER_TYPE_PIXEL, "rTexture" )->SetArray( &mTextures[0], 0, static_cast<uint32_t>( mTextures.size() ) );
			}
			context->SetPipelineState( pipeline.pso );
			context->CommitShaderResources( pipeline.srb, gx::RESOURCE_STATE_TRANSITION_MODE_TRANSITION );
		}
		// if bindless resources are not supported srb might need to be updated
		else if( ! mBindlessResources && command.resources.textureIndex != mCommands[i-1].resources.textureIndex ) {
			Pipeline &pipeline = mPipelines[stateHash];
			pipeline.srb->GetVariableByName( SHADER_TYPE_PIXEL, "rTexture" )->Set( getTextureAt( command.resources.textureIndex ) );
			context->CommitShaderResources( pipeline.srb, gx::RESOURCE_STATE_TRANSITION_MODE_TRANSITION );
		}

		uint32_t offsets[] = { /*command.vertexOffset*/0 };
		Buffer* buffers[] = { mVertexBuffer };
		context->SetVertexBuffers( 0, 1, buffers, offsets, gx::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, gx::SET_VERTEX_BUFFERS_FLAG_RESET );

		context->DrawIndexed( gx::DrawIndexedAttribs()
			.indexType( sizeof( Index ) == 2 ? VT_UINT16 : VT_UINT32 )
			.numIndices( command.indexCount )
			.flags( gx::DRAW_FLAG_VERIFY_STATES )
			.firstIndexLocation( command.indexOffset )
		);
	}

	if( flushAfterSubmit ) {
		flush();
	}
}

#if defined( IMGUI_DEGUG )
void DrawContext::debugSubmit( const char* label, bool* open, bool flushAfterSubmit )
{
	if( ImGui::Begin( label, open ) ) {
		if( mCommands.empty() ) {
			ImGui::Text( "CommandBuffer empty" );
		}
		// Remove empty trailing command
		if( mCommands.back().indexCount == 0 ) {
			ImGui::Text( "CommandBuffer empty trailing command" );
		}
		ImGui::PushItemWidth( ImGui::GetWindowContentRegionWidth() / 4.0f );
		// Check vertex buffer size and grow if needed
		int vertexCount = static_cast<int>( mVertex - mVertices.data() );
		int vertexBufferSize = mVertices.size();
		bool vertexGrow = false;
		if( ! mVertexBuffer || mVertexBufferSize < vertexCount ) {
			vertexGrow = true;
		}
		ImGui::DragInt( "vertexCount", &vertexCount );
		ImGui::DragInt( "vertexBufferSize", &vertexBufferSize );
		ImGui::SameLine();
		ImGui::Checkbox( "###VertexGrow", &vertexGrow );
		// Check index buffer size and grow if needed
		int indexCount = static_cast<uint32_t>( mIndex - mIndices.data() );
		int indexBufferSize = mIndices.size();
		bool indexGrow = false;
		if( !mIndexBuffer || mIndexBufferSize < indexCount ) {
			indexGrow = true;
		}
		ImGui::DragInt( "indexCount", &indexCount );
		ImGui::DragInt( "indexBufferSize", &indexBufferSize );
		ImGui::SameLine();
		ImGui::Checkbox( "###indexGrow", &indexGrow );
		// Copy vertex, index and constant data
		{
			int vertexStreamBytes = vertexCount * sizeof( Vertex );
			int indexStreamBytes = indexCount * sizeof( Index );
			int constantStreamBytes = mConstantCount * sizeof( Constants );
			ImGui::DragInt( "vertexStreamBytes", &vertexStreamBytes );
			ImGui::DragInt( "indexStreamBytes", &indexStreamBytes );
			ImGui::DragInt( "constantStreamBytes", &constantStreamBytes );
		}
		ImGui::PopItemWidth();

		if( ImGui::TreeNodeEx( "Vertices" ) ) {
			for( size_t i = 0; i < mVertices.size(); ++i ) {
				ImGui::PushID( i );
				if( ImGui::TreeNode( "Vertex", "Vertex-%d", i ) ) {
					Vertex& v = mVertices[i];
					ImGui::DragFloat3( "position", &v.position[0] );
					ImGui::DragFloat2( "uv", &v.uv[0] );
					ImGui::DragFloat4( "color", &v.color[0] );
					int constantIndex = v.constants;
					ImGui::DragInt( "constIndex", &constantIndex );
					ImGui::TreePop();
				}
				ImGui::PopID();
			}
			ImGui::TreePop();
		}

		if( ImGui::TreeNodeEx( "Indices" ) ) {
			for( size_t i = 0; i < mIndices.size(); ++i ) {
				int index = mIndices[i];
				ImGui::DragInt( "", &index );
			}
			ImGui::TreePop();
		}

		if( ImGui::TreeNodeEx( "Constants" ) ) {
			for( size_t i = 0; i < mConstants.size(); ++i ) {
				ImGui::PushID( i );
				if( ImGui::TreeNode( "Constant", "Constant-%d", i ) ) {
					Constants& c = mConstants[i];
					ImGui::DragFloat4( "transform", &c.transform[0] );
					ImGui::DragFloat4( "", &c.transform[1] );
					ImGui::DragFloat4( "", &c.transform[2] );
					ImGui::DragFloat4( "", &c.transform[3] );
					ImGui::TreePop();
				}
				ImGui::PopID();
			}
			ImGui::TreePop();
		}

		// Submit commands list
		if( ImGui::TreeNodeEx( "Commands" ) ) {
			for( size_t i = 0; i < mCommands.size(); i++ ) {
				ImGui::PushID( i );
				if( ImGui::TreeNode( "Command", "Command-%d", i ) ) {
					Command &command = mCommands[i];

					ImGui::DragFloat4( "viewport", &command.viewport[0] );
					ImGui::DragInt4( "scissor", &command.scissor[0] );
					ImGui::DragScalar( "vertexOffset", ImGuiDataType_U32, &command.vertexOffset, 1.0f );
					ImGui::DragScalar( "indexOffset", ImGuiDataType_U32, &command.indexOffset, 1.0f );
					ImGui::DragScalar( "indexCount", ImGuiDataType_U32, &command.indexCount, 1.0f );

					
					size_t stateHash = command.state.hash();
					if( i == 0 || stateHash != mCommands[i - 1].state.hash() ) {
						int textureId = command.resources.texture ? (int) command.resources.texture.RawPtr() : (int) mBaseTexture.RawPtr();
						ImGui::DragInt( "CommitTexture", &textureId );
						bool setPipeline = true;
						ImGui::Checkbox( "SetPipeline", &setPipeline );
					}
					else if( command.resources.texture != mCommands[i-1].resources.texture ) {

						int textureId = command.resources.texture ? (int) command.resources.texture.RawPtr() : (int) mBaseTexture.RawPtr();
						ImGui::DragInt( "CommitTexture", &textureId );
						bool setPipeline = false;
						ImGui::Checkbox( "SetPipeline", &setPipeline );
					}

					if( ImGui::TreeNodeEx( "State" ) ) {

						ImGui::TreePop();
					}
					if( ImGui::TreeNodeEx( "Resources" ) ) {
						int textureId = command.resources.texture ? (int) command.resources.texture.RawPtr() : 0;
						ImGui::DragInt( "texture", &textureId );
						ImGui::TreePop();
					}

					if( ImGui::TreeNodeEx( "Vertices###cmdverts" ) ) {
						for( size_t i = 0; i < command.indexCount; ++i ) {
							ImGui::PushID( i );
							if( ImGui::TreeNode( "Vertex", "Vertex-%d", i ) ) {
								int index = mIndices[command.indexOffset + i];
								Vertex& v = mVertices[index];
								ImGui::DragFloat3( "position", &v.position[0] );
								ImGui::DragFloat2( "uv", &v.uv[0] );
								ImGui::DragFloat4( "color", &v.color[0] );
								int constantIndex = v.constants;
								ImGui::DragInt( "constIndex", &constantIndex );
								ImGui::TreePop();
							}
							ImGui::PopID();
						}
						ImGui::TreePop();
					}
					
					ImGui::TreePop();
				}
				ImGui::PopID();
			}
			ImGui::TreePop();
		}

		bool doFlush = flushAfterSubmit;
		ImGui::Checkbox( "flush", &doFlush );
	}
	ImGui::End();
}
#endif

void DrawContext::bindTexture( TextureViewRef &texture )
{
	mTextureIndex = getTextureIndex( texture );
	invalidateResources();
}

void DrawContext::unbindTexture()
{
	mTextureIndex = 0;
	invalidateResources();
}

uint32_t DrawContext::getTextureIndex( TextureViewRef &texture )
{
	for( uint32_t i = 0; i < mTextureCount; ++i ) {
		if( mTextures[i] == texture ) {
			return i;
		}
	}

	mTextureCount++;
	mTextures[mTextureCount-1] = static_cast<IDeviceObject*>( texture.RawPtr() );
	return mTextureCount - 1;
}

IDeviceObject* DrawContext::getTextureAt( uint32_t index ) const
{
	return mTextures[index];
}

DrawContext::Transform& DrawContext::operator[]( const std::string &name )
{
	Transform& transform = getTransform( name );
	return transform;
}

DrawContext::Transform& DrawContext::getTransform( const std::string &name )
{
	if( ! mTransforms.count( name ) ) {
		Transform transform;
		transform.mName = name;
		transform.mParent = this;
		mTransforms.insert( { name, transform } );
	}

	return mTransforms[name];
}

DrawContext::Transform::Transform()
	: mActive( false )
{
}

void DrawContext::detachTransform( Transform &transform, const ci::mat4 &initialValue )
{
	multModelMatrix( initialValue );
	transform.mParentTransform = getModelViewProjection();
	transform.mTargetIndex = mConstantIndex + 1;
	transform.mActive = true;
}

void DrawContext::Transform::operator=( const ci::mat4 &transform )
{
	mTransform = transform;
	mParent->invalidateTransform();
}

gx::CommandListRef DrawContext::bake( DeviceContext* context )
{
	return bake( app::getRenderDevice(), context );
}

gx::CommandListRef DrawContext::bake( RenderDevice* device, DeviceContext* context )
{
	gx::CommandListRef commandList;
	submit( device, context, true );
	context->FinishCommandList( &commandList );
	return commandList;
}

void DrawContext::flush()
{
	mVertexIndex = 0;
	mVertex = mVertices.data();
	mIndex = mIndices.data();
	mConstantIndex = 0;
	mConstantCount = 1;
	mTextureCount = 1;

	mCommands.clear();
}

void DrawContext::startDraw( uint32_t indexCount, uint32_t vertexCount )
{
	// check whether a new Command is needed
	bool needsCommit = mCommands.empty();
	if( ! mTransformValid ) {
		// grow constants buffer
		if( mConstantIndex + 1 >= mConstants.size() ) {
			mConstants.resize( mConstants.size() * 2 );
		}
		// push a new constant to the buffer
		mConstantIndex++;
		mConstantCount++;
		mConstants[mConstantIndex].transform = glm::transpose( getModelViewProjection() );
		mTransformValid = true;
	}
	// viewport / scissor changes signal the end of a Command
	if( ! mViewportValid || ! mScissorValid ) {
		mViewportValid = true;
		mScissorValid = true;
		needsCommit = true;
	}
	// state changes signal the end of a Command
	if( ! mStateValid ) {
		mStateValid = true;
		needsCommit = true;
	}
	// resources changes signal the end of a Command
	if( ! mResourcesValid ) {
		mResourcesValid = true;
		needsCommit = true;
	}
	// commit previous Command and create a new one
	if( needsCommit ) {
		commit();
	}
	// grow vertex and index buffers if needed
	size_t currentVertexIndex = mVertex - mVertices.data();
	size_t currentIndexIndex = mIndex - mIndices.data();
	if( currentVertexIndex + vertexCount >= mVertices.size() ) {
		mVertices.resize( mVertices.size() + vertexCount );
		mVertex = &mVertices[currentVertexIndex];
	}
	if( currentIndexIndex + indexCount >= mIndices.size() ) {
		mIndices.resize( mIndices.size() + indexCount );
		mIndex = &mIndices[currentIndexIndex];
	}
	// sets the default values on the allocated vertices
	for( size_t i = 0; i < vertexCount; ++i ) {
		mVertex[i].setConstantsIndex( mConstantIndex );
		mVertex[i].setColor( mColor );
		mVertex[i].setTextureIndex( mTextureIndex );
	}

	// vertex and index buffers needs to be updated
	mGeomBuffersValid = false;
}

DrawContext::DrawScope DrawContext::getDrawScope( uint32_t indexCount, uint32_t vertexCount )
{
	startDraw( indexCount, vertexCount );
	return DrawScope( this, indexCount, vertexCount );
}

namespace {
uint32_t determineRequiredIndices( geom::Primitive sourcePrimitive, geom::Primitive destPrimitive, size_t numIndices )
	{
		switch( destPrimitive ) {
			case geom::Primitive::TRIANGLES:
				switch( sourcePrimitive ) {
					case geom::Primitive::TRIANGLES:
						return static_cast<uint32_t>( numIndices );
					break;
					case geom::Primitive::TRIANGLE_STRIP: // ABC, CBD, CDE, EDF, etc
						return static_cast<uint32_t>( ( numIndices - 2 ) * 3 ); // Triangles: 3:1  4:2  5:3;
					break;
					case geom::Primitive::TRIANGLE_FAN: // ABC, ACD, ADE, etc
						return static_cast<uint32_t>( ( numIndices - 2 ) * 3 ); // Triangles: 3:1  4:2  5:3;
					break;
					default:
						return 0;
				}
			break;
			case geom::Primitive::LINES:
				switch( sourcePrimitive ) {
					case geom::Primitive::LINES:
						return static_cast<uint32_t>( numIndices );
					break;
					case geom::Primitive::LINE_STRIP:
						return static_cast<uint32_t>( (numIndices - 1) * 2 );
					break;
					default:
						return 0;
				}
			break;
			default:
				return 0;
		}
	}
}

DrawContext::GeomTarget::GeomTarget( DrawContext* context, const geom::Source *source )
	: mContext( context ), 
	mSource( source ), 
	mIndexCount( determineRequiredIndices( source->getPrimitive(), geom::TRIANGLES, source->getNumIndices() ? source->getNumIndices() : source->getNumVertices() ) ),
	mVertexCount( static_cast<uint32_t>( source->getNumVertices() ) )
{
	mContext->startDraw( mIndexCount, mVertexCount );

	if( ! mSource->getNumIndices() || mSource->getPrimitive() != geom::TRIANGLES ) {
		Target::generateIndicesForceTriangles( mSource->getPrimitive(), source->getNumVertices(), getIndexOffset(), getIndices() );
	}
}

DrawContext::GeomTarget::~GeomTarget()
{
	mContext->mVertex += mVertexCount;
	mContext->mVertexIndex += mVertexCount;
	mContext->mIndex += mIndexCount;

	Command &command = mContext->mCommands.back();
	command.indexCount += mIndexCount;
}

DrawContext::Vertex* DrawContext::GeomTarget::getVertices() const
{
	return mContext->mVertex;
}

DrawContext::Index*  DrawContext::GeomTarget::getIndices() const
{
	return mContext->mIndex;
}

DrawContext::Index   DrawContext::GeomTarget::getIndexOffset() const
{
	return mContext->mVertexIndex;
}

uint8_t	DrawContext::GeomTarget::getAttribDims( geom::Attrib attr ) const
{
	return mSource->getAttribDims( attr );
}

void DrawContext::GeomTarget::copyAttrib( geom::Attrib attr, uint8_t dims, size_t strideBytes, const float *sourceData, size_t count )
{
	const size_t stride = strideBytes ? strideBytes / sizeof(float) : dims;
	if( attr == geom::Attrib::POSITION ) {
		Vertex*	vertices = getVertices();
		for( size_t i = 0; i < count; ++i ) {
			vertices[i].setPosition( sourceData[i * stride], sourceData[i * stride + 1], dims > 2 ? sourceData[i * stride + 2] : 0.0f );
		}
	}
	else if( attr == geom::Attrib::TEX_COORD_0 ) {
		Vertex*	vertices = getVertices();
		for( size_t i = 0; i < count; ++i ) {
			vertices[i].setUv( sourceData[i * stride], sourceData[i * stride + 1] );
		}
	}
	else if( attr == geom::Attrib::COLOR ) {
		Vertex*	vertices = getVertices();
		for( size_t i = 0; i < count; ++i ) {
			vertices[i].setColor( sourceData[i * stride], sourceData[i * stride + 1], sourceData[i * stride + 2], dims > 3 ? sourceData[i * stride + 3] : 1.0f );
		}
	}
}

void DrawContext::GeomTarget::copyIndices( geom::Primitive primitive, const uint32_t *sourceData, size_t numIndices, uint8_t requiredBytesPerIndex )
{
	if( numIndices == 0 )
		return;

	const Index index = getIndexOffset();
	Index* indices = getIndices();
	for( size_t i = 0; i < numIndices; ++i ) {
		indices[i] = index + sourceData[i];
	}
}

void DrawContext::draw( const geom::Source &source )
{
	GeomTarget target( this, &source );
	geom::AttribSet requestedAttribs = { geom::Attrib::POSITION, geom::Attrib::TEX_COORD_0/*, geom::Attrib::COLOR*/ };
	source.loadInto( &target, requestedAttribs );
}

void DrawContext::drawSolidRect( const Rectf &r, const vec2 &upperLeftTexCoord, const vec2 &lowerRightTexCoord )
{
	const DrawScope	scope = getDrawScope( 6, 4 );
	const Index			offset = scope.getIndexOffset();
	Vertex*				vertices = scope.getVertices();
	Index*				indices	= scope.getIndices();

	vertices[0].setPosition( r.getUpperLeft() );  vertices[0].setUv( upperLeftTexCoord );
	vertices[1].setPosition( r.getUpperRight() ); vertices[1].setUv( lowerRightTexCoord.x, upperLeftTexCoord.y );
	vertices[2].setPosition( r.getLowerRight() ); vertices[2].setUv( lowerRightTexCoord );
	vertices[3].setPosition( r.getLowerLeft() );  vertices[3].setUv( upperLeftTexCoord.x, lowerRightTexCoord.y );

	indices[0] = offset + 0;
	indices[1] = offset + 1;
	indices[2] = offset + 2;
	indices[3] = offset + 0;
	indices[4] = offset + 2;
	indices[5] = offset + 3;
}

std::pair<vec2, vec2> DrawContext::getViewport()
{
	if( mViewportStack.empty() ) {
		vec2 size( app::getSwapChain()->GetDesc().Width, app::getSwapChain()->GetDesc().Height );
		mViewportStack.push_back( std::pair<vec2, vec2>( vec2( 0, 0 ), size ) );
		mViewportStack.push_back( std::pair<vec2, vec2>( vec2( 0, 0 ), size ) );
	}

	return mViewportStack.back();
}

void DrawContext::setViewport( const std::pair<vec2, vec2> &viewport )
{
	if( setStackState( mViewportStack, viewport ) ) {
		invalidateViewport();
	}
}

void DrawContext::pushViewport( const std::pair<vec2, vec2> &viewport )
{
	if( pushStackState( mViewportStack, viewport ) ) {
		invalidateViewport();
	}
}

void DrawContext::popViewport( bool forceRestore )
{
	if( mViewportStack.empty() ) {
		CI_LOG_E( "Viewport stack underflow" );
	}
	else if( popStackState( mViewportStack ) || forceRestore ) {
		invalidateViewport();
	}
}

void DrawContext::setScissor( const std::pair<ivec2, ivec2> &scissor )
{
	if( setStackState( mScissorStack, scissor ) ) {
		invalidateScissor();
	}
}

void DrawContext::pushScissor( const std::pair<ivec2, ivec2> &scissor )
{
	if( pushStackState( mScissorStack, scissor ) ) {
		invalidateScissor();
	}
}

void DrawContext::pushScissor()
{
	mScissorStack.push_back( getScissor() );
}

void DrawContext::popScissor( bool forceRestore )
{
	if( mScissorStack.empty() )
		CI_LOG_E( "Scissor stack underflow" );
	else if( popStackState( mScissorStack ) || forceRestore ) {
		invalidateScissor();
	}
}

std::pair<ivec2, ivec2> DrawContext::getScissor()
{
	if( mScissorStack.empty() ) {
		ivec2 size( app::getSwapChain()->GetDesc().Width, app::getSwapChain()->GetDesc().Height );
		mScissorStack.push_back( std::pair<ivec2, ivec2>( ivec2( 0, 0 ), size ) );
		mScissorStack.push_back( std::pair<ivec2, ivec2>( ivec2( 0, 0 ), size ) );
	}

	return mScissorStack.back();
}

void DrawContext::enableDepthRead( bool enable )
{
	State& state = mState;
	state.depthEnable = enable;
	invalidateState();
}

void DrawContext::enableDepthWrite( bool enable )
{
	State& state = mState;
	state.depthWriteEnable = enable;
	invalidateState();
}

void DrawContext::enableBlending( bool enable )
{
	State& state = mState;
	state.blendEnable = enable;
	invalidateState();
}

void DrawContext::enableAlphaBlending( bool enable )
{
	State& state = mState;
	state.blendEnable = true;
	state.srcBlend = BLEND_FACTOR_SRC_ALPHA;
	state.destBlend = BLEND_FACTOR_INV_SRC_ALPHA;
	invalidateState();
}

void DrawContext::enableAlphaBlendingPremult()
{
	State& state = mState;
	state.blendEnable = true;
	state.srcBlend = BLEND_FACTOR_ONE;
	state.destBlend = BLEND_FACTOR_INV_SRC_ALPHA;
	invalidateState();
}

void DrawContext::enableAdditiveBlending()
{
	State& state = mState;
	state.blendEnable = true;
	state.srcBlend = BLEND_FACTOR_SRC_ALPHA;
	state.destBlend = BLEND_FACTOR_ONE;
	invalidateState();
}

void DrawContext::enableFaceCulling( bool enable )
{
	State& state = mState;
	state.cullMode = enable ? CULL_MODE_BACK : CULL_MODE_NONE;
	invalidateState();
}

void DrawContext::cullFace( CULL_MODE mode )
{
	State& state = mState;
	state.cullMode = mode;
	invalidateState();
}

void DrawContext::enableStencilTest( bool enable )
{
	State& state = mState;
	state.stencilEnable = enable;
	invalidateState();
}

void DrawContext::stencilReadMask( uint8_t mask )
{
	State& state = mState;
	state.stencilReadMask = mask;
	invalidateState();
}

void DrawContext::stencilWriteMask( uint8_t mask )
{
	State& state = mState;
	state.stencilWriteMask = mask;
	invalidateState();
}

void DrawContext::polygonMode( FILL_MODE mode )
{
	State& state = mState;
	state.fillMode = mode;
	invalidateState();
}

void DrawContext::setWireframeEnabled( bool enable )
{
	State& state = mState;
	state.fillMode = enable ? FILL_MODE_WIREFRAME : FILL_MODE_SOLID;
	invalidateState();
}

void DrawContext::setMatrices( const ci::Camera& cam )
{
	mViewMatrixStack.back() = cam.getViewMatrix();
	mProjectionMatrixStack.back() = cam.getProjectionMatrix();
	mModelMatrixStack.back() = mat4();
	invalidateTransform();
}

void DrawContext::setModelMatrix( const ci::mat4 &m )
{
	mModelMatrixStack.back() = m;
	invalidateTransform();
}

void DrawContext::setViewMatrix( const ci::mat4 &m )
{
	mViewMatrixStack.back() = m;
	invalidateTransform();
}

void DrawContext::setProjectionMatrix( const ci::mat4 &m )
{
	mProjectionMatrixStack.back() = m;
	invalidateTransform();
}

void DrawContext::pushModelMatrix()
{
	mModelMatrixStack.push_back( mModelMatrixStack.back() );
	invalidateTransform();
}

void DrawContext::popModelMatrix()
{
	mModelMatrixStack.pop_back();
	invalidateTransform();
}

void DrawContext::pushViewMatrix()
{
	mViewMatrixStack.push_back( mViewMatrixStack.back() );
	invalidateTransform();
}

void DrawContext::popViewMatrix()
{
	mViewMatrixStack.pop_back();
	invalidateTransform();
}

void DrawContext::pushProjectionMatrix()
{
	mProjectionMatrixStack.push_back( mProjectionMatrixStack.back() );
	invalidateTransform();
}

void DrawContext::popProjectionMatrix()
{
	mProjectionMatrixStack.pop_back();
	invalidateTransform();
}

void DrawContext::pushModelView()
{
	mModelMatrixStack.push_back( mModelMatrixStack.back() );
	mViewMatrixStack.push_back( mViewMatrixStack.back() );
	invalidateTransform();
}

void DrawContext::popModelView()
{
	mModelMatrixStack.pop_back();
	mViewMatrixStack.pop_back();
	invalidateTransform();
}

void DrawContext::pushMatrices()
{
	mModelMatrixStack.push_back( mModelMatrixStack.back() );
	mViewMatrixStack.push_back( mViewMatrixStack.back() );
	mProjectionMatrixStack.push_back( mProjectionMatrixStack.back() );
	invalidateTransform();
}

void DrawContext::popMatrices()
{
	mModelMatrixStack.pop_back();
	mViewMatrixStack.pop_back();
	mProjectionMatrixStack.pop_back();
	invalidateTransform();
}

void DrawContext::multModelMatrix( const ci::mat4& mtx )
{
	mModelMatrixStack.back() *= mtx;
	invalidateTransform();
}

void DrawContext::multViewMatrix( const ci::mat4& mtx )
{
	mViewMatrixStack.back() *= mtx;
	invalidateTransform();
}

void DrawContext::multProjectionMatrix( const ci::mat4& mtx )
{
	mProjectionMatrixStack.back() *= mtx;
	invalidateTransform();
}

mat4 DrawContext::getModelMatrix()
{
	return mModelMatrixStack.back();
}

mat4 DrawContext::getViewMatrix()
{
	return mViewMatrixStack.back();
}

mat4 DrawContext::getProjectionMatrix()
{
	return mProjectionMatrixStack.back();
}

mat4 DrawContext::getModelView()
{
	return mViewMatrixStack.back() * mModelMatrixStack.back();
}

mat4 DrawContext::getModelViewProjection()
{
	return mProjectionMatrixStack.back() * mViewMatrixStack.back() * mModelMatrixStack.back();
}

mat4 DrawContext::calcViewMatrixInverse()
{
	return glm::inverse( getViewMatrix() );
}

mat3 DrawContext::calcNormalMatrix()
{
	return glm::inverseTranspose( glm::mat3( getModelView() ) );
}

mat3 DrawContext::calcModelMatrixInverseTranspose()
{
	auto m = glm::inverseTranspose( getModelMatrix() );
	return mat3( m );
}

mat4 DrawContext::calcViewportMatrix()
{
	auto curViewport = getViewport();

	const float a = ( curViewport.second.x - curViewport.first.x ) / 2.0f;
	const float b = ( curViewport.second.y - curViewport.first.y ) / 2.0f;
	const float c = 1.0f / 2.0f;

	const float tx = ( curViewport.second.x + curViewport.first.x ) / 2.0f;
	const float ty = ( curViewport.second.y + curViewport.second.y ) / 2.0f;
	const float tz = 1.0f / 2.0f;

	return mat4(
		a, 0, 0, 0,
		0, b, 0, 0,
		0, 0, c, 0,
		tx, ty, tz, 1
	);
}

void DrawContext::setMatricesWindowPersp( int screenWidth, int screenHeight, float fovDegrees, float nearPlane, float farPlane, bool originUpperLeft )
{
	CameraPersp cam( screenWidth, screenHeight, fovDegrees, nearPlane, farPlane );
	mModelMatrixStack.back() = mat4();
	mProjectionMatrixStack.back() = cam.getProjectionMatrix();
	mViewMatrixStack.back() = cam.getViewMatrix();
	if( originUpperLeft ) {
		mViewMatrixStack.back() *= glm::scale( vec3( 1, -1, 1 ) );								// invert Y axis so increasing Y goes down.
		mViewMatrixStack.back() *= glm::translate( vec3( 0, (float) -screenHeight, 0 ) );		// shift origin up to upper-left corner.
	}
	invalidateTransform();
}

void DrawContext::setMatricesWindowPersp( const ci::ivec2& screenSize, float fovDegrees, float nearPlane, float farPlane, bool originUpperLeft )
{
	setMatricesWindowPersp( screenSize.x, screenSize.y, fovDegrees, nearPlane, farPlane, originUpperLeft );
}

void DrawContext::setMatricesWindow( int screenWidth, int screenHeight, bool originUpperLeft )
{
	mModelMatrixStack.back() = mat4();
	mViewMatrixStack.back() = mat4();

	float sx = 2.0f / (float) screenWidth;
	float sy = 2.0f / (float) screenHeight;
	float ty = -1;

	if( originUpperLeft ) {
		sy *= -1;
		ty *= -1;
	}

	mat4 &m = mProjectionMatrixStack.back();
	m = mat4( sx, 0, 0, 0,
		0, sy, 0, 0,
		0, 0, -1, 0,
		-1, ty, 0, 1 );

	invalidateTransform();
}

void DrawContext::setMatricesWindow( const ci::ivec2& screenSize, bool originUpperLeft )
{
	setMatricesWindow( screenSize.x, screenSize.y, originUpperLeft );
}

void DrawContext::rotate( const quat &quat )
{
	multModelMatrix( toMat4( quat ) );
}

void DrawContext::rotate( float angleRadians, const ci::vec3 &axis )
{
	if( math<float>::abs( angleRadians ) > EPSILON_VALUE ) {
		multModelMatrix( glm::rotate( angleRadians, axis ) );
	}
}

void DrawContext::scale( const ci::vec3 &v )
{
	multModelMatrix( glm::scale( v ) );
}

void DrawContext::translate( const ci::vec3 &v )
{
	multModelMatrix( glm::translate( v ) );
}

const ColorAf& DrawContext::getCurrentColor() const 
{ 
	return mColor; 
}

void DrawContext::setCurrentColor( const ColorAf &color ) 
{ 
	mColor = color;
}

void DrawContext::color( float r, float g, float b )
{
	setCurrentColor( ColorAf( r, g, b, 1.0f ) );
}

void DrawContext::color( float r, float g, float b, float a )
{
	setCurrentColor( ColorAf( r, g, b, a ) );
}

void DrawContext::color( const ci::Color &c )
{
	setCurrentColor( c );
}

void DrawContext::color( const ci::ColorA &c )
{
	setCurrentColor( c );
}

void DrawContext::color( const ci::Color8u &c )
{
	setCurrentColor( c );
}

void DrawContext::color( const ci::ColorA8u &c )
{
	setCurrentColor( c );
}

//////////////////////////////////////////////////////////////////////////////////////////
// Templated stack management routines
template<typename T>
bool DrawContext::pushStackState( std::vector<T> &stack, T value )
{
	bool needsToBeSet = true;
	if( ( ! stack.empty() ) && ( stack.back() == value ) )
		needsToBeSet = false;
	stack.push_back( value );
	return needsToBeSet;
}

template<typename T>
bool DrawContext::popStackState( std::vector<T> &stack )
{
	if( ! stack.empty() ) {
		T prevValue = stack.back();
		stack.pop_back();
		if( ! stack.empty() )
			return stack.back() != prevValue;
		else
			return true;
	}
	else
		return true;
}

template<typename T>
bool DrawContext::setStackState( std::vector<T> &stack, T value )
{
	bool needsToBeSet = true;
	if( ( ! stack.empty() ) && ( stack.back() == value ) )
		needsToBeSet = false;
	else if( stack.empty() )
		stack.push_back( value );
	else
		stack.back() = value;
	return needsToBeSet;
}

template<typename T>
bool DrawContext::getStackState( std::vector<T> &stack, T *result )
{
	if( stack.empty() )
		return false;
	else {
		*result = stack.back();
		return true;
	}
}

}}