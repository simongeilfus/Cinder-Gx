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

#include "cinder/graphics/wrapper.h"
#include "cinder/graphics/PipelineState.h"
#include "cinder/app/RendererGx.h"

using namespace std;
using namespace Diligent;
using namespace ci::app;
	
namespace cinder { namespace graphics {

void setPipelineState( PipelineState* pipelineState )
{
	getImmediateContext()->SetPipelineState( pipelineState );
}

void transitionShaderResources( PipelineState* pipelineState, ShaderResourceBinding* shaderResourceBinding )
{
	getImmediateContext()->TransitionShaderResources( pipelineState, shaderResourceBinding );
}

void commitShaderResources( ShaderResourceBinding* shaderResourceBinding, RESOURCE_STATE_TRANSITION_MODE stateTransitionMode )
{
	getImmediateContext()->CommitShaderResources( shaderResourceBinding, stateTransitionMode );
}

void setStencilRef( uint32_t StencilRef )
{
	getImmediateContext()->SetStencilRef( StencilRef );
}

void setBlendFactors( const float* pBlendFactors )
{
	getImmediateContext()->SetBlendFactors( pBlendFactors );
}

void invalidateState()
{
	getImmediateContext()->InvalidateState();
}

void setVertexBuffer( Buffer* buffer, RESOURCE_STATE_TRANSITION_MODE stateTransitionMode, SET_VERTEX_BUFFERS_FLAGS flags )
{
	uint32_t offset = 0;
	Buffer* buffers[] = { buffer };
	getImmediateContext()->SetVertexBuffers( 0, 1, buffers, &offset, stateTransitionMode, flags );
}

void setVertexBuffers( uint32_t startSlot, uint32_t numBuffersSet, Buffer** ppBuffers, uint32_t* pOffsets, RESOURCE_STATE_TRANSITION_MODE stateTransitionMode, SET_VERTEX_BUFFERS_FLAGS flags )
{
	getImmediateContext()->SetVertexBuffers( startSlot, numBuffersSet, ppBuffers, pOffsets, stateTransitionMode, flags );
}

void setIndexBuffer( Buffer* indexBuffer, uint32_t byteOffset, RESOURCE_STATE_TRANSITION_MODE stateTransitionMode )
{
	getImmediateContext()->SetIndexBuffer( indexBuffer, byteOffset, stateTransitionMode );
}

void setViewports( uint32_t NumViewports, const Viewport* pViewports, uint32_t RTWidth, uint32_t RTHeight )
{
	getImmediateContext()->SetViewports( NumViewports, pViewports, RTWidth, RTHeight );
}

void setScissorRects( uint32_t NumRects, const Rect* pRects, uint32_t RTWidth, uint32_t RTHeight )
{
	getImmediateContext()->SetScissorRects( NumRects, pRects, RTWidth, RTHeight );
}

void setRenderTargets( uint32_t NumRenderTargets, TextureView* ppRenderTargets[], TextureView* pDepthStencil, RESOURCE_STATE_TRANSITION_MODE StateTransitionMode )
{
	getImmediateContext()->SetRenderTargets( NumRenderTargets, ppRenderTargets, pDepthStencil, StateTransitionMode );
}

void beginRenderPass( const BeginRenderPassAttribs &Attribs )
{
	getImmediateContext()->BeginRenderPass( Attribs );
}

void nextSubpass()
{
	getImmediateContext()->NextSubpass();
}

void endRenderPass()
{
	getImmediateContext()->EndRenderPass();
}

void draw( const Diligent::DrawAttribs &attribs )
{
	getImmediateContext()->Draw( attribs );
}

void drawIndexed( const Diligent::DrawIndexedAttribs &attribs )
{
	getImmediateContext()->DrawIndexed( attribs );
}

void drawIndexedIndirect( const DrawIndexedIndirectAttribs &attribs )
{
	//getImmediateContext()->DrawIndexedIndirect( attribs );
}

void drawMesh( const DrawMeshAttribs &attribs )
{
	getImmediateContext()->DrawMesh( attribs );
}

void drawMeshIndirect( const DrawMeshIndirectAttribs &attribs )
{
	//getImmediateContext()->DrawMeshIndirect( attribs );
}

void dispatchCompute( const DispatchComputeAttribs &Attribs )
{
    getImmediateContext()->DispatchCompute( Attribs );
}

void dispatchComputeIndirect( const DispatchComputeIndirectAttribs &Attribs, Buffer* pAttribsBuffer )
{
    getImmediateContext()->DispatchComputeIndirect( Attribs, pAttribsBuffer );
}

void clear( const ColorA& color, bool clearDepthBuffer )
{
	if( clearDepthBuffer ) {
		auto* pRTV = getSwapChain()->GetCurrentBackBufferRTV();
		auto* pDSV = getSwapChain()->GetDepthBufferDSV();
		getImmediateContext()->ClearRenderTarget( pRTV, &color[0], RESOURCE_STATE_TRANSITION_MODE_TRANSITION );
		getImmediateContext()->ClearDepthStencil( pDSV, CLEAR_DEPTH_FLAG, 1.f, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION );
	}
	else {
		auto* pRTV = getSwapChain()->GetCurrentBackBufferRTV();
		getImmediateContext()->ClearRenderTarget( pRTV, &color[0], RESOURCE_STATE_TRANSITION_MODE_TRANSITION );
	}
}

void clearRenderTarget( const ColorA &color )
{
	auto* pRTV = getSwapChain()->GetCurrentBackBufferRTV();
	getImmediateContext()->ClearRenderTarget( pRTV, &color[0], RESOURCE_STATE_TRANSITION_MODE_TRANSITION );
}

void clearDepth( const float depth )
{
	auto* pDSV = getSwapChain()->GetDepthBufferDSV();
	getImmediateContext()->ClearDepthStencil( pDSV, CLEAR_DEPTH_FLAG, depth, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION );
}

void clearStencil( const int s )
{
	auto* pDSV = getSwapChain()->GetDepthBufferDSV();
	getImmediateContext()->ClearDepthStencil( pDSV, CLEAR_STENCIL_FLAG, 1.0f, s, RESOURCE_STATE_TRANSITION_MODE_TRANSITION );
}

void clearDepthStencil( const float depth, const int s )
{
	auto* pDSV = getSwapChain()->GetDepthBufferDSV();
	getImmediateContext()->ClearDepthStencil( pDSV, CLEAR_DEPTH_FLAG | CLEAR_STENCIL_FLAG, depth, s, RESOURCE_STATE_TRANSITION_MODE_TRANSITION );
}

void clearDepthStencil( TextureView* pView, CLEAR_DEPTH_STENCIL_FLAGS ClearFlags, float fDepth, uint8_t Stencil, RESOURCE_STATE_TRANSITION_MODE StateTransitionMode )
{
    getImmediateContext()->ClearDepthStencil( pView, ClearFlags, fDepth, Stencil, StateTransitionMode );
}

void clearRenderTarget( TextureView* pView, const float* RGBA, RESOURCE_STATE_TRANSITION_MODE StateTransitionMode )
{
    getImmediateContext()->ClearRenderTarget( pView, RGBA, StateTransitionMode );
}

void finishCommandList( ICommandList** ppCommandList )
{
    getImmediateContext()->FinishCommandList( ppCommandList );
}

void executeCommandLists( uint32_t NumCommandLists, ICommandList* const* ppCommandLists )
{
    getImmediateContext()->ExecuteCommandLists( NumCommandLists, ppCommandLists );
}

void signalFence( IFence* pFence, uint64_t Value )
{
    getImmediateContext()->SignalFence( pFence, Value );
}

void waitForFence( IFence* pFence, uint64_t Value, bool FlushContext )
{
    getImmediateContext()->WaitForFence( pFence, Value, FlushContext );
}

void waitForIdle()
{
    getImmediateContext()->WaitForIdle();
}

void beginQuery( IQuery* pQuery )
{
    getImmediateContext()->BeginQuery( pQuery );
}

void endQuery( IQuery* pQuery )
{
    getImmediateContext()->EndQuery( pQuery );
}

void flush()
{
    getImmediateContext()->Flush();
}

void updateBuffer( Buffer* pBuffer, uint32_t Offset, uint32_t Size, const void* pData, RESOURCE_STATE_TRANSITION_MODE StateTransitionMode )
{
    getImmediateContext()->UpdateBuffer( pBuffer, Offset, Size, pData, StateTransitionMode );
}

void copyBuffer( Buffer* pSrcBuffer, uint32_t SrcOffset, RESOURCE_STATE_TRANSITION_MODE SrcBufferTransitionMode, Buffer* pDstBuffer, uint32_t DstOffset, uint32_t Size, RESOURCE_STATE_TRANSITION_MODE DstBufferTransitionMode )
{
    getImmediateContext()->CopyBuffer( pSrcBuffer, SrcOffset, SrcBufferTransitionMode, pDstBuffer, DstOffset, Size, DstBufferTransitionMode );
}

void mapBuffer( Buffer* pBuffer, MAP_TYPE MapType, MAP_FLAGS MapFlags, PVoid &pMappedData )
{
    getImmediateContext()->MapBuffer( pBuffer, MapType, MapFlags, pMappedData );
}

void unmapBuffer( Buffer* pBuffer, MAP_TYPE MapType )
{
    getImmediateContext()->UnmapBuffer( pBuffer, MapType );
}

void updateTexture( Texture* pTexture, uint32_t MipLevel, uint32_t Slice, const Box &DstBox, const TextureSubResData &SubresData, RESOURCE_STATE_TRANSITION_MODE SrcBufferTransitionMode, RESOURCE_STATE_TRANSITION_MODE TextureTransitionMode )
{
    getImmediateContext()->UpdateTexture( pTexture, MipLevel, Slice, DstBox, SubresData, SrcBufferTransitionMode, TextureTransitionMode );
}

void copyTexture( const CopyTextureAttribs &CopyAttribs )
{
    getImmediateContext()->CopyTexture( CopyAttribs );
}

void mapTextureSubresource( Texture* pTexture, uint32_t MipLevel, uint32_t ArraySlice, MAP_TYPE MapType, MAP_FLAGS MapFlags, const Box* pMapRegion, MappedTextureSubresource &MappedData )
{
    getImmediateContext()->MapTextureSubresource( pTexture, MipLevel, ArraySlice, MapType, MapFlags, pMapRegion, MappedData );
}

void unmapTextureSubresource( Texture* pTexture, uint32_t MipLevel, uint32_t ArraySlice )
{
    getImmediateContext()->UnmapTextureSubresource( pTexture, MipLevel, ArraySlice );
}

void generateMips( TextureView* pTextureView )
{
    getImmediateContext()->GenerateMips( pTextureView );
}

void finishFrame()
{
    getImmediateContext()->FinishFrame();
}

uint64_t getFrameNumber()
{
    return getImmediateContext()->GetFrameNumber();
}

void transitionResourceStates( uint32_t BarrierCount, StateTransitionDesc* pResourceBarriers )
{
    getImmediateContext()->TransitionResourceStates( BarrierCount, pResourceBarriers );
}

void resolveTextureSubresource( Texture* pSrcTexture, Texture* pDstTexture, const ResolveTextureSubresourceAttribs &ResolveAttribs )
{
    getImmediateContext()->ResolveTextureSubresource( pSrcTexture, pDstTexture, ResolveAttribs );
}

void buildBLAS( const BuildBLASAttribs &Attribs )
{
    getImmediateContext()->BuildBLAS( Attribs );
}

void buildTLAS( const BuildTLASAttribs &Attribs )
{
    getImmediateContext()->BuildTLAS( Attribs );
}

void copyBLAS( const CopyBLASAttribs &Attribs )
{
    getImmediateContext()->CopyBLAS( Attribs );
}

void copyTLAS( const CopyTLASAttribs &Attribs )
{
    getImmediateContext()->CopyTLAS( Attribs );
}

void writeBLASCompactedSize( const WriteBLASCompactedSizeAttribs &Attribs )
{
    getImmediateContext()->WriteBLASCompactedSize( Attribs );
}

void writeTLASCompactedSize( const WriteTLASCompactedSizeAttribs &Attribs )
{
    getImmediateContext()->WriteTLASCompactedSize( Attribs );
}

void traceRays( const TraceRaysAttribs &Attribs )
{
    getImmediateContext()->TraceRays( Attribs );
}

void enableVerticalSync( bool enable )
{
	std::static_pointer_cast<RendererGx>( app::App::get()->getRenderer() )->enableVerticalSync( enable );
}

bool isVerticalSyncEnabled() 
{
	return std::static_pointer_cast<RendererGx>( app::App::get()->getRenderer() )->isVerticalSyncEnabled();
}

SamplerRef createSampler( const Diligent::SamplerDesc &samDesc )
{
	SamplerRef sampler;
	getRenderDevice()->CreateSampler( samDesc, &sampler );
	return sampler;
}


ResourceMappingRef createResourceMapping( const Diligent::ResourceMappingDesc &mappingDesc )
{
	ResourceMappingRef resourceMapping;
	getRenderDevice()->CreateResourceMapping( mappingDesc, &resourceMapping );
	return resourceMapping;
}

PipelineStateRef createGraphicsPipelineState( const Diligent::GraphicsPipelineStateCreateInfo &psoCreateInfo )
{
	PipelineStateRef pipelineState;
	getRenderDevice()->CreateGraphicsPipelineState( psoCreateInfo, &pipelineState );
	return pipelineState;
}

PipelineStateRef createComputePipelineState( const Diligent::ComputePipelineStateCreateInfo &psoCreateInfo )
{
	PipelineStateRef pipelineState;
	getRenderDevice()->CreateComputePipelineState( psoCreateInfo, &pipelineState );
	return pipelineState;
}

PipelineStateRef createRayTracingPipelineState( const Diligent::RayTracingPipelineStateCreateInfo &psoCreateInfo )
{
	PipelineStateRef pipelineState;
	getRenderDevice()->CreateRayTracingPipelineState( psoCreateInfo, &pipelineState );
	return pipelineState;
}

FenceRef createFence( const Diligent::FenceDesc &desc )
{
	FenceRef fence;
	getRenderDevice()->CreateFence( desc, &fence );
	return fence;
}

QueryRef createQuery( const Diligent::QueryDesc &desc )
{
	QueryRef query;
	getRenderDevice()->CreateQuery( desc, &query );
	return query;
}

RenderPassRef createRenderPass( const Diligent::RenderPassDesc &desc )
{
	RenderPassRef renderPass;
	getRenderDevice()->CreateRenderPass( desc, &renderPass );
	return renderPass;
}

FramebufferRef createFramebuffer( const Diligent::FramebufferDesc &desc )
{
	FramebufferRef framebuffer;
	getRenderDevice()->CreateFramebuffer( desc, &framebuffer );
	return framebuffer;
}

BottomLevelASRef createBLAS( const Diligent::BottomLevelASDesc &desc )
{
	BottomLevelASRef blas;
	getRenderDevice()->CreateBLAS( desc, &blas );
	return blas;
}

TopLevelASRef createTLAS( const Diligent::TopLevelASDesc &desc )
{
	TopLevelASRef tlas;
	getRenderDevice()->CreateTLAS( desc, &tlas );
	return tlas;
}

ShaderBindingTableRef createSBT( const Diligent::ShaderBindingTableDesc &desc )
{
	ShaderBindingTableRef sbt;
	getRenderDevice()->CreateSBT( desc, &sbt );
	return sbt;
}

const DeviceCaps& getDeviceCaps()
{
	return getRenderDevice()->GetDeviceCaps();
}

const DeviceProperties& getDeviceProperties()
{
	return getRenderDevice()->GetDeviceProperties();
}

const TextureFormatInfo& getTextureFormatInfo( TEXTURE_FORMAT texFormat )
{
	return getRenderDevice()->GetTextureFormatInfo( texFormat );
}

const TextureFormatInfoExt& getTextureFormatInfoExt( TEXTURE_FORMAT texFormat )
{
	return getRenderDevice()->GetTextureFormatInfoExt( texFormat );
}

void releaseStaleResources( bool forceRelease )
{
	return getRenderDevice()->ReleaseStaleResources( forceRelease );
}

void idleGPU()
{
	return getRenderDevice()->IdleGPU();
}

EngineFactory* getEngineFactory()
{
	return getRenderDevice()->GetEngineFactory();
}


} } // namespace cinder::graphics
