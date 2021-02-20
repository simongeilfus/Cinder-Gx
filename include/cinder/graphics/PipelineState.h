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

#include "cinder/graphics/wrapper.h"
#include "cinder/graphics/BlendState.h"
#include "cinder/graphics/DepthStencilState.h"
#include "cinder/graphics/RasterizerState.h"
#include "cinder/graphics/Shader.h"

namespace cinder { namespace graphics {

//! Sample description. This structure is used by GraphicsPipelineDesc to describe multisampling parameters
struct SampleDesc : public Diligent::SampleDesc {
    //! Sample count
    SampleDesc& count( uint8_t count ) { Count = count; return *this; }
    //! Quality
    SampleDesc& quality( uint8_t quality ) { Quality = quality; return *this; }
};

struct CI_API PipelineResourceLayoutDesc : public Diligent::PipelineResourceLayoutDesc {
    //! Default shader resource variable type. This type will be used if shader variable description is not found in the Variables array or if Variables == nullptr
    PipelineResourceLayoutDesc& defaultVariableType( SHADER_RESOURCE_VARIABLE_TYPE defaultVariableType ) { DefaultVariableType = defaultVariableType; return *this; }
    //! Array of shader resource variable descriptions               
    PipelineResourceLayoutDesc& variables( const Diligent::ShaderResourceVariableDesc* variables, uint32_t numVariables ) { Variables = variables; NumVariables = numVariables; return *this; }
    //! Array of immutable sampler descriptions                
    PipelineResourceLayoutDesc& immutableSamplers( const Diligent::ImmutableSamplerDesc* immutableSamplers, uint32_t numImmutableSamplers ) { ImmutableSamplers = immutableSamplers; NumImmutableSamplers = numImmutableSamplers; return *this; }
};

//! Describes shader variable
struct ShaderResourceVariableDesc : public Diligent::ShaderResourceVariableDesc {
    //! Shader stages this resources variable applies to. More than one shader stage can be specified.
    ShaderResourceVariableDesc& shaderStages( SHADER_TYPE shaderStages ) { ShaderStages = shaderStages; return *this; }
    //! Shader variable name
    ShaderResourceVariableDesc& name( const std::string &name ) { mName = name; Name = mName.data(); return *this; }
    //! Shader variable type. See Diligent::SHADER_RESOURCE_VARIABLE_TYPE for a list of allowed types
    ShaderResourceVariableDesc& type( SHADER_RESOURCE_VARIABLE_TYPE type ) { Type = type; return *this; }

    ShaderResourceVariableDesc() = default;
    ShaderResourceVariableDesc( SHADER_TYPE shaderStages, const std::string &name, SHADER_RESOURCE_VARIABLE_TYPE type ) noexcept
        : mName( name ), Diligent::ShaderResourceVariableDesc( shaderStages, mName.data(), type ) {}
    ShaderResourceVariableDesc( const ShaderResourceVariableDesc &other ) noexcept
        : mName( other.mName ), Diligent::ShaderResourceVariableDesc( other.ShaderStages, nullptr, other.Type ) { Name = mName.data(); }
    ShaderResourceVariableDesc& operator=( const ShaderResourceVariableDesc &other ) { mName = other.mName; ShaderStages = other.ShaderStages; Name = mName.data(); Type = other.Type; return *this; }
protected:
    std::string mName;
};

//! Immutable sampler description. An immutable sampler is compiled into the pipeline state and can't be changed. It is generally more efficient than a regular sampler and should be used whenever possible.
struct ImmutableSamplerDesc : public Diligent::ImmutableSamplerDesc {
    //! Shader stages that this immutable sampler applies to. More than one shader stage can be specified.
    ImmutableSamplerDesc& shaderStages( SHADER_TYPE shaderStages ) { ShaderStages = shaderStages; return *this; }
    //! The name of the sampler itself or the name of the texture variable that this immutable sampler is assigned to if combined texture samplers are used.
    ImmutableSamplerDesc& samplerOrTextureName( const std::string &samplerOrTextureName ) { mSamplerOrTextureName = samplerOrTextureName; SamplerOrTextureName = mSamplerOrTextureName.data(); return *this; }
    //! Sampler description
    ImmutableSamplerDesc& samplerDesc( const Diligent::SamplerDesc desc ) { Desc = desc; return *this; }

