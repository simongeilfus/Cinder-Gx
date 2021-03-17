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
    ShaderResourceVariableDesc( SHADER_TYPE shaderStages, const std::string &name, SHADER_RESOURCE_VARIABLE_TYPE type );
    
    ShaderResourceVariableDesc( const ShaderResourceVariableDesc &other );
    ShaderResourceVariableDesc( ShaderResourceVariableDesc &&other ) noexcept;
    ShaderResourceVariableDesc& operator=( const ShaderResourceVariableDesc &other );
    ShaderResourceVariableDesc& operator=( ShaderResourceVariableDesc &&other ) noexcept;
    ~ShaderResourceVariableDesc() = default;
protected:
    void updatePtrs() noexcept;
    void swap( ShaderResourceVariableDesc &other ) noexcept;

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
    //ImmutableSamplerDesc( const ImmutableSamplerDesc &other ) noexcept
    //    : mSamplerOrTextureName( other.mSamplerOrTextureName ), Diligent::ImmutableSamplerDesc( other.ShaderStages, nullptr, other.Desc ) { SamplerOrTextureName = mSamplerOrTextureName.data(); }
    //ImmutableSamplerDesc& operator=( const ImmutableSamplerDesc &other ) noexcept { mSamplerOrTextureName = other.mSamplerOrTextureName; ShaderStages = other.ShaderStages; SamplerOrTextureName = mSamplerOrTextureName.data(); Desc = other.Desc; return *this; }

    ImmutableSamplerDesc() = default;
    ImmutableSamplerDesc( const ImmutableSamplerDesc &other );
    ImmutableSamplerDesc( ImmutableSamplerDesc &&other ) noexcept;
    ImmutableSamplerDesc& operator=( const ImmutableSamplerDesc &other );
    ImmutableSamplerDesc& operator=( ImmutableSamplerDesc &&other ) noexcept;
    ~ImmutableSamplerDesc() = default;
protected:
    void updatePtrs() noexcept;
    void swap( ImmutableSamplerDesc &other ) noexcept;
    std::string mSamplerOrTextureName;
};

//! Graphics pipeline state creation attributes
struct CI_API GraphicsPipelineDesc {
    //! Blend state description.
    GraphicsPipelineDesc& blendStateDesc( const Diligent::BlendStateDesc &blendDesc ) { mGraphicsPipeline.BlendDesc = blendDesc; return *this; }
    //! Blend state description.
    GraphicsPipelineDesc& blendState( bool alphaToCoverageEnable, bool independentBlendEnable = false, const Diligent::RenderTargetBlendDesc& RT0 = {} ) { mGraphicsPipeline.BlendDesc = { alphaToCoverageEnable, independentBlendEnable, RT0 }; return *this; }
    //! 32-bit sample mask that determines which samples get updated in all the active render targets. A sample mask is always applied; it is independent of whether multisampling is enabled, and does not depend on whether an application uses multisample render targets.
    GraphicsPipelineDesc& sampleMask( uint32_t sampleMask ) { mGraphicsPipeline.SampleMask = sampleMask; return *this; }
    //! Rasterizer state description.
    GraphicsPipelineDesc& rasterizerStateDesc( const Diligent::RasterizerStateDesc &rasterizerDesc ) { mGraphicsPipeline.RasterizerDesc = rasterizerDesc; return *this; }
    //! Depth-stencil state description.
    GraphicsPipelineDesc& depthStencilDesc( const Diligent::DepthStencilStateDesc &depthStencilDesc ) { mGraphicsPipeline.DepthStencilDesc = depthStencilDesc; return *this; }
    //! Input layout, ignored in a mesh pipeline.
    GraphicsPipelineDesc& inputLayout( const Diligent::InputLayoutDesc &inputLayout ) { mGraphicsPipeline.InputLayout = inputLayout; return *this; }
    //! Input layout, ignored in a mesh pipeline.
    GraphicsPipelineDesc& inputLayout( const std::vector<LayoutElement> &elements );
    //! Primitive topology type, ignored in a mesh pipeline.
    GraphicsPipelineDesc& primitiveTopology( PRIMITIVE_TOPOLOGY primitiveTopology ) { mGraphicsPipeline.PrimitiveTopology = primitiveTopology; return *this; }
    //! The number of viewports used by this pipeline
    GraphicsPipelineDesc& numViewports( uint8_t numViewports ) { mGraphicsPipeline.NumViewports = numViewports; return *this; }
    //! The number of render targets in the RTVFormats array. Must be 0 when pRenderPass is not null.
    GraphicsPipelineDesc& numRenderTargets( uint8_t numRenderTargets ) { mGraphicsPipeline.NumRenderTargets = numRenderTargets; return *this; }
    //! When pRenderPass is not null, the subpass index within the render pass. When pRenderPass is null, this member must be 0.
    GraphicsPipelineDesc& subpassIndex( uint8_t subpassIndex ) { mGraphicsPipeline.SubpassIndex = subpassIndex; return *this; }
    //! Render target formats. All formats must be TEX_FORMAT_UNKNOWN when pRenderPass is not null.
    GraphicsPipelineDesc& rtvFormat( size_t index, TEXTURE_FORMAT rTVFormat ) { mGraphicsPipeline.RTVFormats[index] = rTVFormat; return *this; }
    //! Depth-stencil format. Must be TEX_FORMAT_UNKNOWN when pRenderPass is not null.
    GraphicsPipelineDesc& dsvFormat( TEXTURE_FORMAT dSVFormat ) { mGraphicsPipeline.DSVFormat = dSVFormat; return *this; }
    //! Multisampling parameters.
    GraphicsPipelineDesc& sampleDesc( const Diligent::SampleDesc &smplDesc ) { mGraphicsPipeline.SmplDesc = smplDesc; return *this; }
    //! Pointer to the render pass object. When non-null render pass is specified, NumRenderTargets must be 0, and all RTV formats as well as DSV format must be TEX_FORMAT_UNKNOWN.
    GraphicsPipelineDesc& renderPass( Diligent::IRenderPass* renderPass ){ mGraphicsPipeline.pRenderPass = renderPass; return *this; }
    //! Node mask.
    GraphicsPipelineDesc& nodeMask( uint32_t nodeMask ) { mGraphicsPipeline.NodeMask = nodeMask; return *this; }

