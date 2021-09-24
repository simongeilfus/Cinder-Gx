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

//! Buffer description
struct CI_API BufferDesc : public Diligent::BufferDesc {
    //! Size of the buffer, in bytes. For a uniform buffer, this must be multiple of 16.
    BufferDesc& size( uint32_t sizeInBytes ) { Size = sizeInBytes; return *this; }
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
    //! Defines which immediate contexts are allowed to execute commands that use this buffer.
    BufferDesc& immediateContextMask( uint64_t immediateContextMask ) { ImmediateContextMask = immediateContextMask; return *this; }
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

//! Buffer view description
struct CI_API BufferViewDesc : public Diligent::BufferViewDesc {
    //! View type. See Diligent::BUFFER_VIEW_TYPE for details.
    BufferViewDesc& viewType( BUFFER_VIEW_TYPE viewType ) { ViewType = viewType; return *this; }
    //! Format of the view. This member is only used for formatted and raw buffers. To create raw view of a raw buffer, set Format.ValueType member to VT_UNDEFINED (default value).
    BufferViewDesc& format( BufferFormat format ) { Format = format; return *this; }
    //! Type of components. For a formatted buffer views, this value cannot be VT_UNDEFINED.  This member is only used for formatted and raw buffers.
    BufferViewDesc& valueType( VALUE_TYPE valueType ) { Format.ValueType = valueType; return *this; }
    //! Number of components. Allowed values: 1, 2, 3, 4. For a formatted buffer, this value cannot be 0. This member is only used for formatted and raw buffers.
    BufferViewDesc& numComponents( Uint8 numComponents ) { Format.NumComponents = numComponents; return *this; }
    //! For signed and unsigned integer value types indicates if the value should be normalized to [-1,+1] or [0, 1] range respectively. For floating point types, this member is ignored. This member is only used for formatted and raw buffers.
    BufferViewDesc& isNormalized( Bool isNormalized ) { Format.IsNormalized = isNormalized; return *this; }
    //! Offset in bytes from the beginnig of the buffer to the start of the buffer region referenced by the view
    BufferViewDesc& byteOffset( uint32_t byteOffset ) { ByteOffset = byteOffset; return *this; }
    //! Size in bytes of the referenced buffer region
    BufferViewDesc& byteWidth( uint32_t byteWidth ) { ByteWidth = byteWidth; return *this; }
};

}

namespace gx = graphics;
} // namespace cinder::graphics