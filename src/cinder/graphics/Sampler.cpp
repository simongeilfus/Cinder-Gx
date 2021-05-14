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

#include "cinder/graphics/Sampler.h"
#include "cinder/app/RendererGx.h"

using namespace std;
using namespace ci::app;

namespace cinder { namespace graphics {

SamplerDesc::SamplerDesc( const Diligent::SamplerDesc &other )
	: SamplerDesc( other.MinFilter, other.MagFilter, other.MipFilter, other.AddressU, other.AddressV, other.AddressW, other.MipLODBias, other.MaxAnisotropy, other.ComparisonFunc, other.MinLOD, other.MaxLOD )
{
	memcpy( &BorderColor, &other.BorderColor, sizeof( Float32 ) * 4 );
}

SamplerDesc::SamplerDesc( FILTER_TYPE minFilter, FILTER_TYPE magFilter, FILTER_TYPE mipFilter, TEXTURE_ADDRESS_MODE addressU, TEXTURE_ADDRESS_MODE addressV, TEXTURE_ADDRESS_MODE addressW, Float32 mipLODBias, Uint32 maxAnisotropy, COMPARISON_FUNCTION  comparisonFunc, float minLOD, float maxLOD )
	: Diligent::SamplerDesc( minFilter, magFilter, mipFilter, addressU, addressV, addressW, mipLODBias, maxAnisotropy, comparisonFunc, minLOD, maxLOD )
{
}

SamplerRef createSampler( const Diligent::SamplerDesc &desc )
{
	SamplerRef sampler;
	getRenderDevice()->CreateSampler( desc, &sampler );
	return sampler;
}

}

namespace gx = graphics;
} // namespace cinder::graphics