    //! Vertex shader to be used with the pipeline.
    GraphicsPipelineDesc& vertexShader( const ShaderRef &vertexShader ) { mVS = vertexShader; return *this; }
    //! Vertex shader to be used with the pipeline.
    GraphicsPipelineDesc& vertexShader( const ShaderCreateInfo &shaderCreateInfo )  { mVSCreateInfo = shaderCreateInfo; mVSCreateInfo.Desc.ShaderType = SHADER_TYPE_VERTEX; return *this; }
    //! Pixel shader to be used with the pipeline.
    GraphicsPipelineDesc& pixelShader( const ShaderRef &pixelShader ) { mPS = pixelShader; return *this; }
    //! Pixel shader to be used with the pipeline.
    GraphicsPipelineDesc& pixelShader( const ShaderCreateInfo &shaderCreateInfo )  { mPSCreateInfo = shaderCreateInfo; mPSCreateInfo.Desc.ShaderType = SHADER_TYPE_PIXEL; return *this; }
    //! Domain shader to be used with the pipeline.
    GraphicsPipelineDesc& domainShader( const ShaderRef &domainShader ) { mDS = domainShader; return *this; }
    //! Domain shader to be used with the pipeline.
    GraphicsPipelineDesc& domainShader( const ShaderCreateInfo &shaderCreateInfo )  { mDSCreateInfo = shaderCreateInfo; mDSCreateInfo.Desc.ShaderType = SHADER_TYPE_DOMAIN; return *this; }
    //! Hull shader to be used with the pipeline.
    GraphicsPipelineDesc& hullShader( const ShaderRef &hullShader ) { mHS = hullShader; return *this; }
    //! Hull shader to be used with the pipeline.
    GraphicsPipelineDesc& hullShader( const ShaderCreateInfo &shaderCreateInfo )  { mHSCreateInfo = shaderCreateInfo; mHSCreateInfo.Desc.ShaderType = SHADER_TYPE_HULL; return *this; }
    //! Geometry shader to be used with the pipeline.
    GraphicsPipelineDesc& geometryShader( const ShaderRef &geometryShader ) { mGS = geometryShader; return *this; }
    //! Geometry shader to be used with the pipeline.
    GraphicsPipelineDesc& geometryShader( const ShaderCreateInfo &shaderCreateInfo )  { mGSCreateInfo = shaderCreateInfo; mGSCreateInfo.Desc.ShaderType = SHADER_TYPE_GEOMETRY; return *this; }
    //! Amplification shader to be used with the pipeline.
    GraphicsPipelineDesc& amplificationShader( const ShaderRef &amplificationShader ) { mAS = amplificationShader; return *this; }
    //! Amplification shader to be used with the pipeline.
    GraphicsPipelineDesc& amplificationShader( const ShaderCreateInfo &shaderCreateInfo )  { mASCreateInfo = shaderCreateInfo; mASCreateInfo.Desc.ShaderType = SHADER_TYPE_AMPLIFICATION; return *this; }
    //! Mesh shader to be used with the pipeline.
    GraphicsPipelineDesc& meshShader( const ShaderRef &meshShader ) { mMS = meshShader; return *this; }
    //! Mesh shader to be used with the pipeline.
    GraphicsPipelineDesc& meshShader( const ShaderCreateInfo &shaderCreateInfo )  { mMSCreateInfo = shaderCreateInfo; mMSCreateInfo.Desc.ShaderType = SHADER_TYPE_MESH; return *this; }

