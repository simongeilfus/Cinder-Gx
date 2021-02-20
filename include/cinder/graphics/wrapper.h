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

#pragma once

#include "cinder/Color.h"
#include "cinder/graphics/platform.h"
#include "cinder/graphics/GraphicsTypes.h"

#include "DiligentCore/Graphics/GraphicsEngine/interface/EngineFactory.h"
#include "DiligentCore/Graphics/GraphicsEngine/interface/RenderDevice.h"
#include "DiligentCore/Graphics/GraphicsEngine/interface/DeviceContext.h"
#include "DiligentCore/Graphics/GraphicsEngine/interface/SwapChain.h"
#include "DiligentCore/Graphics/GraphicsTools/interface/MapHelper.hpp"
#include "DiligentCore/Common/interface/RefCntAutoPtr.hpp"
	
namespace cinder { namespace graphics {

using namespace Diligent;

using EngineFactory = Diligent::IEngineFactory;
using EngineFactoryRef = Diligent::RefCntAutoPtr<Diligent::IEngineFactory>;
using RenderDevice = Diligent::IRenderDevice;
using RenderDeviceRef = Diligent::RefCntAutoPtr<Diligent::IRenderDevice>;
using DeviceContext = Diligent::IDeviceContext;
using DeviceContextRef = Diligent::RefCntAutoPtr<Diligent::IDeviceContext>;
using SwapChain = Diligent::ISwapChain;
using SwapChainRef = Diligent::RefCntAutoPtr<Diligent::ISwapChain>;
using Buffer = Diligent::IBuffer;
using BufferRef = Diligent::RefCntAutoPtr<Diligent::IBuffer>;
using Texture = Diligent::ITexture;
using TextureRef = Diligent::RefCntAutoPtr<Diligent::ITexture>;
using TextureView = Diligent::ITextureView;
using TextureViewRef = Diligent::RefCntAutoPtr<Diligent::ITextureView>;
using Sampler = Diligent::ISampler;
using SamplerRef = Diligent::RefCntAutoPtr<Diligent::ISampler>;
using ResourceMapping = Diligent::IResourceMapping;
using ResourceMappingRef = Diligent::RefCntAutoPtr<Diligent::IResourceMapping>;
using PipelineState = Diligent::IPipelineState;
using PipelineStateRef = Diligent::RefCntAutoPtr<Diligent::IPipelineState>;
using CommandList = Diligent::ICommandList;
using CommandListRef = Diligent::RefCntAutoPtr<Diligent::ICommandList>;
using Fence = Diligent::IFence;
using FenceRef = Diligent::RefCntAutoPtr<Diligent::IFence>;
using Query = Diligent::IQuery;
using QueryRef = Diligent::RefCntAutoPtr<Diligent::IQuery>;
using RenderPass = Diligent::IRenderPass;
using RenderPassRef = Diligent::RefCntAutoPtr<Diligent::IRenderPass>;
using Framebuffer = Diligent::IFramebuffer;
using FramebufferRef = Diligent::RefCntAutoPtr<Diligent::IFramebuffer>;
using BottomLevelAS = Diligent::IBottomLevelAS;
using BottomLevelASRef = Diligent::RefCntAutoPtr<Diligent::IBottomLevelAS>;
using TopLevelAS = Diligent::ITopLevelAS;
using TopLevelASRef = Diligent::RefCntAutoPtr<Diligent::ITopLevelAS>;
using Shader = Diligent::IShader;
using ShaderRef = Diligent::RefCntAutoPtr<Diligent::IShader>;
using ShaderBindingTable = Diligent::IShaderBindingTable;
using ShaderBindingTableRef = Diligent::RefCntAutoPtr<Diligent::IShaderBindingTable>;
using ShaderResourceBinding = Diligent::IShaderResourceBinding;
using ShaderResourceBindingRef = Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding>;
using ShaderSourceInputStreamFactory = Diligent::IShaderSourceInputStreamFactory;
using ShaderSourceInputStreamFactoryRef = Diligent::RefCntAutoPtr<Diligent::IShaderSourceInputStreamFactory>;

//! Sets the pipeline state.
CI_API void setPipelineState( PipelineState* pipelineState );
//! Transitions shader resources to the states required by Draw or Dispatch command.
CI_API void transitionShaderResources( PipelineState* pipelineState, ShaderResourceBinding* shaderResourceBinding );
//! Commits shader resources to the device context.
CI_API void commitShaderResources( ShaderResourceBinding* shaderResourceBinding, RESOURCE_STATE_TRANSITION_MODE stateTransitionMode );

//! Sets the stencil reference value.
CI_API void setStencilRef( uint32_t StencilRef);
//! \param [in] pBlendFactors - Array of four blend factors, one for each RGBA component. 
CI_API void setBlendFactors( const float* pBlendFactors = nullptr );
//! Invalidates the cached context state.
CI_API void invalidateState();

//! Binds vertex buffers to the pipeline.
CI_API void setVertexBuffer( Buffer* buffer, RESOURCE_STATE_TRANSITION_MODE stateTransitionMode, SET_VERTEX_BUFFERS_FLAGS flags );
//! Binds vertex buffers to the pipeline.
CI_API void setVertexBuffers( uint32_t startSlot, uint32_t numBuffersSet, Buffer** ppBuffers, uint32_t* pOffsets, RESOURCE_STATE_TRANSITION_MODE stateTransitionMode, SET_VERTEX_BUFFERS_FLAGS flags );
//! Binds an index buffer to the pipeline.
CI_API void setIndexBuffer( Buffer* indexBuffer, uint32_t byteOffset, RESOURCE_STATE_TRANSITION_MODE stateTransitionMode );

//! Sets an array of viewports.
CI_API void setViewports( uint32_t NumViewports, const Viewport* pViewports, uint32_t RTWidth, uint32_t RTHeight );
//! Sets active scissor rects.
CI_API void setScissorRects( uint32_t NumRects, const Rect* pRects, uint32_t RTWidth, uint32_t RTHeight );
//! Binds one or more render targets and the depth-stencil buffer to the context. It also sets the viewport to match the first non-null render target or depth-stencil buffer.
CI_API void setRenderTargets( uint32_t NumRenderTargets, TextureView* ppRenderTargets[], TextureView* pDepthStencil, RESOURCE_STATE_TRANSITION_MODE StateTransitionMode );
//! Begins a new render pass.
CI_API void beginRenderPass( const BeginRenderPassAttribs &Attribs );
//! Transitions to the next subpass in the render pass instance.
CI_API void nextSubpass();
//! Ends current render pass.
CI_API void endRenderPass();

struct CI_API DrawAttribs : public Diligent::DrawAttribs {
    //! The number of vertices to draw.
    DrawAttribs& numVertices( uint32_t count ) { NumVertices = count; return *this; }
    //! Additional flags, see Diligent::DRAW_FLAGS.
    DrawAttribs& flags( DRAW_FLAGS flags ) { Flags = flags; return *this; }
    //! The number of instances to draw. If more than one instance is specified, instanced draw call will be performed.
    DrawAttribs& numInstances( uint32_t count ) { NumInstances = count; return *this; }
    //! LOCATION (or INDEX, but NOT the byte offset) of the first vertex in the vertex buffer to start reading vertices from.
    DrawAttribs& startVertexLocation( uint32_t index ) { StartVertexLocation = index; return *this; }
    //! LOCATION (or INDEX, but NOT the byte offset) in the vertex buffer to start reading instance data from.
    DrawAttribs& firstInstanceLocation( uint32_t index ) { FirstInstanceLocation = index; return *this; }
};

//! Executes a draw command
CI_API void draw( const Diligent::DrawAttribs &attribs );

//! Defines the indexed draw command attributes.
struct DrawIndexedAttribs : public Diligent::DrawIndexedAttribs {
    //! The number of indices to draw.
    DrawIndexedAttribs& numIndices( uint32_t numIndices ) { NumIndices = numIndices; return *this; }
    //! The type of elements in the index buffer. Allowed values: VT_UINT16 and VT_UINT32.
    DrawIndexedAttribs& indexType( VALUE_TYPE indexType ) { IndexType = indexType; return *this; }
    //! Additional flags, see Diligent::DRAW_FLAGS.
    DrawIndexedAttribs& flags( DRAW_FLAGS flags ) { Flags = flags; return *this; }
    //! Number of instances to draw. If more than one instance is specified, instanced draw call will be performed.
    DrawIndexedAttribs& numInstances( uint32_t numInstances ) { NumInstances = numInstances; return *this; }
    //! LOCATION (NOT the byte offset) of the first index in the index buffer to start reading indices from.
    DrawIndexedAttribs& firstIndexLocation( uint32_t firstIndexLocation ) { FirstIndexLocation = firstIndexLocation; return *this; }
    //! A constant which is added to each index before accessing the vertex buffer.
    DrawIndexedAttribs& baseVertex( uint32_t baseVertex ) { BaseVertex = baseVertex; return *this; }
    //! LOCATION (or INDEX, but NOT the byte offset) in the vertex buffer to start reading instance data from.
    DrawIndexedAttribs& firstInstanceLocation( uint32_t firstInstanceLocation ) { FirstInstanceLocation = firstInstanceLocation; return *this; }
};

//! Executes an indexed draw command
CI_API void drawIndexed( const Diligent::DrawIndexedAttribs &attribs );
//! Executes an indexed indirect draw command
CI_API void drawIndexedIndirect( const Diligent::DrawIndexedIndirectAttribs &attribs );
//! Executes a mesh draw command
CI_API void drawMesh( const Diligent::DrawMeshAttribs &attribs );
//! Executes a mesh indirect draw command
CI_API void drawMeshIndirect( const Diligent::DrawMeshIndirectAttribs &attribs );
//! Executes a dispatch compute command.
CI_API void dispatchCompute( const DispatchComputeAttribs &Attribs );
//! Executes an indirect dispatch compute command.
CI_API void dispatchComputeIndirect( const DispatchComputeIndirectAttribs &Attribs, Buffer* pAttribsBuffer );

//! Clears a render target view and depth-stencil view
CI_API void clear( const ColorA &color = ColorA::black(), bool clearDepthBuffer = true );
//! Clears a render target view
CI_API void clearRenderTarget( const ColorA &color );
//! Clears a depth-stencil view
CI_API void clearDepth( const float depth );
//! Clears a depth-stencil view
CI_API void clearStencil( const int s );
//! Clears a depth-stencil view.
CI_API void clearDepthStencil( TextureView* pView, CLEAR_DEPTH_STENCIL_FLAGS ClearFlags, float fDepth, uint8_t Stencil, RESOURCE_STATE_TRANSITION_MODE StateTransitionMode );
//! Clears a render target view
CI_API void clearRenderTarget( TextureView* pView, const float* RGBA, RESOURCE_STATE_TRANSITION_MODE StateTransitionMode );

//! Finishes recording commands and generates a command list.
CI_API void finishCommandList( ICommandList** ppCommandList );
//! Submits an array of recorded command lists for execution.
CI_API void executeCommandLists( uint32_t NumCommandLists, ICommandList* const* ppCommandLists );
//! Tells the GPU to set a fence to a specified value after all previous work has completed.
CI_API void signalFence( IFence* pFence, uint64_t Value );
//! Waits until the specified fence reaches or exceeds the specified value, on the host.
CI_API void waitForFence( IFence* pFence, uint64_t Value, bool FlushContext );
//! Submits all outstanding commands for execution to the GPU and waits until they are complete.
CI_API void waitForIdle();
//! Marks the beginning of a query.
CI_API void beginQuery( IQuery* pQuery );
//! Marks the end of a query.
CI_API void endQuery( IQuery* pQuery );
//! Submits all pending commands in the context for execution to the command queue.
CI_API void flush();
//! Updates the data in the buffer.
CI_API void updateBuffer( Buffer* pBuffer, uint32_t Offset, uint32_t Size, const void* pData, RESOURCE_STATE_TRANSITION_MODE StateTransitionMode );
//! Copies the data from one buffer to another.
CI_API void copyBuffer( Buffer* pSrcBuffer, uint32_t SrcOffset, RESOURCE_STATE_TRANSITION_MODE SrcBufferTransitionMode, Buffer* pDstBuffer, uint32_t DstOffset, uint32_t Size, RESOURCE_STATE_TRANSITION_MODE DstBufferTransitionMode );
//! Maps the buffer.
CI_API void mapBuffer( Buffer* pBuffer, MAP_TYPE MapType, MAP_FLAGS MapFlags, PVoid &pMappedData );
//! Unmaps the previously mapped buffer.
CI_API void unmapBuffer( Buffer* pBuffer, MAP_TYPE MapType );
//! Updates the data in the texture.
CI_API void updateTexture( Texture* pTexture, uint32_t MipLevel, uint32_t Slice, const Box &DstBox, const TextureSubResData &SubresData, RESOURCE_STATE_TRANSITION_MODE SrcBufferTransitionMode, RESOURCE_STATE_TRANSITION_MODE TextureTransitionMode );
//! Copies data from one texture to another.
CI_API void copyTexture( const CopyTextureAttribs &CopyAttribs );
//! Maps the texture subresource.
CI_API void mapTextureSubresource( Texture* pTexture, uint32_t MipLevel, uint32_t ArraySlice, MAP_TYPE MapType, MAP_FLAGS MapFlags, const Box* pMapRegion, MappedTextureSubresource &MappedData );
//! Unmaps the texture subresource.
CI_API void unmapTextureSubresource( Texture* pTexture, uint32_t MipLevel, uint32_t ArraySlice );
//! Generates a mipmap chain.
CI_API void generateMips( TextureView* pTextureView );
//! Finishes the current frame and releases dynamic resources allocated by the context.
CI_API void finishFrame();
//! Returns the current frame number.
CI_API uint64_t getFrameNumber();
//! Transitions resource states.
CI_API void transitionResourceStates( uint32_t BarrierCount, StateTransitionDesc* pResourceBarriers );
//! Resolves a multi-sampled texture subresource into a non-multi-sampled texture subresource.
CI_API void resolveTextureSubresource( Texture* pSrcTexture, Texture* pDstTexture, const ResolveTextureSubresourceAttribs &ResolveAttribs );
//! Builds a bottom-level acceleration structure with the specified geometries.
CI_API void buildBLAS( const BuildBLASAttribs &Attribs );
//! Builds a top-level acceleration structure with the specified instances.
CI_API void buildTLAS( const BuildTLASAttribs &Attribs );
//! Copies data from one acceleration structure to another.
CI_API void copyBLAS( const CopyBLASAttribs &Attribs );
//! Copies data from one acceleration structure to another.
CI_API void copyTLAS( const CopyTLASAttribs &Attribs );
//! Writes a bottom-level acceleration structure memory size required for compacting operation to a buffer.
CI_API void writeBLASCompactedSize( const WriteBLASCompactedSizeAttribs &Attribs );
//! Writes a top-level acceleration structure memory size required for compacting operation to a buffer.
CI_API void writeTLASCompactedSize( const WriteTLASCompactedSizeAttribs &Attribs );
//! Executes a trace rays command.
CI_API void traceRays( const TraceRaysAttribs &Attribs );


CI_API void enableVerticalSync( bool enable = true );
CI_API bool isVerticalSyncEnabled();

//! Creates a new sampler object using the default RenderDevice
CI_API SamplerRef createSampler( const Diligent::SamplerDesc &samDesc );
//! Creates a new resource mapping using the default RenderDevice
CI_API ResourceMappingRef createResourceMapping( const Diligent::ResourceMappingDesc &mappingDesc );
//! Creates a new graphics pipeline state object using the default RenderDevice
CI_API PipelineStateRef createGraphicsPipelineState( const Diligent::GraphicsPipelineStateCreateInfo &psoCreateInfo );
//! Creates a new compute pipeline state object using the default RenderDevice
CI_API PipelineStateRef createComputePipelineState( const Diligent::ComputePipelineStateCreateInfo &psoCreateInfo );
//! Creates a new ray tracing pipeline state object using the default RenderDevice
CI_API PipelineStateRef createRayTracingPipelineState( const Diligent::RayTracingPipelineStateCreateInfo &psoCreateInfo );
//! Creates a new fence object using the default RenderDevice
CI_API FenceRef createFence( const Diligent::FenceDesc &desc );
//! Creates a new query object using the default RenderDevice
CI_API QueryRef createQuery( const Diligent::QueryDesc &desc );
//! Creates a render pass object using the default RenderDevice
CI_API RenderPassRef createRenderPass( const Diligent::RenderPassDesc &desc );
//! Creates a framebuffer object using the default RenderDevice
CI_API FramebufferRef createFramebuffer( const Diligent::FramebufferDesc &desc );
//! Creates a bottom-level acceleration structure object (BLAS) using the default RenderDevice.
CI_API BottomLevelASRef createBLAS( const Diligent::BottomLevelASDesc &desc );
//! Creates a top-level acceleration structure object (TLAS) using the default RenderDevice.
CI_API TopLevelASRef createTLAS( const Diligent::TopLevelASDesc &desc );
//! Creates a shader resource binding table object (SBT) using the default RenderDevice.
CI_API ShaderBindingTableRef createSBT( const Diligent::ShaderBindingTableDesc &desc );

//! Gets the default device capabilities, see Diligent::DeviceCaps for details
CI_API const DeviceCaps& getDeviceCaps();
//! Gets the default device properties, see Diligent::DeviceProperties for details
CI_API const DeviceProperties& getDeviceProperties();
//! Returns the basic texture format information.
CI_API const TextureFormatInfo& getTextureFormatInfo( TEXTURE_FORMAT texFormat );
//! Returns the extended texture format information.
CI_API const TextureFormatInfoExt& getTextureFormatInfoExt( TEXTURE_FORMAT texFormat );

//! Purges default device release queues and releases all stale resources.
CI_API void releaseStaleResources( bool forceRelease = false );

//! Waits until all outstanding operations on the GPU are complete using the default RenderDevice.
CI_API void idleGPU();
//! Returns engine factory the default device was created from.
CI_API EngineFactory* getEngineFactory();
} 

namespace gx = graphics;
} // namespace cinder::graphics