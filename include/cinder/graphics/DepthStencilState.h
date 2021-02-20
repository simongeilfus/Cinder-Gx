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
#include "DiligentCore/Graphics/GraphicsEngine/interface/DepthStencilState.h"

namespace cinder { namespace graphics {

//! Describes stencil operations that are performed based on the results of depth test. 
struct CI_API StencilOpDesc : public Diligent::StencilOpDesc {
    //! The stencil operation to perform when stencil testing fails. Default value: Diligent::STENCIL_OP_KEEP.
    StencilOpDesc& stencilFailOp( STENCIL_OP stencilFailOp ) { StencilFailOp = stencilFailOp; return *this; }
    //! The stencil operation to perform when stencil testing passes and depth testing fails. Default value: Diligent::STENCIL_OP_KEEP.
    StencilOpDesc& stencilDepthFailOp( STENCIL_OP stencilDepthFailOp ) { StencilDepthFailOp = stencilDepthFailOp; return *this; }
    //! The stencil operation to perform when stencil testing and depth testing both pass. Default value: Diligent::STENCIL_OP_KEEP.
    StencilOpDesc& stencilPassOp( STENCIL_OP stencilPassOp ) { StencilPassOp = stencilPassOp; return *this; }
    //! A function that compares stencil data against existing stencil data. Default value: Diligent::COMPARISON_FUNC_ALWAYS. See Diligent::COMPARISON_FUNCTION.
    StencilOpDesc& stencilFunc( COMPARISON_FUNCTION stencilFunc ) { StencilFunc = stencilFunc; return *this; }
};

//! Depth stencil state description
struct CI_API DepthStencilStateDesc : public Diligent::DepthStencilStateDesc {
    //! Enable depth-stencil operations. When it is set to False, depth test always passes, depth writes are disabled, and no stencil operations are performed. Default value: True.
    DepthStencilStateDesc& depthEnable( bool depthEnable ) { DepthEnable = depthEnable; return *this; }
    //! Enable or disable writes to a depth buffer. Default value: True.
    DepthStencilStateDesc& depthWriteEnable( bool depthWriteEnable ) { DepthWriteEnable = depthWriteEnable; return *this; }
    //! A function that compares depth data against existing depth data. See Diligent::COMPARISON_FUNCTION for details. Default value: Diligent::COMPARISON_FUNC_LESS.
    DepthStencilStateDesc& depthFunc( COMPARISON_FUNCTION depthFunc ) { DepthFunc = depthFunc; return *this; }
    //! Enable stencil opertaions. Default value: False.
    DepthStencilStateDesc& stencilEnable( bool stencilEnable ) { StencilEnable = stencilEnable; return *this; }
    //! Identify which bits of the depth-stencil buffer are accessed when reading stencil data. Default value: 0xFF.
    DepthStencilStateDesc& stencilReadMask( uint8_t stencilReadMask ) { StencilReadMask = stencilReadMask; return *this; }
    //! Identify which bits of the depth-stencil buffer are accessed when writing stencil data. Default value: 0xFF.
    DepthStencilStateDesc& stencilWriteMask( uint8_t stencilWriteMask ) { StencilWriteMask = stencilWriteMask; return *this; }
    //! Identify stencil operations for the front-facing triangles, see Diligent::StencilOpDesc.
    DepthStencilStateDesc& frontFace( const StencilOpDesc &frontFace ) { FrontFace = frontFace; return *this; }
    /// Identify stencil operations for the back-facing triangles, see Diligent::StencilOpDesc.
    DepthStencilStateDesc& backFace( const StencilOpDesc &backFace ) { BackFace = backFace; return *this; }

};

}

namespace gx = graphics;
} // namespace cinder::graphics