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
#include "DiligentCore/Graphics/GraphicsEngine/interface/Buffer.h"

namespace cinder { namespace graphics {

/// Buffer description
struct CI_API BufferDesc : public Diligent::BufferDesc {
    //! Size of the buffer, in bytes. For a uniform buffer, this must be multiple of 16.
    BufferDesc& sizeInBytes( uint32_t sizeInBytes ) { uiSizeInBytes = sizeInBytes; return *this; }
    //! Buffer bind flags, see Diligent::BIND_FLAGS for details
    BufferDesc& bindFlags( BIND_FLAGS bindFlags ) { BindFlags = bindFlags; return *this; }
    //! Buffer usage, see Diligent::USAGE for details
    BufferDesc& usage( USAGE usage ) { Usage = usage; return *this; }
    //! CPU access flags or 0 if no CPU access is allowed, see Diligent::CPU_ACCESS_FLAGS for details.
    BufferDesc& cpuAccessFlags( CPU_ACCESS_FLAGS cpuAccessFlags ) { CPUAccessFlags = cpuAccessFlags; return *this; }
    //! Buffer mode, see Diligent::BUFFER_MODE
    BufferDesc& mode( BUFFER_MODE mode ) { Mode = mode; return *this; }
    //! Buffer element stride, in bytes.
    BufferDesc& elementByteStride( uint32_t elementByteStride ) { ElementByteStride = elementByteStride; return *this; }
    //! Defines which command queues this buffer can be used with
    BufferDesc& commandQueueMask( uint64_t commandQueueMask ) { CommandQueueMask = commandQueueMask; return *this; }
    //! Specifies the object's name.
    BufferDesc& name( const char* name ) { Name = name; return *this; }

    BufferDesc();
    BufferDesc( const BufferDesc &other );
    BufferDesc( BufferDesc &&other ) noexcept;
    BufferDesc& operator=( const BufferDesc &other );
    BufferDesc& operator=( BufferDesc &&other ) noexcept;
    virtual ~BufferDesc() = default;
protected:
    void updatePtrs() noexcept;
    void swap( BufferDesc &other ) noexcept;

    std::string mName;
};

//! Creates a new buffer object using the default RenderDevice
CI_API BufferRef createBuffer( const Diligent::BufferDesc &buffDesc, const Diligent::BufferData* buffData = nullptr );
//! Creates a new buffer object using the default RenderDevice
CI_API BufferRef createBuffer( const Diligent::BufferDesc &buffDesc, const void* data, uint32_t dataSize );

}

namespace gx = graphics;
} // namespace cinder::graphics