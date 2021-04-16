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
struct CI_API GraphicsPipelineCreateInfo {
    //! Blend state description.
    GraphicsPipelineCreateInfo& blendStateDesc( const Diligent::BlendStateDesc &blendDesc ) { mGraphicsPipeline.BlendDesc = blendDesc; return *this; }
    //! Specifies the blend states per render targets
    GraphicsPipelineCreateInfo& renderTargetBlendDesc( size_t rtIndex, const Diligent::RenderTargetBlendDesc &desc ) { mGraphicsPipeline.BlendDesc.RenderTargets[rtIndex] = desc; return *this; }
    //! Specifies whether to use alpha-to-coverage as a multisampling technique when setting a pixel to a render target. Default value: False.
    GraphicsPipelineCreateInfo& alphaToCoverageEnable( bool alphaToCoverageEnable ) { mGraphicsPipeline.BlendDesc.AlphaToCoverageEnable = alphaToCoverageEnable; return *this; }
    //! Specifies whether to enable independent blending in simultaneous render targets. If set to False, only RenderTargets[0] is used. Default value: False.
    GraphicsPipelineCreateInfo& independentBlendEnable( bool independentBlendEnable ) { mGraphicsPipeline.BlendDesc.IndependentBlendEnable = independentBlendEnable; return *this; }
    //! Enables blending and sets the blend function to unpremultiplied alpha blending
    GraphicsPipelineCreateInfo& alphaBlending();
    //! Enables blending and sets the blend function to unpremultiplied alpha blending
    GraphicsPipelineCreateInfo& alphaBlending( size_t renderTargetIndex );
    //! Enables blending and sets the blend function to premultiplied alpha blending
    GraphicsPipelineCreateInfo& alphaBlendingPremult();
    //! Enables blending and sets the blend function to premultiplied alpha blending
    GraphicsPipelineCreateInfo& alphaBlendingPremult( size_t renderTargetIndex );
    //! Enables \c GL_BLEND and sets the blend function to additive blending
    GraphicsPipelineCreateInfo& additiveBlending();
    //! Enables \c GL_BLEND and sets the blend function to additive blending
    GraphicsPipelineCreateInfo& additiveBlending( size_t renderTargetIndex );

    //! 32-bit sample mask that determines which samples get updated in all the active render targets. A sample mask is always applied; it is independent of whether multisampling is enabled, and does not depend on whether an application uses multisample render targets.
    GraphicsPipelineCreateInfo& sampleMask( uint32_t sampleMask ) { mGraphicsPipeline.SampleMask = sampleMask; return *this; }

    //! Rasterizer state description.
    GraphicsPipelineCreateInfo& rasterizerStateDesc( const Diligent::RasterizerStateDesc &rasterizerDesc ) { mGraphicsPipeline.RasterizerDesc = rasterizerDesc; return *this; }
    //! Determines traingle fill mode, see Diligent::FILL_MODE for details. Default value: Diligent::FILL_MODE_SOLID.
    GraphicsPipelineCreateInfo& fillMode( FILL_MODE fillMode ) { mGraphicsPipeline.RasterizerDesc.FillMode = fillMode; return *this; }
    //! Determines traingle cull mode, see Diligent::CULL_MODE for details. Default value: Diligent::CULL_MODE_BACK.
    GraphicsPipelineCreateInfo& cullMode( CULL_MODE cullMode ) { mGraphicsPipeline.RasterizerDesc.CullMode = cullMode; return *this; }
    //! Determines if a triangle is front- or back-facing. If this parameter is True, a triangle will be considered front-facing if its vertices are counter-clockwise on the render target. Default value: False.
    GraphicsPipelineCreateInfo& frontCounterClockwise( bool frontCounterClockwise ) { mGraphicsPipeline.RasterizerDesc.FrontCounterClockwise = frontCounterClockwise; return *this; }
    //! Enable clipping against near and far clip planes. Default value: True.
    GraphicsPipelineCreateInfo& depthClipEnable( bool depthClipEnable ) { mGraphicsPipeline.RasterizerDesc.DepthClipEnable = depthClipEnable; return *this; }
    //! Enable scissor-rectangle culling. All pixels outside an active scissor rectangle are culled. Default value: False.
    GraphicsPipelineCreateInfo& scissorEnable( bool scissorEnable ) { mGraphicsPipeline.RasterizerDesc.ScissorEnable = scissorEnable; return *this; }
    //! Specifies whether to enable line antialiasing. Default value: False.
    GraphicsPipelineCreateInfo& antialiasedLineEnable( bool antialiasedLineEnable ) { mGraphicsPipeline.RasterizerDesc.AntialiasedLineEnable = antialiasedLineEnable; return *this; }
    //! Constant value added to the depth of a given pixel. Default value: 0.
    GraphicsPipelineCreateInfo& depthBias( int32_t depthBias ) { mGraphicsPipeline.RasterizerDesc.DepthBias = depthBias; return *this; }
    //! Maximum depth bias of a pixel. \warning Depth bias clamp is not available in OpenGL Default value: 0.
    GraphicsPipelineCreateInfo& depthBiasClamp( float depthBiasClamp ) { mGraphicsPipeline.RasterizerDesc.DepthBiasClamp = depthBiasClamp; return *this; }
    //! Scalar that scales the given pixel's slope before adding to the pixel's depth. Default value: 0.
    GraphicsPipelineCreateInfo& slopeScaledDepthBias( float slopeScaledDepthBias ) { mGraphicsPipeline.RasterizerDesc.SlopeScaledDepthBias = slopeScaledDepthBias; return *this; }