    ImmutableSamplerDesc( SHADER_TYPE shaderStages, const std::string &samplerOrTextureName, const SamplerDesc& desc ) noexcept
        : mSamplerOrTextureName( samplerOrTextureName ), Diligent::ImmutableSamplerDesc( shaderStages, mSamplerOrTextureName.data(), desc ) {}
    ImmutableSamplerDesc( const ImmutableSamplerDesc &other ) noexcept
        : mSamplerOrTextureName( other.mSamplerOrTextureName ), Diligent::ImmutableSamplerDesc( other.ShaderStages, nullptr, other.Desc ) { SamplerOrTextureName = mSamplerOrTextureName.data(); }
    ImmutableSamplerDesc& operator=( const ImmutableSamplerDesc &other ) noexcept { mSamplerOrTextureName = other.mSamplerOrTextureName; ShaderStages = other.ShaderStages; SamplerOrTextureName = mSamplerOrTextureName.data(); Desc = other.Desc; return *this; }
protected:
    std::string mSamplerOrTextureName;
};

//! Graphics pipeline state creation attributes
struct CI_API GraphicsPipelineStateCreateInfo : public Diligent::GraphicsPipelineStateCreateInfo {
    //! Blend state description.
    GraphicsPipelineStateCreateInfo& blendStateDesc( const Diligent::BlendStateDesc &blendDesc ) { GraphicsPipeline.BlendDesc = blendDesc; return *this; }
    //! Blend state description.
    GraphicsPipelineStateCreateInfo& blendState( bool alphaToCoverageEnable, bool independentBlendEnable = false, const Diligent::RenderTargetBlendDesc& RT0 = {} ) { GraphicsPipeline.BlendDesc = { alphaToCoverageEnable, independentBlendEnable, RT0 }; return *this; }
    //! 32-bit sample mask that determines which samples get updated in all the active render targets. A sample mask is always applied; it is independent of whether multisampling is enabled, and does not depend on whether an application uses multisample render targets.
    GraphicsPipelineStateCreateInfo& sampleMask( uint32_t sampleMask ) { GraphicsPipeline.SampleMask = sampleMask; return *this; }
    //! Rasterizer state description.
    GraphicsPipelineStateCreateInfo& rasterizerStateDesc( const Diligent::RasterizerStateDesc &rasterizerDesc ) { GraphicsPipeline.RasterizerDesc = rasterizerDesc; return *this; }
    //! Depth-stencil state description.
    GraphicsPipelineStateCreateInfo& depthStencilDesc( const Diligent::DepthStencilStateDesc &depthStencilDesc ) { GraphicsPipeline.DepthStencilDesc = depthStencilDesc; return *this; }
    //! Input layout, ignored in a mesh pipeline.
    GraphicsPipelineStateCreateInfo& inputLayout( const Diligent::InputLayoutDesc &inputLayout ) { GraphicsPipeline.InputLayout = inputLayout; return *this; }
    //! Input layout, ignored in a mesh pipeline.
    GraphicsPipelineStateCreateInfo& inputLayout( const std::vector<LayoutElement> &elements ) { mLayoutElements = elements; GraphicsPipeline.InputLayout.LayoutElements = mLayoutElements.data(); GraphicsPipeline.InputLayout.NumElements = static_cast<Diligent::Uint32>( mLayoutElements.size() ); return *this; }
    //! Primitive topology type, ignored in a mesh pipeline.
    GraphicsPipelineStateCreateInfo& primitiveTopology( PRIMITIVE_TOPOLOGY primitiveTopology ) { GraphicsPipeline.PrimitiveTopology = primitiveTopology; return *this; }
    //! The number of viewports used by this pipeline
    GraphicsPipelineStateCreateInfo& numViewports( uint8_t numViewports ) { GraphicsPipeline.NumViewports = numViewports; return *this; }
    //! The number of render targets in the RTVFormats array. Must be 0 when pRenderPass is not null.
    GraphicsPipelineStateCreateInfo& numRenderTargets( uint8_t numRenderTargets ) { GraphicsPipeline.NumRenderTargets = numRenderTargets; return *this; }
    //! When pRenderPass is not null, the subpass index within the render pass. When pRenderPass is null, this member must be 0.
    GraphicsPipelineStateCreateInfo& subpassIndex( uint8_t subpassIndex ) { GraphicsPipeline.SubpassIndex = subpassIndex; return *this; }
    //! Render target formats. All formats must be TEX_FORMAT_UNKNOWN when pRenderPass is not null.
    GraphicsPipelineStateCreateInfo& rtvFormat( size_t index, TEXTURE_FORMAT rTVFormat ) { GraphicsPipeline.RTVFormats[index] = rTVFormat; return *this; }
    //! Depth-stencil format. Must be TEX_FORMAT_UNKNOWN when pRenderPass is not null.
    GraphicsPipelineStateCreateInfo& dsvFormat( TEXTURE_FORMAT dSVFormat ) { GraphicsPipeline.DSVFormat = dSVFormat; return *this; }
    //! Multisampling parameters.
    GraphicsPipelineStateCreateInfo& sampleDesc( const Diligent::SampleDesc &smplDesc ) { GraphicsPipeline.SmplDesc = smplDesc; return *this; }
    //! Pointer to the render pass object. When non-null render pass is specified, NumRenderTargets must be 0, and all RTV formats as well as DSV format must be TEX_FORMAT_UNKNOWN.
    GraphicsPipelineStateCreateInfo& renderPass( Diligent::IRenderPass* renderPass ){ GraphicsPipeline.pRenderPass = renderPass; return *this; }
    //! Node mask.
    GraphicsPipelineStateCreateInfo& nodeMask( uint32_t nodeMask ) { GraphicsPipeline.NodeMask = nodeMask; return *this; }

