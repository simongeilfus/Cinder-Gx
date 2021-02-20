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
#include "DiligentCore/Graphics/GraphicsEngine/interface/RasterizerState.h"

namespace cinder { namespace graphics {

struct CI_API RasterizerStateDesc : public Diligent::RasterizerStateDesc {
    //! Determines traingle fill mode, see Diligent::FILL_MODE for details. Default value: Diligent::FILL_MODE_SOLID.
    RasterizerStateDesc& fillMode( FILL_MODE fillMode ) { FillMode = fillMode; return *this; }
    //! Determines traingle cull mode, see Diligent::CULL_MODE for details. Default value: Diligent::CULL_MODE_BACK.
    RasterizerStateDesc& cullMode( CULL_MODE cullMode ) { CullMode = cullMode; return *this; }
    //! Determines if a triangle is front- or back-facing. If this parameter is True, a triangle will be considered front-facing if its vertices are counter-clockwise on the render target and considered back-facing if they are clockwise. If this parameter is False, the opposite is true. Default value: False.
    RasterizerStateDesc& frontCounterClockwise( bool frontCounterClockwise ) { FrontCounterClockwise = frontCounterClockwise; return *this; }
    //! Enable clipping against near and far clip planes. Default value: True.
    RasterizerStateDesc& depthClipEnable( bool depthClipEnable ) { DepthClipEnable = depthClipEnable; return *this; }
    //! Enable scissor-rectangle culling. All pixels outside an active scissor rectangle are culled. Default value: False.
    RasterizerStateDesc& scissorEnable( bool scissorEnable ) { ScissorEnable = scissorEnable; return *this; }
    //! Specifies whether to enable line antialiasing. Default value: False.
    RasterizerStateDesc& antialiasedLineEnable( bool antialiasedLineEnable ) { AntialiasedLineEnable = antialiasedLineEnable; return *this; }
    //! Constant value added to the depth of a given pixel. Default value: 0.
    RasterizerStateDesc& depthBias( int32_t depthBias ) { DepthBias = depthBias; return *this; }
    //! Maximum depth bias of a pixel. \warning Depth bias clamp is not available in OpenGL. Default value: 0.
    RasterizerStateDesc& depthBiasClamp( float depthBiasClamp ) { DepthBiasClamp = depthBiasClamp; return *this; }
    //! Scalar that scales the given pixel's slope before adding to the pixel's depth. Default value: 0.
    RasterizerStateDesc& slopeScaledDepthBias( float slopeScaledDepthBias ) { SlopeScaledDepthBias = slopeScaledDepthBias; return *this; }
};

}

namespace gx = graphics;
} // namespace cinder::graphics