    //! Depth-stencil state description.
    GraphicsPipelineCreateInfo& depthStencilDesc( const Diligent::DepthStencilStateDesc &depthStencilDesc ) { mGraphicsPipeline.DepthStencilDesc = depthStencilDesc; return *this; }
    //! Enable depth-stencil operations. When it is set to False, depth test always passes, depth writes are disabled, and no stencil operations are performed. Default value: True.
    GraphicsPipelineCreateInfo& depthEnable( bool depthEnable ) { mGraphicsPipeline.DepthStencilDesc.DepthEnable = depthEnable; return *this; };
    //! Enable or disable writes to a depth buffer. Default value: True.
    GraphicsPipelineCreateInfo& depthWriteEnable( bool depthWriteEnable ) { mGraphicsPipeline.DepthStencilDesc.DepthWriteEnable = depthWriteEnable; return *this; };
    //! A function that compares depth data against existing depth data. See Diligent::COMPARISON_FUNCTION for details. Default value: Diligent::COMPARISON_FUNC_LESS.
    GraphicsPipelineCreateInfo& depthFunc( COMPARISON_FUNCTION depthFunc ) { mGraphicsPipeline.DepthStencilDesc.DepthFunc = depthFunc; return *this; };
    //! Enable stencil opertaions. Default value: False.
    GraphicsPipelineCreateInfo& stencilEnable( bool stencilEnable ) { mGraphicsPipeline.DepthStencilDesc.StencilEnable = stencilEnable; return *this; };
    //! Identify which bits of the depth-stencil buffer are accessed when reading stencil data. Default value: 0xFF.
    GraphicsPipelineCreateInfo& stencilReadMask( uint8_t stencilReadMask ) { mGraphicsPipeline.DepthStencilDesc.StencilReadMask = stencilReadMask; return *this; }
    //! Identify which bits of the depth-stencil buffer are accessed when writing stencil data. Default value: 0xFF.
    GraphicsPipelineCreateInfo& stencilWriteMask( uint8_t stencilWriteMask ) { mGraphicsPipeline.DepthStencilDesc.StencilWriteMask = stencilWriteMask; return *this; };
    //! Identify stencil operations for the front-facing triangles, see Diligent::StencilOpDesc.
    GraphicsPipelineCreateInfo& frontFace( StencilOpDesc frontFace ) { mGraphicsPipeline.DepthStencilDesc.FrontFace = frontFace; return *this; };
    //! Identify stencil operations for the back-facing triangles, see Diligent::StencilOpDesc.
    GraphicsPipelineCreateInfo& backFace( StencilOpDesc backFace ) { mGraphicsPipeline.DepthStencilDesc.BackFace = backFace; return *this; };

    //! Input layout, ignored in a mesh pipeline.
    GraphicsPipelineCreateInfo& inputLayout( const Diligent::InputLayoutDesc &inputLayout ) { mGraphicsPipeline.InputLayout = inputLayout; return *this; }
    //! Input layout, ignored in a mesh pipeline.
    GraphicsPipelineCreateInfo& inputLayout( const std::vector<LayoutElement> &elements );
    //! Primitive topology type, ignored in a mesh pipeline.
    GraphicsPipelineCreateInfo& primitiveTopology( PRIMITIVE_TOPOLOGY primitiveTopology ) { mGraphicsPipeline.PrimitiveTopology = primitiveTopology; return *this; }