    //! Vertex shader to be used with the pipeline.
    GraphicsPipelineStateCreateInfo& vertexShader( Shader* vertexShader ) { pVS = vertexShader; return *this; }
    //! Vertex shader to be used with the pipeline.
    GraphicsPipelineStateCreateInfo& vertexShader( ShaderRef &vertexShader ) { pVS = mVS = vertexShader; return *this; }
    //! Pixel shader to be used with the pipeline.
    GraphicsPipelineStateCreateInfo& pixelShader( Shader* pixelShader ) { pPS = pixelShader; return *this; }
    //! Pixel shader to be used with the pipeline.
    GraphicsPipelineStateCreateInfo& pixelShader( ShaderRef &pixelShader ) { pPS = mPS = pixelShader; return *this; }
    //! Domain shader to be used with the pipeline.
    GraphicsPipelineStateCreateInfo& domainShader( Shader* domainShader ) { pDS = domainShader; return *this; }
    //! Domain shader to be used with the pipeline.
    GraphicsPipelineStateCreateInfo& domainShader( ShaderRef &domainShader ) { pDS = mDS = domainShader; return *this; }
    //! Hull shader to be used with the pipeline.
    GraphicsPipelineStateCreateInfo& hullShader( Shader* hullShader ) { pHS = hullShader; return *this; }
    //! Hull shader to be used with the pipeline.
    GraphicsPipelineStateCreateInfo& hullShader( ShaderRef &hullShader ) { pHS = mHS = hullShader; return *this; }
    //! Geometry shader to be used with the pipeline.
    GraphicsPipelineStateCreateInfo& geometryShader( Shader* geometryShader ) { pGS = geometryShader; return *this; }
    //! Geometry shader to be used with the pipeline.
    GraphicsPipelineStateCreateInfo& geometryShader( ShaderRef &geometryShader ) { pGS = mGS = geometryShader; return *this; }
    //! Amplification shader to be used with the pipeline.
    GraphicsPipelineStateCreateInfo& amplificationShader( Shader* amplificationShader ) { pAS = amplificationShader; return *this; }
    //! Amplification shader to be used with the pipeline.
    GraphicsPipelineStateCreateInfo& amplificationShader( ShaderRef &amplificationShader ) { pAS = mAS = amplificationShader; return *this; }
    //! Mesh shader to be used with the pipeline.
    GraphicsPipelineStateCreateInfo& meshShader( Shader* meshShader ) { pMS = meshShader; return *this; }
    //! Mesh shader to be used with the pipeline.
    GraphicsPipelineStateCreateInfo& meshShader( ShaderRef &meshShader ) { pMS = mMS = meshShader; return *this; }

