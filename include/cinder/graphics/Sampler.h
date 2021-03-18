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

struct SamplerDesc : public Diligent::SamplerDesc {
    //! Texture minification filter, see Diligent::FILTER_TYPE for details. Default value: Diligent::FILTER_TYPE_LINEAR.
    SamplerDesc& minFilter( FILTER_TYPE minFilter ) { MinFilter = minFilter; return *this; }
    //! Texture magnification filter, see Diligent::FILTER_TYPE for details. Default value: Diligent::FILTER_TYPE_LINEAR.
    SamplerDesc& magFilter( FILTER_TYPE magFilter ) { MagFilter = magFilter; return *this; }
    //! Mip filter, see Diligent::FILTER_TYPE for details. Only FILTER_TYPE_POINT, FILTER_TYPE_LINEAR, FILTER_TYPE_ANISOTROPIC, and FILTER_TYPE_COMPARISON_ANISOTROPIC are allowed.Default value: Diligent::FILTER_TYPE_LINEAR.
    SamplerDesc& mipFilter( FILTER_TYPE mipFilter ) { MipFilter = mipFilter; return *this; }
    //! Texture address mode for U coordinate, see Diligent::TEXTURE_ADDRESS_MODE for details Default value: Diligent::TEXTURE_ADDRESS_CLAMP.
    SamplerDesc& addressU( TEXTURE_ADDRESS_MODE addressU ) { AddressU = addressU; return *this; }
    //! Texture address mode for V coordinate, see Diligent::TEXTURE_ADDRESS_MODE for details Default value: Diligent::TEXTURE_ADDRESS_CLAMP.
    SamplerDesc& addressV( TEXTURE_ADDRESS_MODE addressV ) { AddressV = addressV; return *this; }
    //! Texture address mode for W coordinate, see Diligent::TEXTURE_ADDRESS_MODE for details Default value: Diligent::TEXTURE_ADDRESS_CLAMP.
    SamplerDesc& addressW( TEXTURE_ADDRESS_MODE addressW ) { AddressW = addressW; return *this; }
    //! Offset from the calculated mipmap level. For example, if a sampler calculates that a texture should be sampled at mipmap level 1.2 and MipLODBias is 2.3, then the texture will be sampled at mipmap level 3.5. Default value: 0.
    SamplerDesc& mipLODBias( Float32 mipLODBias ) { MipLODBias = mipLODBias; return *this; }
    //! Maximum anisotropy level for the anisotropic filter. Default value: 0.
    SamplerDesc& maxAnisotropy( Uint32 maxAnisotropy ) { MaxAnisotropy = maxAnisotropy; return *this; }
    //! A function that compares sampled data against existing sampled data when comparsion filter is used. Default value: Diligent::COMPARISON_FUNC_NEVER.
    SamplerDesc& comparisonFunc( COMPARISON_FUNCTION comparisonFunc ) { ComparisonFunc = comparisonFunc; return *this; }
    //! Border color to use if TEXTURE_ADDRESS_BORDER is specified for AddressU, AddressV, or AddressW. Default value: {0,0,0,0}
    SamplerDesc& borderColor( const ColorAf &color ) { BorderColor[0] = color.r; BorderColor[1] = color.g; BorderColor[2] = color.b; BorderColor[3] = color.a; return *this; }
    //! Border color to use if TEXTURE_ADDRESS_BORDER is specified for AddressU, AddressV, or AddressW. Default value: {0,0,0,0}
    SamplerDesc& borderColor( const vec4 &color ) { BorderColor[0] = color.x; BorderColor[1] = color.y; BorderColor[2] = color.z; BorderColor[3] = color.w; return *this; }
    //! Specifies the minimum value that LOD is clamped to before accessing the texture MIP levels. Must be less than or equal to MaxLOD. Default value: 0.
    SamplerDesc& minLOD( float minLOD ) { MinLOD = minLOD; return *this; }
    //! Specifies the maximum value that LOD is clamped to before accessing the texture MIP levels. Must be greater than or equal to MinLOD. Default value: +FLT_MAX.
    SamplerDesc& maxLOD( float maxLOD ) { MaxLOD = maxLOD; return *this; }

    SamplerDesc() noexcept {}
    SamplerDesc( FILTER_TYPE _MinFilter, FILTER_TYPE _MagFilter, FILTER_TYPE _MipFilter, TEXTURE_ADDRESS_MODE _AddressU = SamplerDesc{}.AddressU, TEXTURE_ADDRESS_MODE _AddressV = SamplerDesc{}.AddressV, TEXTURE_ADDRESS_MODE _AddressW = SamplerDesc{}.AddressW, 
    Float32 _MipLODBias = SamplerDesc{}.MipLODBias, Uint32 _MaxAnisotropy = SamplerDesc{}.MaxAnisotropy, COMPARISON_FUNCTION  _ComparisonFunc = SamplerDesc{}.ComparisonFunc, float _MinLOD = SamplerDesc{}.MinLOD, float _MaxLOD = SamplerDesc{}.MaxLOD  ) 
        : Diligent::SamplerDesc( _MinFilter, _MagFilter, _MipFilter, _AddressU, _AddressV, _AddressW, _MipLODBias, _MaxAnisotropy, _ComparisonFunc, _MinLOD, _MaxLOD )
    {
    }
};

//! Creates a new Sampler object using the default RenderDevice
CI_API SamplerRef createSampler( const Diligent::SamplerDesc &desc );

}

namespace gx = graphics;
} // namespace cinder::graphics