    //! The number of viewports used by this pipeline
    GraphicsPipelineCreateInfo& numViewports( uint8_t numViewports ) { mGraphicsPipeline.NumViewports = numViewports; return *this; }
    //! The number of render targets in the RTVFormats array. Must be 0 when pRenderPass is not null.
    GraphicsPipelineCreateInfo& numRenderTargets( uint8_t numRenderTargets ) { mGraphicsPipeline.NumRenderTargets = numRenderTargets; return *this; }
    //! When pRenderPass is not null, the subpass index within the render pass. When pRenderPass is null, this member must be 0.
    GraphicsPipelineCreateInfo& subpassIndex( uint8_t subpassIndex ) { mGraphicsPipeline.SubpassIndex = subpassIndex; return *this; }
    //! Render target formats. All formats must be TEX_FORMAT_UNKNOWN when pRenderPass is not null.
    GraphicsPipelineCreateInfo& rtvFormat( size_t index, TEXTURE_FORMAT rTVFormat ) { mGraphicsPipeline.RTVFormats[index] = rTVFormat; return *this; }
    //! Depth-stencil format. Must be TEX_FORMAT_UNKNOWN when pRenderPass is not null.
    GraphicsPipelineCreateInfo& dsvFormat( TEXTURE_FORMAT dSVFormat ) { mGraphicsPipeline.DSVFormat = dSVFormat; return *this; }
    //! Multisampling parameters.
    GraphicsPipelineCreateInfo& sampleDesc( const Diligent::SampleDesc &smplDesc ) { mGraphicsPipeline.SmplDesc = smplDesc; return *this; }
    //! Pointer to the render pass object. When non-null render pass is specified, NumRenderTargets must be 0, and all RTV formats as well as DSV format must be TEX_FORMAT_UNKNOWN.
    GraphicsPipelineCreateInfo& renderPass( Diligent::IRenderPass* renderPass ){ mGraphicsPipeline.pRenderPass = renderPass; return *this; }
    //! Node mask.
    GraphicsPipelineCreateInfo& nodeMask( uint32_t nodeMask ) { mGraphicsPipeline.NodeMask = nodeMask; return *this; }

    //! Vertex shader to be used with the pipeline.
    GraphicsPipelineCreateInfo& vertexShader( const ShaderRef &vertexShader ) { mVS = vertexShader; return *this; }
    //! Vertex shader to be used with the pipeline.
    GraphicsPipelineCreateInfo& vertexShader( const ShaderCreateInfo &shaderCreateInfo )  { mVSCreateInfo = shaderCreateInfo; mVSCreateInfo.Desc.ShaderType = SHADER_TYPE_VERTEX; return *this; }
    //! Pixel shader to be used with the pipeline.
    GraphicsPipelineCreateInfo& pixelShader( const ShaderRef &pixelShader ) { mPS = pixelShader; return *this; }
    //! Pixel shader to be used with the pipeline.
    GraphicsPipelineCreateInfo& pixelShader( const ShaderCreateInfo &shaderCreateInfo )  { mPSCreateInfo = shaderCreateInfo; mPSCreateInfo.Desc.ShaderType = SHADER_TYPE_PIXEL; return *this; }
    //! Domain shader to be used with the pipeline.
    GraphicsPipelineCreateInfo& domainShader( const ShaderRef &domainShader ) { mDS = domainShader; return *this; }
    //! Domain shader to be used with the pipeline.
    GraphicsPipelineCreateInfo& domainShader( const ShaderCreateInfo &shaderCreateInfo )  { mDSCreateInfo = shaderCreateInfo; mDSCreateInfo.Desc.ShaderType = SHADER_TYPE_DOMAIN; return *this; }
    //! Hull shader to be used with the pipeline.
    GraphicsPipelineCreateInfo& hullShader( const ShaderRef &hullShader ) { mHS = hullShader; return *this; }
    //! Hull shader to be used with the pipeline.
    GraphicsPipelineCreateInfo& hullShader( const ShaderCreateInfo &shaderCreateInfo )  { mHSCreateInfo = shaderCreateInfo; mHSCreateInfo.Desc.ShaderType = SHADER_TYPE_HULL; return *this; }
    //! Geometry shader to be used with the pipeline.
    GraphicsPipelineCreateInfo& geometryShader( const ShaderRef &geometryShader ) { mGS = geometryShader; return *this; }
    //! Geometry shader to be used with the pipeline.
    GraphicsPipelineCreateInfo& geometryShader( const ShaderCreateInfo &shaderCreateInfo )  { mGSCreateInfo = shaderCreateInfo; mGSCreateInfo.Desc.ShaderType = SHADER_TYPE_GEOMETRY; return *this; }
    //! Amplification shader to be used with the pipeline.
    GraphicsPipelineCreateInfo& amplificationShader( const ShaderRef &amplificationShader ) { mAS = amplificationShader; return *this; }
    //! Amplification shader to be used with the pipeline.
    GraphicsPipelineCreateInfo& amplificationShader( const ShaderCreateInfo &shaderCreateInfo )  { mASCreateInfo = shaderCreateInfo; mASCreateInfo.Desc.ShaderType = SHADER_TYPE_AMPLIFICATION; return *this; }
    //! Mesh shader to be used with the pipeline.
    GraphicsPipelineCreateInfo& meshShader( const ShaderRef &meshShader ) { mMS = meshShader; return *this; }
    //! Mesh shader to be used with the pipeline.
    GraphicsPipelineCreateInfo& meshShader( const ShaderCreateInfo &shaderCreateInfo )  { mMSCreateInfo = shaderCreateInfo; mMSCreateInfo.Desc.ShaderType = SHADER_TYPE_MESH; return *this; }