    //! Shader resource binding allocation granularity. This member defines allocation granularity for internal resources required by the shader resource binding object instances.
    GraphicsPipelineDesc& srbAllocationGranularity( uint32_t srbAllocationGranularity ) { mPSODesc.SRBAllocationGranularity = srbAllocationGranularity; return *this; }
    //! Defines which command queues this pipeline state can be used with
    GraphicsPipelineDesc& commandQueueMask( uint64_t commandQueueMask ) { mPSODesc.CommandQueueMask = commandQueueMask; return *this; }

    //! Pipeline layout description
    GraphicsPipelineDesc& resourceLayout( Diligent::PipelineResourceLayoutDesc resourceLayout ) { mPSODesc.ResourceLayout = resourceLayout; return *this; }
    //! Default shader resource variable type. This type will be used if shader variable description is not found in the Variables array or if Variables == nullptr
    GraphicsPipelineDesc& defaultVariableType( SHADER_RESOURCE_VARIABLE_TYPE defaultVariableType ) { mPSODesc.ResourceLayout.DefaultVariableType = defaultVariableType; return *this; }
    //! Array of shader resource variable descriptions               
    GraphicsPipelineDesc& variables( const std::vector<ShaderResourceVariableDesc> &variables );
    //! Array of immutable sampler descriptions                
    GraphicsPipelineDesc& immutableSamplers( const std::vector<ImmutableSamplerDesc> &immutableSamplers );

    //! Specifies the object's name.
    GraphicsPipelineDesc& name( const std::string &name ) { mName = name; mPSODesc.Name = mName.c_str(); return *this; }

    GraphicsPipelineDesc();
    GraphicsPipelineDesc( const GraphicsPipelineDesc &other );
    GraphicsPipelineDesc( GraphicsPipelineDesc &&other ) noexcept;
    GraphicsPipelineDesc& operator=( const GraphicsPipelineDesc &other );
    GraphicsPipelineDesc& operator=( GraphicsPipelineDesc &&other ) noexcept;
    ~GraphicsPipelineDesc() = default;

    const std::vector<LayoutElement>& getLayoutElements() const { return mLayoutElements; }
    const std::vector<ShaderResourceVariableDesc>& getVariables() const { return mVariables; }
    const std::vector<ImmutableSamplerDesc>& getImmutableSamplers() const { return mImmutableSamplers; }
    const std::vector<Diligent::ShaderResourceVariableDesc>& getVariablesBase() const { return mVariablesBase; }
    const std::vector<Diligent::ImmutableSamplerDesc>& getImmutableSamplersBase() const { return mImmutableSamplersBase; }

protected:
    void updatePtrs() noexcept;
    void swap( GraphicsPipelineDesc &other ) noexcept;

    std::string mName;
    ShaderRef mVS, mPS, mDS, mHS, mGS, mAS, mMS;
    ShaderCreateInfo mVSCreateInfo, mPSCreateInfo, mDSCreateInfo, mHSCreateInfo, mGSCreateInfo, mASCreateInfo, mMSCreateInfo;
    std::vector<LayoutElement> mLayoutElements;
    std::vector<ShaderResourceVariableDesc> mVariables;
    std::vector<ImmutableSamplerDesc> mImmutableSamplers;

    Diligent::GraphicsPipelineDesc  mGraphicsPipeline;
    Diligent::PipelineStateDesc     mPSODesc;
    PSO_CREATE_FLAGS                mFlags;

    std::vector<Diligent::ShaderResourceVariableDesc> mVariablesBase;
    std::vector<Diligent::ImmutableSamplerDesc> mImmutableSamplersBase;

    friend PipelineStateRef createGraphicsPipelineState( RenderDevice* device, const gx::GraphicsPipelineDesc &pipelineDesc );
};

PipelineStateRef createGraphicsPipelineState( const gx::GraphicsPipelineDesc &pipelineDesc );
PipelineStateRef createGraphicsPipelineState( RenderDevice* device, const gx::GraphicsPipelineDesc &pipelineDesc );

}

namespace gx = graphics;
} // namespace cinder::graphics