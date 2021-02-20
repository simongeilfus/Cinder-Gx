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

#include "cinder/Export.h"
#include "cinder/Vector.h"

#include "DiligentCore/Graphics/GraphicsEngine/interface/GraphicsTypes.h"

namespace cinder { namespace graphics {

using namespace Diligent;

//! Swap chain description
struct CI_API SwapChainDesc : public Diligent::SwapChainDesc {
	//! The swap chain width. Default value is 0
	SwapChainDesc& width( uint32_t width ) { Width = width; return *this; }
	//! The swap chain height. Default value is 0
	SwapChainDesc& height( uint32_t height ) { Height = height; return *this; }
	//! The swap chain size. Default value is [0,0]
	SwapChainDesc& size( const ci::ivec2 &size ) { Width = size.x; Height = size.y; return *this; }
	//! Back buffer format. Default value is Diligent::TEX_FORMAT_RGBA8_UNORM_SRGB
	SwapChainDesc& colorBufferFormat( TEXTURE_FORMAT colorBufferFormat ) { ColorBufferFormat = colorBufferFormat; return *this; }
	//! Depth buffer format. Default value is Diligent::TEX_FORMAT_D32_FLOAT. Use Diligent::TEX_FORMAT_UNKNOWN to create the swap chain without depth buffer.
	SwapChainDesc& depthBufferFormat( TEXTURE_FORMAT depthBufferFormat ) { DepthBufferFormat = depthBufferFormat; return *this; }
	//! Swap chain usage flags. Default value is Diligent::SWAP_CHAIN_USAGE_RENDER_TARGET
	SwapChainDesc& usage( SWAP_CHAIN_USAGE_FLAGS usage ) { Usage = usage; return *this; }
	//! The transform, relative to the presentation engine's natural orientation, applied to the image content prior to presentation.
	SwapChainDesc& preTransform( SURFACE_TRANSFORM preTransform ) { PreTransform = preTransform; return *this; }
	//! The number of buffers in the swap chain
	SwapChainDesc& bufferCount( uint32_t bufferCount ) { BufferCount = bufferCount; return *this; }
	//! Default depth value, which is used as optimized depth clear value in D3D12
	SwapChainDesc& defaultDepthValue( float defaultDepthValue ) { DefaultDepthValue = defaultDepthValue; return *this; }
	//! Default stencil value, which is used as optimized stencil clear value in D3D12
	SwapChainDesc& defaultStencilValue( uint8_t defaultStencilValue ) { DefaultStencilValue = defaultStencilValue; return *this; }
	//! Indicates if this is a primary swap chain. When Present() is called for the primary swap chain, the engine releases stale resources.
	SwapChainDesc& isPrimary( bool isPrimary ) { IsPrimary = isPrimary; return *this; }
};

//! Describes the device features
struct DeviceFeatures : Diligent::DeviceFeatures {
    //! Indicates if device supports separable shader programs.
    DeviceFeatures& separablePrograms( DEVICE_FEATURE_STATE state ) { SeparablePrograms = state; return *this; }
    //! Indicates if device supports resource queries from shader objects.
    DeviceFeatures& shaderResourceQueries( DEVICE_FEATURE_STATE state ) { ShaderResourceQueries = state; return *this; }
    //! Indicates if device supports indirect draw commands
    DeviceFeatures& indirectRendering( DEVICE_FEATURE_STATE state ) { IndirectRendering = state; return *this; }
    //! Indicates if device supports wireframe fill mode
    DeviceFeatures& wireframeFill( DEVICE_FEATURE_STATE state ) { WireframeFill = state; return *this; }
    //! Indicates if device supports multithreaded resource creation
    DeviceFeatures& multithreadedResourceCreation( DEVICE_FEATURE_STATE state ) { MultithreadedResourceCreation = state; return *this; }
    //! Indicates if device supports compute shaders
    DeviceFeatures& computeShaders( DEVICE_FEATURE_STATE state ) { ComputeShaders = state; return *this; }
    //! Indicates if device supports geometry shaders
    DeviceFeatures& geometryShaders( DEVICE_FEATURE_STATE state ) { GeometryShaders = state; return *this; }
    //! Indicates if device supports tessellation
    DeviceFeatures& tessellation( DEVICE_FEATURE_STATE state ) { Tessellation = state; return *this; }
    //! Indicates if device supports mesh and amplification shaders
    DeviceFeatures& meshShaders( DEVICE_FEATURE_STATE state ) { MeshShaders = state; return *this; }
    //! Indicates if device supports ray tracing shaders
    DeviceFeatures& rayTracing( DEVICE_FEATURE_STATE state ) { RayTracing = state; return *this; }
    //! Indicates if device supports bindless resources
    DeviceFeatures& bindlessResources( DEVICE_FEATURE_STATE state ) { BindlessResources = state; return *this; }
    //! Indicates if device supports occlusion queries (see Diligent::QUERY_TYPE_OCCLUSION).
    DeviceFeatures& occlusionQueries( DEVICE_FEATURE_STATE state ) { OcclusionQueries = state; return *this; }
    //! Indicates if device supports binary occlusion queries (see Diligent::QUERY_TYPE_BINARY_OCCLUSION).
    DeviceFeatures& binaryOcclusionQueries( DEVICE_FEATURE_STATE state ) { BinaryOcclusionQueries = state; return *this; }
    //! Indicates if device supports timestamp queries (see Diligent::QUERY_TYPE_TIMESTAMP).
    DeviceFeatures& timestampQueries( DEVICE_FEATURE_STATE state ) { TimestampQueries = state; return *this; }
    //! Indicates if device supports pipeline statistics queries (see Diligent::QUERY_TYPE_PIPELINE_STATISTICS).
    DeviceFeatures& pipelineStatisticsQueries( DEVICE_FEATURE_STATE state ) { PipelineStatisticsQueries = state; return *this; }
    //! Indicates if device supports duration queries (see Diligent::QUERY_TYPE_DURATION).
    DeviceFeatures& durationQueries( DEVICE_FEATURE_STATE state ) { DurationQueries = state; return *this; }
    //! Indicates if device supports depth bias clamping
    DeviceFeatures& depthBiasClamp( DEVICE_FEATURE_STATE state ) { DepthBiasClamp = state; return *this; }
    //! Indicates if device supports depth clamping
    DeviceFeatures& depthClamp( DEVICE_FEATURE_STATE state ) { DepthClamp = state; return *this; }
    //! Indicates if device supports depth clamping
    DeviceFeatures& independentBlend( DEVICE_FEATURE_STATE state ) { IndependentBlend = state; return *this; }
    //! Indicates if device supports dual-source blend
    DeviceFeatures& dualSourceBlend( DEVICE_FEATURE_STATE state ) { DualSourceBlend = state; return *this; }
    //! Indicates if device supports multiviewport
    DeviceFeatures& multiViewport( DEVICE_FEATURE_STATE state ) { MultiViewport = state; return *this; }
    //! Indicates if device supports all BC-compressed formats
    DeviceFeatures& textureCompressionBC( DEVICE_FEATURE_STATE state ) { TextureCompressionBC = state; return *this; }
    //! Indicates if device supports writes to UAVs as well as atomic operations in vertex, tessellation, and geometry shader stages.
    DeviceFeatures& vertexPipelineUAVWritesAndAtomics( DEVICE_FEATURE_STATE state ) { VertexPipelineUAVWritesAndAtomics = state; return *this; }
    //! Indicates if device supports writes to UAVs as well as atomic operations in pixel shader stage.
    DeviceFeatures& pixelUAVWritesAndAtomics( DEVICE_FEATURE_STATE state ) { PixelUAVWritesAndAtomics = state; return *this; }
    //! Specifies whether all the extended UAV texture formats are available in shader code.
    DeviceFeatures& textureUAVExtendedFormats( DEVICE_FEATURE_STATE state ) { TextureUAVExtendedFormats = state; return *this; }
    //! Indicates if device supports native 16-bit float operations. Note that there are separate features that indicate if device supports loading 16-bit floats from buffers and passing them between shader stages. 
    DeviceFeatures& shaderFloat16( DEVICE_FEATURE_STATE state ) { ShaderFloat16 = state; return *this; }
    //! Indicates if device supports reading and writing 16-bit floats and ints from buffers bound as shader resource or unordered access views.
    DeviceFeatures& resourceBuffer16BitAccess( DEVICE_FEATURE_STATE state ) { ResourceBuffer16BitAccess = state; return *this; }
    //! Indicates if device supports reading 16-bit floats and ints from uniform buffers.
    DeviceFeatures& uniformBuffer16BitAccess( DEVICE_FEATURE_STATE state ) { UniformBuffer16BitAccess = state; return *this; }
    //! Indicates if 16-bit floats and ints can be used as input/output of a shader entry point.
    DeviceFeatures& shaderInputOutput16( DEVICE_FEATURE_STATE state ) { ShaderInputOutput16 = state; return *this; }
    //! Indicates if device supports native 8-bit integer operations.
    DeviceFeatures& shaderInt8( DEVICE_FEATURE_STATE state ) { ShaderInt8 = state; return *this; }
    //! Indicates if device supports reading and writing 8-bit types from buffers bound as shader resource or unordered access views.
    DeviceFeatures& resourceBuffer8BitAccess( DEVICE_FEATURE_STATE state ) { ResourceBuffer8BitAccess = state; return *this; }
    //! Indicates if device supports reading 8-bit types from uniform buffers.
    DeviceFeatures& uniformBuffer8BitAccess( DEVICE_FEATURE_STATE state ) { UniformBuffer8BitAccess = state; return *this; }
};

}

namespace gx = graphics;
} // namespace cinder::graphics