    //! Shader resource binding allocation granularity. This member defines allocation granularity for internal resources required by the shader resource binding object instances.
    GraphicsPipelineCreateInfo& srbAllocationGranularity( uint32_t srbAllocationGranularity ) { mPSODesc.SRBAllocationGranularity = srbAllocationGranularity; return *this; }
    //! Defines which command queues this pipeline state can be used with
    GraphicsPipelineCreateInfo& commandQueueMask( uint64_t commandQueueMask ) { mPSODesc.CommandQueueMask = commandQueueMask; return *this; }

    //! Pipeline layout description
    GraphicsPipelineCreateInfo& resourceLayout( Diligent::PipelineResourceLayoutDesc resourceLayout ) { mPSODesc.ResourceLayout = resourceLayout; return *this; }
    //! Default shader resource variable type. This type will be used if shader variable description is not found in the Variables array or if Variables == nullptr
    GraphicsPipelineCreateInfo& defaultVariableType( SHADER_RESOURCE_VARIABLE_TYPE defaultVariableType ) { mPSODesc.ResourceLayout.DefaultVariableType = defaultVariableType; return *this; }
    //! Array of shader resource variable descriptions               
    GraphicsPipelineCreateInfo& variables( const std::vector<ShaderResourceVariableDesc> &variables );
    //! Array of immutable sampler descriptions                
    GraphicsPipelineCreateInfo& immutableSamplers( const std::vector<ImmutableSamplerDesc> &immutableSamplers );

    //! Specifies the Pipeline state creation flags, see Diligent::PSO_CREATE_FLAGS.
    GraphicsPipelineCreateInfo& flags( PSO_CREATE_FLAGS flags ) { mFlags = flags; return *this; }
    //! Specifies the Pipeline type. Default to PIPELINE_TYPE_GRAPHICS but can be changed to PIPELINE_TYPE_MESH.
    GraphicsPipelineCreateInfo& pipelineType( PIPELINE_TYPE type ) { mPSODesc.PipelineType = type; return *this; }
    //! Specifies the object's name.
    GraphicsPipelineCreateInfo& name( const std::string &name ) { mName = name; mPSODesc.Name = mName.c_str(); return *this; }

    GraphicsPipelineCreateInfo();
    GraphicsPipelineCreateInfo( const GraphicsPipelineCreateInfo &other );
    GraphicsPipelineCreateInfo( GraphicsPipelineCreateInfo &&other ) noexcept;
    GraphicsPipelineCreateInfo& operator=( const GraphicsPipelineCreateInfo &other );
    GraphicsPipelineCreateInfo& operator=( GraphicsPipelineCreateInfo &&other ) noexcept;
    ~GraphicsPipelineCreateInfo() = default;

    const std::vector<LayoutElement>& getLayoutElements() const { return mLayoutElements; }
    const std::vector<ShaderResourceVariableDesc>& getVariables() const { return mVariables; }
    const std::vector<ImmutableSamplerDesc>& getImmutableSamplers() const { return mImmutableSamplers; }
    const std::vector<Diligent::ShaderResourceVariableDesc>& getVariablesBase() const { return mVariablesBase; }
    const std::vector<Diligent::ImmutableSamplerDesc>& getImmutableSamplersBase() const { return mImmutableSamplersBase; }

protected:
    void updatePtrs() noexcept;
    void swap( GraphicsPipelineCreateInfo &other ) noexcept;

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

    friend PipelineStateRef createGraphicsPipelineState( RenderDevice* device, const gx::GraphicsPipelineCreateInfo &pipelineDesc );
};

PipelineStateRef createGraphicsPipelineState( const gx::GraphicsPipelineCreateInfo &pipelineDesc );
PipelineStateRef createGraphicsPipelineState( RenderDevice* device, const gx::GraphicsPipelineCreateInfo &pipelineDesc );


