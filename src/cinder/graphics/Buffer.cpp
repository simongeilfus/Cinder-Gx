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

#include "cinder/graphics/Buffer.h"
#include "cinder/app/RendererGx.h"

using namespace std;
using namespace ci::app;

namespace cinder { namespace graphics {

BufferDesc::BufferDesc()
	: Diligent::BufferDesc()
{
}

BufferDesc::BufferDesc( const BufferDesc &other )
	: mName( other.mName )
{
	uiSizeInBytes = other.uiSizeInBytes;
	BindFlags = other.BindFlags;
	Usage = other.Usage;
	CPUAccessFlags = other.CPUAccessFlags;
	Mode = other.Mode;
	ElementByteStride = other.ElementByteStride;
	CommandQueueMask = other.CommandQueueMask;

	updatePtrs();
}

BufferDesc::BufferDesc( BufferDesc &&other ) noexcept
	: BufferDesc()
{
	other.swap( *this );
	updatePtrs();
}

BufferDesc& BufferDesc::operator=( const BufferDesc &other )
{
	BufferDesc( other ).swap( *this );
	updatePtrs();
	return *this;
}

BufferDesc& BufferDesc::operator=( BufferDesc &&other ) noexcept
{
	other.swap( *this );
	updatePtrs();
	return *this;
}

void BufferDesc::swap( BufferDesc &other ) noexcept
{
	std::swap( uiSizeInBytes, other.uiSizeInBytes );
	std::swap( BindFlags, other.BindFlags );
	std::swap( Usage, other.Usage );
	std::swap( CPUAccessFlags, other.CPUAccessFlags );
	std::swap( Mode, other.Mode );
	std::swap( ElementByteStride, other.ElementByteStride );
	std::swap( CommandQueueMask, other.CommandQueueMask );

	std::swap( mName, other.mName );
}

void BufferDesc::updatePtrs() noexcept
{
	if( ! mName.empty() ) Name = mName.c_str();
}


BufferRef createBuffer( const Diligent::BufferDesc &buffDesc, const Diligent::BufferData* buffData )
{
	BufferRef buffer;
	getRenderDevice()->CreateBuffer( buffDesc, buffData, &buffer );
	return buffer;
}

//! Creates a new buffer object using the default RenderDevice
BufferRef createBuffer( const Diligent::BufferDesc &buffDesc, const void* data, uint32_t dataSize )
{
	BufferData bufferData{ data, dataSize };
	return createBuffer( buffDesc, &bufferData );
}

}

namespace gx = graphics;
} // namespace cinder::graphics