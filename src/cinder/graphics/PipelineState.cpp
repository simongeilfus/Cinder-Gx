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

GraphicsPipelineStateCreateInfo::GraphicsPipelineStateCreateInfo()
{
	PSODesc.PipelineType = Diligent::PIPELINE_TYPE_GRAPHICS;
	GraphicsPipeline.RTVFormats[0] = app::getSwapChainColorFormat();
	GraphicsPipeline.DSVFormat = app::getSwapChainDepthFormat();
	GraphicsPipeline.NumRenderTargets = 1;
}

GraphicsPipelineStateCreateInfo::GraphicsPipelineStateCreateInfo( const GraphicsPipelineStateCreateInfo &other )
	: mName( other.mName ),
	mVS( other.mVS ),
	mPS( other.mPS ),
	mDS( other.mDS ),
	mHS( other.mHS ),
	mGS( other.mGS ),
	mAS( other.mAS ),
	mMS( other.mMS ),
	mLayoutElements( other.mLayoutElements ),
	mVariables( other.mVariables ),
	mImmutableSamplers( other.mImmutableSamplers )
{
	PSODesc = other.PSODesc;
	Flags = other.Flags;
	GraphicsPipeline = other.GraphicsPipeline;
	pVS = other.pVS;
	pPS = other.pPS;
	pDS = other.pDS;
	pHS = other.pHS;
	pGS = other.pGS;
	pAS = other.pAS;
	pMS = other.pMS;

	updatePtrs();
}

GraphicsPipelineStateCreateInfo::GraphicsPipelineStateCreateInfo( GraphicsPipelineStateCreateInfo &&other ) noexcept
	: GraphicsPipelineStateCreateInfo()
{
	other.swap( *this );
	updatePtrs();
}

GraphicsPipelineStateCreateInfo& GraphicsPipelineStateCreateInfo::operator=( const GraphicsPipelineStateCreateInfo &other )
{
	GraphicsPipelineStateCreateInfo( other ).swap( *this );
	updatePtrs();
	return *this;
}

GraphicsPipelineStateCreateInfo& GraphicsPipelineStateCreateInfo::operator=( GraphicsPipelineStateCreateInfo &&other ) noexcept
{
	other.swap( *this );
	updatePtrs();
	return *this;
}

void GraphicsPipelineStateCreateInfo::updatePtrs() noexcept
{
	if( mVS ) pVS = mVS;
	if( mPS ) pPS = mPS;
	if( mDS ) pDS = mDS;
	if( mHS ) pHS = mHS;
	if( mGS ) pGS = mGS;
	if( mAS ) pAS = mAS;
	if( mMS ) pMS = mMS;
	if( ! mName.empty() ) PSODesc.Name = mName.c_str();

	if( ! mLayoutElements.empty() ) {
		GraphicsPipeline.InputLayout.LayoutElements = mLayoutElements.data();
		GraphicsPipeline.InputLayout.NumElements = static_cast<uint32_t>( mLayoutElements.size() );
	}

	if( ! mVariables.empty() ) {
		mVariablesBase.resize( mVariables.size() );
		for( size_t i = 0; i < mVariables.size(); ++i ) {
			mVariablesBase[i] = mVariables[i];
		}
		PSODesc.ResourceLayout.Variables = mVariablesBase.data();
		PSODesc.ResourceLayout.NumVariables = static_cast<uint32_t>( mVariablesBase.size() );
	}

	if( ! mImmutableSamplers.empty() ) {
		mImmutableSamplersBase.resize( mImmutableSamplers.size() );
		for( size_t i = 0; i < mImmutableSamplers.size(); ++i ) {
			mImmutableSamplersBase[i] = mImmutableSamplers[i];
		}
		PSODesc.ResourceLayout.ImmutableSamplers = mImmutableSamplersBase.data();
		PSODesc.ResourceLayout.NumImmutableSamplers = static_cast<uint32_t>( mImmutableSamplersBase.size() );
	}
}

GraphicsPipelineStateCreateInfo& GraphicsPipelineStateCreateInfo::inputLayout( const std::vector<LayoutElement> &elements ) 
{ 
	mLayoutElements = elements; 
	GraphicsPipeline.InputLayout.LayoutElements = mLayoutElements.data(); 
	GraphicsPipeline.InputLayout.NumElements = static_cast<Diligent::Uint32>( mLayoutElements.size() ); 
	return *this; 
}

GraphicsPipelineStateCreateInfo& GraphicsPipelineStateCreateInfo::variables( const std::vector<ShaderResourceVariableDesc> &variables ) 
{ 
	mVariables = variables;
	mVariablesBase.resize( mVariables.size() );
	for( size_t i = 0; i < mVariables.size(); ++i ) {
		mVariablesBase[i] = mVariables[i];
	}
	PSODesc.ResourceLayout.Variables = mVariablesBase.data();
	PSODesc.ResourceLayout.NumVariables = static_cast<uint32_t>( mVariablesBase.size() );
	return *this; 
}

GraphicsPipelineStateCreateInfo& GraphicsPipelineStateCreateInfo::immutableSamplers( const std::vector<ImmutableSamplerDesc> &immutableSamplers ) 
{ 
	mImmutableSamplers = immutableSamplers;
	mImmutableSamplersBase.resize( mImmutableSamplers.size() );
	for( size_t i = 0; i < mImmutableSamplers.size(); ++i ) {
		mImmutableSamplersBase[i] = mImmutableSamplers[i];
	}
	PSODesc.ResourceLayout.ImmutableSamplers = mImmutableSamplersBase.data();
	PSODesc.ResourceLayout.NumImmutableSamplers = static_cast<uint32_t>( mImmutableSamplersBase.size() );
	return *this; 
}

void GraphicsPipelineStateCreateInfo::swap( GraphicsPipelineStateCreateInfo &other ) noexcept
{
	std::swap( PSODesc, other.PSODesc );
	std::swap( Flags, other.Flags );
	std::swap( GraphicsPipeline, other.GraphicsPipeline );
	std::swap( pVS, other.pVS );
	std::swap( pPS, other.pPS );
	std::swap( pDS, other.pDS );
	std::swap( pHS, other.pHS );
	std::swap( pGS, other.pGS );
	std::swap( pAS, other.pAS );
	std::swap( pMS, other.pMS );

	std::swap( mName, other.mName );
	std::swap( mVS, other.mVS );
	std::swap( mPS, other.mPS );
	std::swap( mDS, other.mDS );
	std::swap( mHS, other.mHS );
	std::swap( mGS, other.mGS );
	std::swap( mAS, other.mAS );
	std::swap( mMS, other.mMS );
	std::swap( mLayoutElements, other.mLayoutElements );
	std::swap( mVariables, other.mVariables );
	std::swap( mImmutableSamplers, other.mImmutableSamplers );
}

PipelineStateRef createGraphicsPipelineState( const gx::GraphicsPipelineStateCreateInfo &createInfo )
{
	return createGraphicsPipelineState( app::getRenderDevice(), createInfo );
}

PipelineStateRef createGraphicsPipelineState( RenderDevice* device, const gx::GraphicsPipelineStateCreateInfo &createInfo )
{
	PipelineStateRef pipelineState;
	device->CreateGraphicsPipelineState( createInfo, &pipelineState );
	return pipelineState;
}

}

namespace gx = graphics;
} // namespace cinder::graphics