//! Graphics pipeline state creation attributes
struct CI_API ComputePipelineCreateInfo {
    //! Vertex shader to be used with the pipeline.
    ComputePipelineCreateInfo& shader( const ShaderRef &shader ) { mCS = shader; return *this; }
    //! Vertex shader to be used with the pipeline.
    ComputePipelineCreateInfo& shader( const ShaderCreateInfo &shaderCreateInfo ) { mCSCreateInfo = shaderCreateInfo; mCSCreateInfo.Desc.ShaderType = SHADER_TYPE_COMPUTE; return *this; }

    //! Shader resource binding allocation granularity. This member defines allocation granularity for internal resources required by the shader resource binding object instances.
    ComputePipelineCreateInfo& srbAllocationGranularity( uint32_t srbAllocationGranularity ) { mPSODesc.SRBAllocationGranularity = srbAllocationGranularity; return *this; }
    //! Defines which command queues this pipeline state can be used with
    ComputePipelineCreateInfo& commandQueueMask( uint64_t commandQueueMask ) { mPSODesc.CommandQueueMask = commandQueueMask; return *this; }

    //! Pipeline layout description
    ComputePipelineCreateInfo& resourceLayout( Diligent::PipelineResourceLayoutDesc resourceLayout ) { mPSODesc.ResourceLayout = resourceLayout; return *this; }
    //! Default shader resource variable type. This type will be used if shader variable description is not found in the Variables array or if Variables == nullptr
    ComputePipelineCreateInfo& defaultVariableType( SHADER_RESOURCE_VARIABLE_TYPE defaultVariableType ) { mPSODesc.ResourceLayout.DefaultVariableType = defaultVariableType; return *this; }
    //! Array of shader resource variable descriptions               
    ComputePipelineCreateInfo& variables( const std::vector<ShaderResourceVariableDesc> &variables );
    //! Array of immutable sampler descriptions                
    ComputePipelineCreateInfo& immutableSamplers( const std::vector<ImmutableSamplerDesc> &immutableSamplers );

    //! Specifies the Pipeline type. Default to PIPELINE_TYPE_GRAPHICS but can be changed to PIPELINE_TYPE_MESH.
    ComputePipelineCreateInfo& pipelineType( PIPELINE_TYPE type ) { mPSODesc.PipelineType = type; return *this; }
    //! Specifies the object's name.
    ComputePipelineCreateInfo& name( const std::string &name ) { mName = name; mPSODesc.Name = mName.c_str(); return *this; }

    ComputePipelineCreateInfo();
    ComputePipelineCreateInfo( const ComputePipelineCreateInfo &other );
    ComputePipelineCreateInfo( ComputePipelineCreateInfo &&other ) noexcept;
    ComputePipelineCreateInfo& operator=( const ComputePipelineCreateInfo &other );
    ComputePipelineCreateInfo& operator=( ComputePipelineCreateInfo &&other ) noexcept;
    ~ComputePipelineCreateInfo() = default;

    const std::vector<ShaderResourceVariableDesc>& getVariables() const { return mVariables; }
    const std::vector<ImmutableSamplerDesc>& getImmutableSamplers() const { return mImmutableSamplers; }
    const std::vector<Diligent::ShaderResourceVariableDesc>& getVariablesBase() const { return mVariablesBase; }
    const std::vector<Diligent::ImmutableSamplerDesc>& getImmutableSamplersBase() const { return mImmutableSamplersBase; }

protected:
    void updatePtrs() noexcept;
    void swap( ComputePipelineCreateInfo &other ) noexcept;

    std::string mName;
    ShaderRef mCS;
    ShaderCreateInfo mCSCreateInfo;

    std::vector<ShaderResourceVariableDesc> mVariables;
    std::vector<ImmutableSamplerDesc> mImmutableSamplers;

    Diligent::PipelineStateDesc     mPSODesc;
    PSO_CREATE_FLAGS                mFlags;

    std::vector<Diligent::ShaderResourceVariableDesc> mVariablesBase;
    std::vector<Diligent::ImmutableSamplerDesc> mImmutableSamplersBase;

    friend PipelineStateRef createComputePipelineState( RenderDevice* device, const gx::ComputePipelineCreateInfo &pipelineDesc );
};

PipelineStateRef createComputePipelineState( const gx::ComputePipelineCreateInfo &pipelineDesc );
PipelineStateRef createComputePipelineState( RenderDevice* device, const gx::ComputePipelineCreateInfo &pipelineDesc );

}

namespace gx = graphics;
} // namespace cinder::graphics