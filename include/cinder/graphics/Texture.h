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

#include "cinder/Filesystem.h"
#include "cinder/DataSource.h"
#include "cinder/Surface.h"
#include "cinder/Filesystem.h"
#include "cinder/ImageIo.h"
#include "cinder/graphics/wrapper.h"

#include "DiligentCore/Graphics/GraphicsEngine/interface/GraphicsTypes.h"
#include "DiligentCore/Graphics/GraphicsEngine/interface/Texture.h"
#include "DiligentCore/Graphics/GraphicsEngine/interface/TextureView.h"

// TODO:
//
// [ ] Check issue with D3D11 Textures
// [ ] Mipmapping support
// [ ] Add isSRGB argument to create* functions

namespace cinder { namespace graphics {

//! Texture description
struct TextureDesc : public Diligent::TextureDesc {
    //! Texture type. See Diligent::RESOURCE_DIMENSION for details.
    TextureDesc& type( RESOURCE_DIMENSION type ) { Type = type; return *this; }
    //! Texture width and height, in pixels.
    TextureDesc& size( const ivec2 &size ) { Width = size.x; Height = size.y; return *this; }
    //! Texture width, in pixels.
    TextureDesc& width( uint32_t width ) { Width = width; return *this; }
    //! Texture height, in pixels.
    TextureDesc& height( uint32_t height ) { Height = height; return *this; }
    //! For a 1D array or 2D array, number of array slices
    TextureDesc& arraySize( uint32_t arraySize ) { ArraySize = arraySize; return *this; }
    //! For a 3D texture, number of depth slices
    TextureDesc& depth( uint32_t depth ) { Depth = depth; return *this; }
    //! Texture format, see Diligent::TEXTURE_FORMAT.
    TextureDesc& format( TEXTURE_FORMAT format ) { Format = format; return *this; }
    //! Number of Mip levels in the texture. Multisampled textures can only have 1 Mip level. Specify 0 to create full mipmap chain.
    TextureDesc& mipLevels( uint32_t mipLevels ) { MipLevels = mipLevels; return *this; }
    //! Number of samples. Only 2D textures or 2D texture arrays can be multisampled.
    TextureDesc& sampleCount( uint32_t sampleCount ) { SampleCount = sampleCount; return *this; }
    //! Texture usage. See Diligent::USAGE for details.
    TextureDesc& usage( USAGE usage ) { Usage = usage; return *this; }
    //! Bind flags, see Diligent::BIND_FLAGS for details.et
    TextureDesc& bindFlags( BIND_FLAGS bindFlags ) { BindFlags = bindFlags; return *this; }
    //! CPU access flags or 0 if no CPU access is allowed, see Diligent::CPU_ACCESS_FLAGS for details.
    TextureDesc& cpuAccessFlags( CPU_ACCESS_FLAGS cpuAccessFlags ) { CPUAccessFlags = cpuAccessFlags; return *this; }
    //! Miscellaneous flags, see Diligent::MISC_TEXTURE_FLAGS for details.
    TextureDesc& miscFlags( MISC_TEXTURE_FLAGS miscFlags ) { MiscFlags = miscFlags; return *this; }
    //! Optimized clear value
    TextureDesc& clearValue( OptimizedClearValue clearValue ) { ClearValue = clearValue; return *this; }
    //! Defines which command queues this texture can be used with
    TextureDesc& commandQueueMask( uint64_t commandQueueMask ) { CommandQueueMask = commandQueueMask; return *this; }
    //! Speficies the object's name.
    TextureDesc& name( const std::string &name ) { mName = name; Name = mName.c_str(); return *this; }

    TextureDesc();
    TextureDesc( const TextureDesc &other );
    TextureDesc( TextureDesc &&other ) noexcept;
    TextureDesc& operator=( const TextureDesc &other );
    TextureDesc& operator=( TextureDesc &&other ) noexcept;
    virtual ~TextureDesc() = default;
protected:
    void updatePtrs() noexcept;
    void swap( TextureDesc &other ) noexcept;

