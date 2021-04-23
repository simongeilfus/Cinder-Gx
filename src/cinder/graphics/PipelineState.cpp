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


ImmutableSamplerDesc::ImmutableSamplerDesc( SHADER_TYPE shaderStages, const std::string &samplerOrTextureName, const SamplerDesc& desc ) noexcept
	: mSamplerOrTextureName( samplerOrTextureName ), 
	Diligent::ImmutableSamplerDesc( shaderStages, mSamplerOrTextureName.c_str(), desc )
{
	updatePtrs();
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

GraphicsPipelineCreateInfo::GraphicsPipelineCreateInfo()
{
	mFlags = PSO_CREATE_FLAG_NONE;
	mPSODesc.PipelineType = Diligent::PIPELINE_TYPE_GRAPHICS;
	mGraphicsPipeline.RTVFormats[0] = app::getSwapChainColorFormat();
	mGraphicsPipeline.DSVFormat = app::getSwapChainDepthFormat();
	mGraphicsPipeline.NumRenderTargets = 1;
}

GraphicsPipelineCreateInfo::GraphicsPipelineCreateInfo( const GraphicsPipelineCreateInfo &other )
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

GraphicsPipelineCreateInfo::GraphicsPipelineCreateInfo( GraphicsPipelineCreateInfo &&other ) noexcept
	: GraphicsPipelineCreateInfo()
{
	other.swap( *this );
	updatePtrs();
}

GraphicsPipelineCreateInfo& GraphicsPipelineCreateInfo::operator=( const GraphicsPipelineCreateInfo &other )
{
	GraphicsPipelineCreateInfo( other ).swap( *this );
	updatePtrs();
	return *this;
}

GraphicsPipelineCreateInfo& GraphicsPipelineCreateInfo::operator=( GraphicsPipelineCreateInfo &&other ) noexcept
{
	other.swap( *this );
	updatePtrs();
	return *this;
}

void GraphicsPipelineCreateInfo::updatePtrs() noexcept
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

GraphicsPipelineCreateInfo& GraphicsPipelineCreateInfo::alphaBlending()
{
	for( uint8_t i = 0; i < mGraphicsPipeline.NumRenderTargets; ++i ) {
		alphaBlending( i );
	}
	return *this;
}

GraphicsPipelineCreateInfo& GraphicsPipelineCreateInfo::alphaBlending( size_t renderTargetIndex )
{
	mGraphicsPipeline.BlendDesc.RenderTargets[renderTargetIndex].BlendEnable = true;
	mGraphicsPipeline.BlendDesc.RenderTargets[renderTargetIndex].SrcBlend = BLEND_FACTOR_SRC_ALPHA;
	mGraphicsPipeline.BlendDesc.RenderTargets[renderTargetIndex].DestBlend = BLEND_FACTOR_INV_SRC_ALPHA;
	mGraphicsPipeline.BlendDesc.RenderTargets[renderTargetIndex].BlendOp = BLEND_OPERATION_ADD;
	mGraphicsPipeline.BlendDesc.RenderTargets[renderTargetIndex].SrcBlendAlpha = BLEND_FACTOR_ONE;
	mGraphicsPipeline.BlendDesc.RenderTargets[renderTargetIndex].DestBlendAlpha = BLEND_FACTOR_ZERO;
	mGraphicsPipeline.BlendDesc.RenderTargets[renderTargetIndex].BlendOpAlpha = BLEND_OPERATION_ADD;
			
	return *this;
}

GraphicsPipelineCreateInfo& GraphicsPipelineCreateInfo::alphaBlendingPremult()
{
	for( uint8_t i = 0; i < mGraphicsPipeline.NumRenderTargets; ++i ) {
		alphaBlendingPremult( i );
	}
	return *this;
}

GraphicsPipelineCreateInfo& GraphicsPipelineCreateInfo::alphaBlendingPremult( size_t renderTargetIndex )
{
	mGraphicsPipeline.BlendDesc.RenderTargets[renderTargetIndex].BlendEnable = true;
	mGraphicsPipeline.BlendDesc.RenderTargets[renderTargetIndex].SrcBlend = BLEND_FACTOR_ONE;
	mGraphicsPipeline.BlendDesc.RenderTargets[renderTargetIndex].DestBlend = BLEND_FACTOR_INV_SRC_ALPHA;
	mGraphicsPipeline.BlendDesc.RenderTargets[renderTargetIndex].BlendOp = BLEND_OPERATION_ADD;
	mGraphicsPipeline.BlendDesc.RenderTargets[renderTargetIndex].SrcBlendAlpha = BLEND_FACTOR_ONE;
	mGraphicsPipeline.BlendDesc.RenderTargets[renderTargetIndex].DestBlendAlpha = BLEND_FACTOR_ZERO;
	mGraphicsPipeline.BlendDesc.RenderTargets[renderTargetIndex].BlendOpAlpha = BLEND_OPERATION_ADD;
	return *this;
}

GraphicsPipelineCreateInfo& GraphicsPipelineCreateInfo::additiveBlending()
{
	for( uint8_t i = 0; i < mGraphicsPipeline.NumRenderTargets; ++i ) {
		additiveBlending( i );
	}
	return *this;
}

GraphicsPipelineCreateInfo& GraphicsPipelineCreateInfo::additiveBlending( size_t renderTargetIndex )
{
	mGraphicsPipeline.BlendDesc.RenderTargets[renderTargetIndex].BlendEnable = true;
	mGraphicsPipeline.BlendDesc.RenderTargets[renderTargetIndex].SrcBlend = BLEND_FACTOR_SRC_ALPHA;
	mGraphicsPipeline.BlendDesc.RenderTargets[renderTargetIndex].DestBlend = BLEND_FACTOR_ONE;
	mGraphicsPipeline.BlendDesc.RenderTargets[renderTargetIndex].BlendOp = BLEND_OPERATION_ADD;
	mGraphicsPipeline.BlendDesc.RenderTargets[renderTargetIndex].SrcBlendAlpha = BLEND_FACTOR_ONE;
	mGraphicsPipeline.BlendDesc.RenderTargets[renderTargetIndex].DestBlendAlpha = BLEND_FACTOR_ZERO;
	mGraphicsPipeline.BlendDesc.RenderTargets[renderTargetIndex].BlendOpAlpha = BLEND_OPERATION_ADD;
	return *this;
}

GraphicsPipelineCreateInfo& GraphicsPipelineCreateInfo::inputLayout( const std::vector<LayoutElement> &elements ) 
{ 
	mLayoutElements = elements; 
	mGraphicsPipeline.InputLayout.LayoutElements = mLayoutElements.data(); 
	mGraphicsPipeline.InputLayout.NumElements = static_cast<Diligent::Uint32>( mLayoutElements.size() ); 
	return *this; 
}

GraphicsPipelineCreateInfo& GraphicsPipelineCreateInfo::variables( const std::vector<ShaderResourceVariableDesc> &variables ) 
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

GraphicsPipelineCreateInfo& GraphicsPipelineCreateInfo::immutableSamplers( const std::vector<ImmutableSamplerDesc> &immutableSamplers ) 
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

void GraphicsPipelineCreateInfo::swap( GraphicsPipelineCreateInfo &other ) noexcept
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

PipelineStateRef createGraphicsPipelineState( const gx::GraphicsPipelineCreateInfo &pipelineDesc )
{
	return createGraphicsPipelineState( app::getRenderDevice(), pipelineDesc );
}

PipelineStateRef createGraphicsPipelineState( RenderDevice* device, const gx::GraphicsPipelineCreateInfo &pipelineDesc )
{
	PipelineStateRef pipelineState;

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

ComputePipelineCreateInfo::ComputePipelineCreateInfo()
{
	mFlags = PSO_CREATE_FLAG_NONE;
	mPSODesc.PipelineType = Diligent::PIPELINE_TYPE_COMPUTE;
}

ComputePipelineCreateInfo::ComputePipelineCreateInfo( const ComputePipelineCreateInfo &other )
	: mName( other.mName ),
	mCS( other.mCS ),
	mCSCreateInfo( other.mCSCreateInfo ),
	mVariables( other.mVariables ),
	mImmutableSamplers( other.mImmutableSamplers ),
	mPSODesc( other.mPSODesc ),
	mFlags( other.mFlags )
{
	updatePtrs();
}

ComputePipelineCreateInfo::ComputePipelineCreateInfo( ComputePipelineCreateInfo &&other ) noexcept
	: ComputePipelineCreateInfo()
{
	other.swap( *this );
	updatePtrs();
}

ComputePipelineCreateInfo& ComputePipelineCreateInfo::operator=( const ComputePipelineCreateInfo &other )
{
	ComputePipelineCreateInfo( other ).swap( *this );
	updatePtrs();
	return *this;
}

ComputePipelineCreateInfo& ComputePipelineCreateInfo::operator=( ComputePipelineCreateInfo &&other ) noexcept
{
	other.swap( *this );
	updatePtrs();
	return *this;
}

void ComputePipelineCreateInfo::updatePtrs() noexcept
{
	if( ! mName.empty() ) mPSODesc.Name = mName.c_str();

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

ComputePipelineCreateInfo& ComputePipelineCreateInfo::variables( const std::vector<ShaderResourceVariableDesc> &variables )
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

ComputePipelineCreateInfo& ComputePipelineCreateInfo::immutableSamplers( const std::vector<ImmutableSamplerDesc> &immutableSamplers )
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

void ComputePipelineCreateInfo::swap( ComputePipelineCreateInfo &other ) noexcept
{
	std::swap( mPSODesc, other.mPSODesc );
	std::swap( mFlags, other.mFlags );

	std::swap( mName, other.mName );
	std::swap( mCS, other.mCS );
	std::swap( mCSCreateInfo, other.mCSCreateInfo );
	std::swap( mVariables, other.mVariables );
	std::swap( mImmutableSamplers, other.mImmutableSamplers );
}

PipelineStateRef createComputePipelineState( const gx::ComputePipelineCreateInfo &pipelineDesc )
{
	return createComputePipelineState( app::getRenderDevice(), pipelineDesc );
}

PipelineStateRef createComputePipelineState( RenderDevice* device, const gx::ComputePipelineCreateInfo &pipelineDesc )
{
	PipelineStateRef pipelineState;

	gx::ComputePipelineStateCreateInfo createInfo;
	createInfo.Flags = pipelineDesc.mFlags;
	createInfo.PSODesc = pipelineDesc.mPSODesc;

	// initialize shaders if no pointer is present but a ShaderCreateInfo is available
	ShaderRef cs;
	if( ! createInfo.pCS && pipelineDesc.mCSCreateInfo.Desc.ShaderType != SHADER_TYPE_UNKNOWN ) {
		device->CreateShader( pipelineDesc.mCSCreateInfo, &cs );
		createInfo.pCS = cs;
	}

	// set shader pointers
	if( pipelineDesc.mCS ) createInfo.pCS = pipelineDesc.mCS.RawPtr<Shader>();

	device->CreateComputePipelineState( createInfo, &pipelineState );
	return pipelineState;
}


}
} // namespace cinder::graphics