    //! Shader resource binding allocation granularity. This member defines allocation granularity for internal resources required by the shader resource binding object instances.
    GraphicsPipelineStateCreateInfo& srbAllocationGranularity( uint32_t srbAllocationGranularity ) { PSODesc.SRBAllocationGranularity = srbAllocationGranularity; return *this; }
    //! Defines which command queues this pipeline state can be used with
    GraphicsPipelineStateCreateInfo& commandQueueMask( uint64_t commandQueueMask ) { PSODesc.CommandQueueMask = commandQueueMask; return *this; }

    //! Pipeline layout description
    GraphicsPipelineStateCreateInfo& resourceLayout( Diligent::PipelineResourceLayoutDesc resourceLayout ) { PSODesc.ResourceLayout = resourceLayout; return *this; }
    //! Default shader resource variable type. This type will be used if shader variable description is not found in the Variables array or if Variables == nullptr
    GraphicsPipelineStateCreateInfo& defaultVariableType( SHADER_RESOURCE_VARIABLE_TYPE defaultVariableType ) { PSODesc.ResourceLayout.DefaultVariableType = defaultVariableType; return *this; }
    //! Array of shader resource variable descriptions               
    GraphicsPipelineStateCreateInfo& variables( const std::vector<ShaderResourceVariableDesc> &variables ) { mVariables = variables; PSODesc.ResourceLayout.Variables = mVariables.data(); PSODesc.ResourceLayout.NumVariables = static_cast<Diligent::Uint32>( mVariables.size() ); return *this; }
    //! Array of immutable sampler descriptions                
    GraphicsPipelineStateCreateInfo& immutableSamplers( const std::vector<ImmutableSamplerDesc> &immutableSamplers ) { mImmutableSamplers = immutableSamplers; PSODesc.ResourceLayout.ImmutableSamplers = mImmutableSamplers.data(); PSODesc.ResourceLayout.NumImmutableSamplers = static_cast<Diligent::Uint32>( mImmutableSamplers.size() ); return *this; }

    //! Speficies the object's name.
    GraphicsPipelineStateCreateInfo& name( const char* name ) { PSODesc.Name = name; return *this; }
    GraphicsPipelineStateCreateInfo() noexcept;
    GraphicsPipelineStateCreateInfo( const GraphicsPipelineStateCreateInfo &other ) noexcept;
    GraphicsPipelineStateCreateInfo& operator=( GraphicsPipelineStateCreateInfo other ) noexcept;
protected:
    void copy( const GraphicsPipelineStateCreateInfo &other );
    ShaderRef mVS, mPS, mDS, mHS, mGS, mAS, mMS;
    std::vector<LayoutElement> mLayoutElements;
    std::vector<ShaderResourceVariableDesc> mVariables;
    std::vector<ImmutableSamplerDesc> mImmutableSamplers;
};

}

namespace gx = graphics;
} // namespace cinder::graphics