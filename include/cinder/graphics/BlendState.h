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
#include "DiligentCore/Graphics/GraphicsEngine/interface/BlendState.h"

namespace cinder { namespace graphics {

//! Describes a blend state for a single render target. This structure is used by BlendStateDesc to describe blend states for render targets
struct CI_API RenderTargetBlendDesc : public Diligent::RenderTargetBlendDesc {
    //! Enable or disable blending for this render target. Default value: False.
    RenderTargetBlendDesc& blendEnable( bool blendEnable ) { BlendEnable = blendEnable; return *this; }
    //! Enable or disable a logical operation for this render target. Default value: False.
    RenderTargetBlendDesc& logicOperationEnable( bool logicOperationEnable ) { LogicOperationEnable = logicOperationEnable; return *this; }
    //! Specifies the blend factor to apply to the RGB value output from the pixel shader. Default value: Diligent::BLEND_FACTOR_ONE.
    RenderTargetBlendDesc& srcBlend( BLEND_FACTOR srcBlend ) { SrcBlend = srcBlend; return *this; }
    //! Specifies the blend factor to apply to the RGB value in the render target. Default value: Diligent::BLEND_FACTOR_ZERO.
    RenderTargetBlendDesc& destBlend( BLEND_FACTOR destBlend ) { DestBlend = destBlend; return *this; }
    //! Defines how to combine the source and destination RGB values after applying the SrcBlend and DestBlend factors. Default value: Diligent::BLEND_OPERATION_ADD.
    RenderTargetBlendDesc& blendOp( BLEND_OPERATION blendOp ) { BlendOp = blendOp; return *this; }
    //! Specifies the blend factor to apply to the alpha value output from the pixel shader. Blend factors that end in _COLOR are not allowed. Default value: Diligent::BLEND_FACTOR_ONE.
    RenderTargetBlendDesc& srcBlendAlpha( BLEND_FACTOR srcBlendAlpha ) { SrcBlendAlpha = srcBlendAlpha; return *this; }
    //! Specifies the blend factor to apply to the alpha value in the render target. Blend factors that end in _COLOR are not allowed. Default value: Diligent::BLEND_FACTOR_ZERO.
    RenderTargetBlendDesc& destBlendAlpha( BLEND_FACTOR destBlendAlpha ) { DestBlendAlpha = destBlendAlpha; return *this; }
    //! Defines how to combine the source and destination alpha values after applying the SrcBlendAlpha and DestBlendAlpha factors. Default value: Diligent::BLEND_OPERATION_ADD.
    RenderTargetBlendDesc& blendOpAlpha( BLEND_OPERATION blendOpAlpha ) { BlendOpAlpha = blendOpAlpha; return *this; }
    //! Defines logical operation for the render target. Default value: Diligent::LOGIC_OP_NOOP.
    RenderTargetBlendDesc& logicOp( LOGIC_OPERATION logicOp ) { LogicOp = logicOp; return *this; }
    //! Render target write mask. Default value: Diligent::COLOR_MASK_ALL.
    RenderTargetBlendDesc& renderTargetWriteMask( uint8_t renderTargetWriteMask ) { RenderTargetWriteMask = renderTargetWriteMask; return *this; }
};

//! Blend state description. This structure describes the blend state and is part of the GraphicsPipelineDesc.
struct CI_API BlendStateDesc : public Diligent::BlendStateDesc {
    //! Specifies whether to use alpha-to-coverage as a multisampling technique when setting a pixel to a render target. Default value: False.
    BlendStateDesc& alphaToCoverageEnable( bool enabled ) { AlphaToCoverageEnable = enabled; return *this; }
    //! Specifies whether to enable independent blending in simultaneous render targets. If set to False, only RenderTargets[0] is used. Default value: False.
    BlendStateDesc& independentBlendEnable( bool enabled ) { IndependentBlendEnable = enabled; return *this; }
    //! Specifies a RenderTargetBlendDesc structures that describe the blend states for the render target at the specified index.
    BlendStateDesc& renderTarget( size_t index, const Diligent::RenderTargetBlendDesc &desc ) { RenderTargets[index] = desc; return *this; }
};

}

namespace gx = graphics;
} // namespace cinder::graphics