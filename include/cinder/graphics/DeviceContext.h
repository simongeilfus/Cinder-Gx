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

namespace cinder { namespace graphics {

//! Defines copy texture command attributes. This structure is used by IDeviceContext::CopyTexture().
struct CopyTextureAttribs : public Diligent::CopyTextureAttribs {
    //! Source texture to copy data from.
    CopyTextureAttribs& srcTexture( Texture* srcTexture ) { pSrcTexture = srcTexture; return *this; }
    //! Mip level of the source texture to copy data from.
    CopyTextureAttribs& srcMipLevel( uint32_t srcMipLevel ) { SrcMipLevel = srcMipLevel; return *this; }
    //! Array slice of the source texture to copy data from. Must be 0 for non-array textures.
    CopyTextureAttribs& srcSlice( uint32_t srcSlice ) { SrcSlice = srcSlice; return *this; }
    //! Source region to copy. Use nullptr to copy the entire subresource.
    CopyTextureAttribs& srcBox( const Box* srcBox ) { pSrcBox = srcBox; return *this; }
    //! Source texture state transition mode (see Diligent::RESOURCE_STATE_TRANSITION_MODE).
    CopyTextureAttribs& srcTextureTransitionMode( RESOURCE_STATE_TRANSITION_MODE srcTextureTransitionMode ) { SrcTextureTransitionMode = srcTextureTransitionMode; return *this; }
    //! Destination texture.
    CopyTextureAttribs& dstTexture( Texture* dstTexture ) { pDstTexture = dstTexture; return *this; }
    //! Destination mip level.
    CopyTextureAttribs& dstMipLevel( uint32_t dstMipLevel ) { DstMipLevel = dstMipLevel; return *this; }
    //! Destination array slice. Must be 0 for non-array textures.
    CopyTextureAttribs& dstSlice( uint32_t dstSlice ) { DstSlice = dstSlice; return *this; }
    //! X offset on the destination subresource.
    CopyTextureAttribs& dstX( uint32_t dstX ) { DstX = dstX; return *this; }
    //! Y offset on the destination subresource.
    CopyTextureAttribs& dstY( uint32_t dstY ) { DstY = dstY; return *this; }
    //! Z offset on the destination subresource
    CopyTextureAttribs& dstZ( uint32_t dstZ ) { DstZ = dstZ; return *this; }
    //! Destination texture state transition mode (see Diligent::RESOURCE_STATE_TRANSITION_MODE).
    CopyTextureAttribs& dstTextureTransitionMode( RESOURCE_STATE_TRANSITION_MODE dstTextureTransitionMode ) { DstTextureTransitionMode = dstTextureTransitionMode; return *this; }

    CopyTextureAttribs() noexcept : Diligent::CopyTextureAttribs() {}
    CopyTextureAttribs( ITexture* srcTexture, RESOURCE_STATE_TRANSITION_MODE srcTextureTransitionMode, ITexture* dstTexture, RESOURCE_STATE_TRANSITION_MODE dstTextureTransitionMode ) noexcept
        : Diligent::CopyTextureAttribs( srcTexture, srcTextureTransitionMode, dstTexture, dstTextureTransitionMode ) {}
};

}

namespace gx = graphics;
} // namespace cinder::graphics