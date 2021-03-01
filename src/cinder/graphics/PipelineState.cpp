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

GraphicsPipelineStateCreateInfo::GraphicsPipelineStateCreateInfo() noexcept
{
	PSODesc.PipelineType = Diligent::PIPELINE_TYPE_GRAPHICS;
	GraphicsPipeline.RTVFormats[0] = app::getSwapChainColorFormat();
	GraphicsPipeline.DSVFormat = app::getSwapChainDepthFormat();
	GraphicsPipeline.NumRenderTargets = 1;
}

GraphicsPipelineStateCreateInfo::GraphicsPipelineStateCreateInfo( const GraphicsPipelineStateCreateInfo &other ) noexcept
{
	copy( other );
}

GraphicsPipelineStateCreateInfo& GraphicsPipelineStateCreateInfo::operator=( GraphicsPipelineStateCreateInfo other ) noexcept
{
	copy( other );
	return *this;
}

void GraphicsPipelineStateCreateInfo::copy( const GraphicsPipelineStateCreateInfo &other )
{
	PSODesc = other.PSODesc;
	Flags = other.Flags;
	GraphicsPipeline = other.GraphicsPipeline;
	mVS = other.mVS;
	mPS = other.mPS;
	mDS = other.mDS;
	mHS = other.mHS;
	mGS = other.mGS;
	mAS = other.mAS;
	mMS = other.mMS;
	mLayoutElements = other.mLayoutElements;
	mVariables = other.mVariables;
	mImmutableSamplers = other.mImmutableSamplers;

	pVS = mVS ? mVS : other.pVS;
	pPS = mPS ? mPS : other.pPS;
	pDS = mDS ? mDS : other.pDS;
	pHS = mHS ? mHS : other.pHS;
	pGS = mGS ? mGS : other.pGS;
	pAS = mAS ? mAS : other.pAS;
	pMS = mMS ? mMS : other.pMS;

	if( ! mLayoutElements.empty() ) {
		GraphicsPipeline.InputLayout.LayoutElements = mLayoutElements.data();
		GraphicsPipeline.InputLayout.NumElements = mLayoutElements.size();
	}

	if( ! mVariables.empty() ) {
		PSODesc.ResourceLayout.Variables = mVariables.data();
		PSODesc.ResourceLayout.NumVariables = mVariables.size();
	}

	if( ! mImmutableSamplers.empty() ) {
		PSODesc.ResourceLayout.ImmutableSamplers = mImmutableSamplers.data();
		PSODesc.ResourceLayout.NumImmutableSamplers = mImmutableSamplers.size();
	}
}

}

namespace gx = graphics;
} // namespace cinder::graphics