    std::string mName;
};

//! Constructs a Texture based on the contents of \a data.
CI_API TextureRef createTexture( const Diligent::TextureDesc &desc, const TextureData* data = nullptr );
//! Constructs a Texture based on the contents of \a surface using the default RenderDevice.
CI_API TextureRef createTexture( const Surface8u &surface, const Diligent::TextureDesc &desc = Diligent::TextureDesc() );
//! Constructs a Texture based on the contents of \a channel using the default RenderDevice.
CI_API TextureRef createTexture( const Channel8u &channel, const Diligent::TextureDesc &desc = Diligent::TextureDesc() );
//! Constructs a Texture based on the contents of \a surface using the default RenderDevice.
CI_API TextureRef createTexture( const Surface16u &surface, const Diligent::TextureDesc &desc = Diligent::TextureDesc() );
//! Constructs a Texture based on the contents of \a channel using the default RenderDevice.
CI_API TextureRef createTexture( const Channel16u &channel, const Diligent::TextureDesc &desc = Diligent::TextureDesc() );
//! Constructs a Texture based on the contents of \a surface using the default RenderDevice.
CI_API TextureRef createTexture( const Surface32f &surface, const Diligent::TextureDesc &desc = Diligent::TextureDesc() );
//! brief Constructs a texture based on the contents of \a channel using the default RenderDevice.
CI_API TextureRef createTexture( const Channel32f &channel, const Diligent::TextureDesc &desc = Diligent::TextureDesc() );
//! Constructs a Texture based on \a imageSource using the default RenderDevice.
CI_API TextureRef createTexture( ImageSourceRef imageSource, const Diligent::TextureDesc &desc = Diligent::TextureDesc() );
//! Constructs a Texture from an optionally compressed KTX file using the default RenderDevice.
CI_API TextureRef createTextureFromKtx( const DataSourceRef &dataSource, const Diligent::TextureDesc &desc = Diligent::TextureDesc() );
//! Constructs a Texture from a DDS file using the default RenderDevice. Supports DXT1, DTX3, and DTX5. Supports BC7 in the presence of \c GL_ARB_texture_compression_bptc.
CI_API TextureRef createTextureFromDds( const DataSourceRef &dataSource, const Diligent::TextureDesc &desc = Diligent::TextureDesc() );

//! Constructs a Texture based on the contents of \a surface.
CI_API TextureRef createTexture( RenderDevice* device, const Surface8u &surface, const Diligent::TextureDesc &desc = Diligent::TextureDesc() );
//! Constructs a Texture based on the contents of \a channel.
CI_API TextureRef createTexture( RenderDevice* device, const Channel8u &channel, const Diligent::TextureDesc &desc = Diligent::TextureDesc() );
//! Constructs a Texture based on the contents of \a surface.
CI_API TextureRef createTexture( RenderDevice* device, const Surface16u &surface, const Diligent::TextureDesc &desc = Diligent::TextureDesc() );
//! Constructs a Texture based on the contents of \a channel.
CI_API TextureRef createTexture( RenderDevice* device, const Channel16u &channel, const Diligent::TextureDesc &desc = Diligent::TextureDesc() );
//! Constructs a Texture based on the contents of \a surface.
CI_API TextureRef createTexture( RenderDevice* device, const Surface32f &surface, const Diligent::TextureDesc &desc = Diligent::TextureDesc() );
//! brief Constructs a texture based on the contents of \a channel.
CI_API TextureRef createTexture( RenderDevice* device, const Channel32f &channel, const Diligent::TextureDesc &desc = Diligent::TextureDesc() );
//! Constructs a Texture based on \a imageSource.
CI_API TextureRef createTexture( RenderDevice* device, ImageSourceRef imageSource, const Diligent::TextureDesc &desc = Diligent::TextureDesc() );
//! Constructs a Texture from an optionally compressed KTX file.
CI_API TextureRef createTextureFromKtx( RenderDevice* device, const DataSourceRef &dataSource, const Diligent::TextureDesc &desc = Diligent::TextureDesc() );
//! Constructs a Texture from a DDS file. Supports DXT1, DTX3, and DTX5. Supports BC7 in the presence of \c GL_ARB_texture_compression_bptc.
CI_API TextureRef createTextureFromDds( RenderDevice* device, const DataSourceRef &dataSource, const Diligent::TextureDesc &desc = Diligent::TextureDesc() );
}

namespace gx = graphics;
} // namespace cinder::graphics