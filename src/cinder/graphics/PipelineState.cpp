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

#include "cinder/graphics/PipelineState.h"
#include "cinder/app/RendererGx.h"

namespace cinder { namespace graphics {

ShaderResourceVariableDesc::ShaderResourceVariableDesc( SHADER_TYPE shaderStages, const std::string &name, SHADER_RESOURCE_VARIABLE_TYPE type ) 
	: mName( name ), 
	Diligent::ShaderResourceVariableDesc( shaderStages, mName.c_str(), type ) 
{
	updatePtrs();
}

ShaderResourceVariableDesc::ShaderResourceVariableDesc( const ShaderResourceVariableDesc &other )
	: mName( other.mName ),
	Diligent::ShaderResourceVariableDesc( other.ShaderStages, other.Name, other.Type )
{
	updatePtrs();
}

ShaderResourceVariableDesc::ShaderResourceVariableDesc( ShaderResourceVariableDesc &&other ) noexcept
	: ShaderResourceVariableDesc()
{
	other.swap( *this );
	updatePtrs();
}

ShaderResourceVariableDesc& ShaderResourceVariableDesc::operator=( const ShaderResourceVariableDesc &other )
{
	ShaderResourceVariableDesc( other ).swap( *this );
	updatePtrs();
	return *this;
}

ShaderResourceVariableDesc& ShaderResourceVariableDesc::operator=( ShaderResourceVariableDesc &&other ) noexcept
{
	other.swap( *this );
	updatePtrs();
	return *this;
}

void ShaderResourceVariableDesc::updatePtrs() noexcept
{
	if( ! mName.empty() ) Name = mName.c_str();
}

void ShaderResourceVariableDesc::swap( ShaderResourceVariableDesc &other ) noexcept
{
	std::swap( mName, other.mName );
	std::swap( ShaderStages, other.ShaderStages );
	std::swap( Name, other.Name );
	std::swap( Type, other.Type );
}

ImmutableSamplerDesc::ImmutableSamplerDesc( const ImmutableSamplerDesc &other )
	: mSamplerOrTextureName( other.mSamplerOrTextureName ),
	Diligent::ImmutableSamplerDesc( other.ShaderStages, other.SamplerOrTextureName, other.Desc )
{
	updatePtrs();
}

ImmutableSamplerDesc::ImmutableSamplerDesc( ImmutableSamplerDesc &&other ) noexcept
	: ImmutableSamplerDesc()
{
	other.swap( *this );
	updatePtrs();
}

ImmutableSamplerDesc& ImmutableSamplerDesc::operator=( const ImmutableSamplerDesc &other )
{
	ImmutableSamplerDesc( other ).swap( *this );
	updatePtrs();
	return *this;
}

ImmutableSamplerDesc& ImmutableSamplerDesc::operator=( ImmutableSamplerDesc &&other ) noexcept
{
	other.swap( *this );
	updatePtrs();
	return *this;
}

void ImmutableSamplerDesc::updatePtrs() noexcept
{
	if( ! mSamplerOrTextureName.empty() ) SamplerOrTextureName = mSamplerOrTextureName.c_str();
}

void ImmutableSamplerDesc::swap( ImmutableSamplerDesc &other ) noexcept
{
	std::swap( mSamplerOrTextureName, other.mSamplerOrTextureName );
	std::swap( ShaderStages, other.ShaderStages );
	std::swap( SamplerOrTextureName, other.SamplerOrTextureName );
	std::swap( Desc, other.Desc );
}

GraphicsPipelineDesc::GraphicsPipelineDesc()
{
	mFlags = PSO_CREATE_FLAG_NONE;
	mPSODesc.PipelineType = Diligent::PIPELINE_TYPE_GRAPHICS;
	mGraphicsPipeline.RTVFormats[0] = app::getSwapChainColorFormat();
	mGraphicsPipeline.DSVFormat = app::getSwapChainDepthFormat();
	mGraphicsPipeline.NumRenderTargets = 1;
}

GraphicsPipelineDesc::GraphicsPipelineDesc( const GraphicsPipelineDesc &other )
	: mName( other.mName ),
	mVS( other.mVS ),
	mPS( other.mPS ),
	mDS( other.mDS ),
	mHS( other.mHS ),
	mGS( other.mGS ),
	mAS( other.mAS ),
	mMS( other.mMS ),
	mVSCreateInfo( other.mVSCreateInfo ),
	mPSCreateInfo( other.mPSCreateInfo ),
	mDSCreateInfo( other.mDSCreateInfo ),
	mHSCreateInfo( other.mHSCreateInfo ),
	mGSCreateInfo( other.mGSCreateInfo ),
	mASCreateInfo( other.mASCreateInfo ),
	mMSCreateInfo( other.mMSCreateInfo ),
	mLayoutElements( other.mLayoutElements ),
	mVariables( other.mVariables ),
	mImmutableSamplers( other.mImmutableSamplers ),
	mPSODesc( other.mPSODesc ),
	mFlags( other.mFlags ),
	mGraphicsPipeline( other.mGraphicsPipeline )
{
	updatePtrs();
}

GraphicsPipelineDesc::GraphicsPipelineDesc( GraphicsPipelineDesc &&other ) noexcept
	: GraphicsPipelineDesc()
{
	other.swap( *this );
	updatePtrs();
}

GraphicsPipelineDesc& GraphicsPipelineDesc::operator=( const GraphicsPipelineDesc &other )
{
	GraphicsPipelineDesc( other ).swap( *this );
	updatePtrs();
	return *this;
}

GraphicsPipelineDesc& GraphicsPipelineDesc::operator=( GraphicsPipelineDesc &&other ) noexcept
{
	other.swap( *this );
	updatePtrs();
	return *this;
}

void GraphicsPipelineDesc::updatePtrs() noexcept
{
	if( ! mName.empty() ) mPSODesc.Name = mName.c_str();

	if( ! mLayoutElements.empty() ) {
		mGraphicsPipeline.InputLayout.LayoutElements = mLayoutElements.data();
		mGraphicsPipeline.InputLayout.NumElements = static_cast<uint32_t>( mLayoutElements.size() );
	}

	if( ! mVariables.empty() ) {
		mVariablesBase.resize( mVariables.size() );
		for( size_t i = 0; i < mVariables.size(); ++i ) {
			mVariablesBase[i] = mVariables[i];
		}
		mPSODesc.ResourceLayout.Variables = mVariablesBase.data();
		mPSODesc.ResourceLayout.NumVariables = static_cast<uint32_t>( mVariablesBase.size() );
	}

	if( ! mImmutableSamplers.empty() ) {
		mImmutableSamplersBase.resize( mImmutableSamplers.size() );
		for( size_t i = 0; i < mImmutableSamplers.size(); ++i ) {
			mImmutableSamplersBase[i] = mImmutableSamplers[i];
		}
		mPSODesc.ResourceLayout.ImmutableSamplers = mImmutableSamplersBase.data();
		mPSODesc.ResourceLayout.NumImmutableSamplers = static_cast<uint32_t>( mImmutableSamplersBase.size() );
	}
}

GraphicsPipelineDesc& GraphicsPipelineDesc::inputLayout( const std::vector<LayoutElement> &elements ) 
{ 
	mLayoutElements = elements; 
	mGraphicsPipeline.InputLayout.LayoutElements = mLayoutElements.data(); 
	mGraphicsPipeline.InputLayout.NumElements = static_cast<Diligent::Uint32>( mLayoutElements.size() ); 
	return *this; 
}

GraphicsPipelineDesc& GraphicsPipelineDesc::variables( const std::vector<ShaderResourceVariableDesc> &variables ) 
{ 
	mVariables = variables;
	mVariablesBase.resize( mVariables.size() );
	for( size_t i = 0; i < mVariables.size(); ++i ) {
		mVariablesBase[i] = mVariables[i];
	}
	mPSODesc.ResourceLayout.Variables = mVariablesBase.data();
	mPSODesc.ResourceLayout.NumVariables = static_cast<uint32_t>( mVariablesBase.size() );
	return *this; 
}

GraphicsPipelineDesc& GraphicsPipelineDesc::immutableSamplers( const std::vector<ImmutableSamplerDesc> &immutableSamplers ) 
{ 
	mImmutableSamplers = immutableSamplers;
	mImmutableSamplersBase.resize( mImmutableSamplers.size() );
	for( size_t i = 0; i < mImmutableSamplers.size(); ++i ) {
		mImmutableSamplersBase[i] = mImmutableSamplers[i];
	}
	mPSODesc.ResourceLayout.ImmutableSamplers = mImmutableSamplersBase.data();
	mPSODesc.ResourceLayout.NumImmutableSamplers = static_cast<uint32_t>( mImmutableSamplersBase.size() );
	return *this; 
}

void GraphicsPipelineDesc::swap( GraphicsPipelineDesc &other ) noexcept
{
	std::swap( mPSODesc, other.mPSODesc );
	std::swap( mFlags, other.mFlags );
	std::swap( mGraphicsPipeline, other.mGraphicsPipeline );

	std::swap( mName, other.mName );
	std::swap( mVS, other.mVS );
	std::swap( mPS, other.mPS );
	std::swap( mDS, other.mDS );
	std::swap( mHS, other.mHS );
	std::swap( mGS, other.mGS );
	std::swap( mAS, other.mAS );
	std::swap( mMS, other.mMS );
	std::swap( mVSCreateInfo, other.mVSCreateInfo );
	std::swap( mPSCreateInfo, other.mPSCreateInfo );
	std::swap( mDSCreateInfo, other.mDSCreateInfo );
	std::swap( mHSCreateInfo, other.mHSCreateInfo );
	std::swap( mGSCreateInfo, other.mGSCreateInfo );
	std::swap( mASCreateInfo, other.mASCreateInfo );
	std::swap( mMSCreateInfo, other.mMSCreateInfo );
	std::swap( mLayoutElements, other.mLayoutElements );
	std::swap( mVariables, other.mVariables );
	std::swap( mImmutableSamplers, other.mImmutableSamplers );
}

PipelineStateRef createGraphicsPipelineState( const gx::GraphicsPipelineDesc &pipelineDesc )
{
	return createGraphicsPipelineState( app::getRenderDevice(), pipelineDesc );
}

PipelineStateRef createGraphicsPipelineState( RenderDevice* device, const gx::GraphicsPipelineDesc &pipelineDesc )
{
	PipelineStateRef pipelineState;

	// not sure if there's a way to avoid a copy here?
	gx::GraphicsPipelineStateCreateInfo createInfo;
	createInfo.Flags = pipelineDesc.mFlags;
	createInfo.GraphicsPipeline = pipelineDesc.mGraphicsPipeline;
	createInfo.PSODesc = pipelineDesc.mPSODesc;

	// initialize shaders if no pointer is present but a ShaderCreateInfo is available
	ShaderRef vs, ps, ds, hs, gs, as, ms;
	if( ! createInfo.pVS && pipelineDesc.mVSCreateInfo.Desc.ShaderType != SHADER_TYPE_UNKNOWN ) {
		device->CreateShader( pipelineDesc.mVSCreateInfo, &vs );
		createInfo.pVS = vs;
	}
	if( ! createInfo.pPS && pipelineDesc.mPSCreateInfo.Desc.ShaderType != SHADER_TYPE_UNKNOWN ) {
		device->CreateShader( pipelineDesc.mPSCreateInfo, &ps );
		createInfo.pPS = ps;
	}
	if( ! createInfo.pDS && pipelineDesc.mDSCreateInfo.Desc.ShaderType != SHADER_TYPE_UNKNOWN ) {
		device->CreateShader( pipelineDesc.mDSCreateInfo, &ds );
		createInfo.pDS = ds;
	}
	if( ! createInfo.pHS && pipelineDesc.mHSCreateInfo.Desc.ShaderType != SHADER_TYPE_UNKNOWN ) {
		device->CreateShader( pipelineDesc.mHSCreateInfo, &hs );
		createInfo.pHS = hs;
	}
	if( ! createInfo.pGS && pipelineDesc.mGSCreateInfo.Desc.ShaderType != SHADER_TYPE_UNKNOWN ) {
		device->CreateShader( pipelineDesc.mGSCreateInfo, &gs );
		createInfo.pGS = gs;
	}
	if( ! createInfo.pAS && pipelineDesc.mASCreateInfo.Desc.ShaderType != SHADER_TYPE_UNKNOWN ) {
		device->CreateShader( pipelineDesc.mASCreateInfo, &as );
		createInfo.pAS = as;
	}
	if( ! createInfo.pMS && pipelineDesc.mMSCreateInfo.Desc.ShaderType != SHADER_TYPE_UNKNOWN ) {
		device->CreateShader( pipelineDesc.mMSCreateInfo, &ms );
		createInfo.pMS = ms;
	}

	// set shader pointers
	if( pipelineDesc.mVS ) createInfo.pVS = pipelineDesc.mVS.RawPtr<Shader>();
	if( pipelineDesc.mPS ) createInfo.pPS = pipelineDesc.mPS.RawPtr<Shader>();
	if( pipelineDesc.mDS ) createInfo.pDS = pipelineDesc.mDS.RawPtr<Shader>();
	if( pipelineDesc.mHS ) createInfo.pHS = pipelineDesc.mHS.RawPtr<Shader>();
	if( pipelineDesc.mGS ) createInfo.pGS = pipelineDesc.mGS.RawPtr<Shader>();
	if( pipelineDesc.mAS ) createInfo.pAS = pipelineDesc.mAS.RawPtr<Shader>();
	if( pipelineDesc.mMS ) createInfo.pMS = pipelineDesc.mMS.RawPtr<Shader>();

	device->CreateGraphicsPipelineState( createInfo, &pipelineState );
	return pipelineState;
}

}

namespace gx = graphics;
} // namespace cinder::graphics