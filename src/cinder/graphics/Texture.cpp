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

#include "cinder/graphics/Texture.h"
#include "cinder/app/RendererGx.h"

using namespace std;
using namespace ci::app;

namespace cinder {
namespace graphics {

TextureRef createTexture( const Diligent::TextureDesc &texDesc, const Diligent::TextureData* data )
{
	TextureRef texture;
	getRenderDevice()->CreateTexture( texDesc, data, &texture );
	return texture;
}

namespace {
	template<typename T>
	TEXTURE_FORMAT getTextureFormatFromSurface( const SurfaceT<T> &surface )
	{
		TEXTURE_FORMAT format = Diligent::TEX_FORMAT_UNKNOWN;
		SurfaceChannelOrder sco = surface.getChannelOrder();

		bool isSRGB = true;
		uint8_t numComponents = 4;// surface.hasAlpha()
		if( std::is_same<T,uint8_t>::value ) {
			switch( numComponents ) {
			case 1: format = Diligent::TEX_FORMAT_R8_UNORM; break;
			case 2: format = Diligent::TEX_FORMAT_RG8_UNORM; break;
			case 4: format = isSRGB ? Diligent::TEX_FORMAT_RGBA8_UNORM_SRGB : Diligent::TEX_FORMAT_RGBA8_UNORM; break;
			//default: LOG_ERROR_AND_THROW( "Unexpected number of color channels (", ImgDesc.NumComponents, ")" );
			}
		}
		else if( std::is_same<T,uint16_t>::value ) { // 16-bit
			switch( numComponents ) {
			case 1: format = Diligent::TEX_FORMAT_R16_UNORM; break;
			case 2: format = Diligent::TEX_FORMAT_RG16_UNORM; break;
			case 4: format = Diligent::TEX_FORMAT_RGBA16_UNORM; break;
			//default: LOG_ERROR_AND_THROW( "Unexpected number of color channels (", ImgDesc.NumComponents, ")" );
			}
		}
		//else if( std::is_same<T, float>::value ) { // 32-bit float
		//	switch( numComponents ) {
		//	case 1: format = Diligent::TEX_FORMAT_R32_UNORM; break;
		//	case 2: format = Diligent::TEX_FORMAT_RG32_UNORM; break;
		//	case 4: format = Diligent::TEX_FORMAT_RGBA32_UNORM; break;
		//	//default: LOG_ERROR_AND_THROW( "Unexpected number of color channels (", ImgDesc.NumComponents, ")" );
		//	}
		//}
		//else
		//	LOG_ERROR_AND_THROW( "Unsupported color channel depth (", ChannelDepth, ")" );

		return format;
	}

	template<typename T>
	Diligent::TextureDesc getTextureDescFromSurface( const SurfaceT<T> &surface, const Diligent::TextureDesc &desc )
	{
		Diligent::TextureDesc textureDesc = desc;
		textureDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
		textureDesc.Width = surface.getWidth();
		textureDesc.Height = surface.getHeight();
		textureDesc.Depth = 1;
		textureDesc.Format = desc.Format != Diligent::TEX_FORMAT_UNKNOWN ? desc.Format : getTextureFormatFromSurface( surface );
		textureDesc.MipLevels = 1;
		textureDesc.SampleCount = 1;
		textureDesc.Usage = desc.Usage != Diligent::USAGE_DEFAULT ? desc.Usage : Diligent::USAGE_IMMUTABLE;
		textureDesc.BindFlags = desc.BindFlags != Diligent::BIND_NONE ? desc.BindFlags : Diligent::BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = desc.CPUAccessFlags != Diligent::CPU_ACCESS_NONE ? desc.CPUAccessFlags : Diligent::CPU_ACCESS_READ;

		return textureDesc;
	}

	template<typename T>
	TextureRef createTextureFromSurface( RenderDevice* renderDevice, const SurfaceT<T> &surface, const Diligent::TextureDesc &desc )
	{
		TextureSubResData subData[] = { { surface.getData(), (uint32_t) surface.getRowBytes() } };
		TextureData data = { subData, 1 };
		TextureRef texture;
		renderDevice->CreateTexture( getTextureDescFromSurface( surface, desc ), &data, &texture );
		return texture;
	}

	TEXTURE_FORMAT getTextureFormatFromImageSource( const ImageSourceRef &imageSource )
	{
		TEXTURE_FORMAT format = Diligent::TEX_FORMAT_UNKNOWN;
		uint8_t numComponents = 4;
		switch( imageSource->getChannelOrder() ) {
			case ImageIo::ChannelOrder::RGBA: numComponents = 4; break;
			case ImageIo::ChannelOrder::BGRA: numComponents = 4; break;
			case ImageIo::ChannelOrder::ARGB: numComponents = 4; break;
			case ImageIo::ChannelOrder::ABGR: numComponents = 4; break;
			case ImageIo::ChannelOrder::RGBX: numComponents = 4; break;
			case ImageIo::ChannelOrder::BGRX: numComponents = 4; break;
			case ImageIo::ChannelOrder::XRGB: numComponents = 4; break;
			case ImageIo::ChannelOrder::XBGR: numComponents = 4; break;
			case ImageIo::ChannelOrder::RGB: numComponents = 4; break;
			case ImageIo::ChannelOrder::BGR: numComponents = 4; break;
			case ImageIo::ChannelOrder::Y: numComponents = 1; break;
			case ImageIo::ChannelOrder::YA: numComponents = 2; break;
			default: numComponents = 4; break;
		}
		bool isSRGB = true;
		if( imageSource->getDataType() == ImageIo::DataType::UINT8 ) {
			switch( numComponents ) {
			case 1: format = Diligent::TEX_FORMAT_R8_UNORM; break;
			case 2: format = Diligent::TEX_FORMAT_RG8_UNORM; break;
			case 4: format = isSRGB ? Diligent::TEX_FORMAT_RGBA8_UNORM_SRGB : Diligent::TEX_FORMAT_RGBA8_UNORM; break;
			//default: LOG_ERROR_AND_THROW( "Unexpected number of color channels (", ImgDesc.NumComponents, ")" );
			}
		}
		else if( imageSource->getDataType() == ImageIo::DataType::UINT16 ) {
			switch( numComponents ) {
			case 1: format = Diligent::TEX_FORMAT_R16_UNORM; break;
			case 2: format = Diligent::TEX_FORMAT_RG16_UNORM; break;
			case 4: format = Diligent::TEX_FORMAT_RGBA16_UNORM; break;
			//default: LOG_ERROR_AND_THROW( "Unexpected number of color channels (", ImgDesc.NumComponents, ")" );
			}
		}
		//else if( std::is_same<T, float>::value ) { // 32-bit float
		//	switch( numComponents ) {
		//	case 1: format = Diligent::TEX_FORMAT_R32_UNORM; break;
		//	case 2: format = Diligent::TEX_FORMAT_RG32_UNORM; break;
		//	case 4: format = Diligent::TEX_FORMAT_RGBA32_UNORM; break;
		//	//default: LOG_ERROR_AND_THROW( "Unexpected number of color channels (", ImgDesc.NumComponents, ")" );
		//	}
		//}
		//else
		//	LOG_ERROR_AND_THROW( "Unsupported color channel depth (", ChannelDepth, ")" );

		return format;
	}

	Diligent::TextureDesc getTextureDescFromImageSource( const ImageSourceRef &imageSource, const Diligent::TextureDesc &desc )
	{
		Diligent::TextureDesc textureDesc = desc;
		textureDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
		textureDesc.Width = imageSource->getWidth();
		textureDesc.Height = imageSource->getHeight();
		textureDesc.Depth = 1;
		textureDesc.Format = desc.Format != Diligent::TEX_FORMAT_UNKNOWN ? desc.Format : getTextureFormatFromImageSource( imageSource );
		textureDesc.MipLevels = 1;
		textureDesc.SampleCount = 1;
		textureDesc.Usage = desc.Usage != Diligent::USAGE_DEFAULT ? desc.Usage : Diligent::USAGE_IMMUTABLE;
		textureDesc.BindFlags = desc.BindFlags != Diligent::BIND_NONE ? desc.BindFlags : Diligent::BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = desc.CPUAccessFlags != Diligent::CPU_ACCESS_NONE ? desc.CPUAccessFlags : Diligent::CPU_ACCESS_READ;

		return textureDesc;
	}

	class BasicImageTarget : public ImageTarget {
	public:
		BasicImageTarget( const ImageSourceRef &imageSource );
		~BasicImageTarget();
	
		void* getRowPointer( int32_t row ) override { return static_cast<void*>( static_cast<char *>( mData ) + static_cast<int64_t>( row ) * mRowInc ); }
		void* getData() const { return mData; }

	private:
		void*	mData;
		int64_t	mRowInc;
	};

	BasicImageTarget::BasicImageTarget( const ImageSourceRef &imageSource ) 
	{
		mData = malloc( imageSource->getRowBytes() * imageSource->getHeight() );

		if( imageSource->getColorModel() == ImageIo::ColorModel::CM_GRAY ) {
			mRowInc = imageSource->getWidth() * ( imageSource->hasAlpha() ? 2 : 1 );
		}
		else {
			mRowInc = imageSource->getWidth() * ( imageSource->hasAlpha() ? 4 : 3 );
		}
		setSize( imageSource->getWidth(), imageSource->getHeight() );
		setDataType( imageSource->getDataType() );

		ImageIo::ChannelOrder channelOrder;
		switch( imageSource->getColorModel() ) {
		case ImageSource::CM_RGB:
		default:
			channelOrder = ( imageSource->hasAlpha() ) ? ImageIo::RGBA : ImageIo::RGB;
			break;
		case ImageSource::CM_GRAY:
			channelOrder = ( imageSource->hasAlpha() ) ? ImageIo::YA : ImageIo::Y;
			break;
		}

		setChannelOrder( channelOrder );
		setColorModel( imageSource->getColorModel() );
	}

	BasicImageTarget::~BasicImageTarget()
	{
		free( mData );
	}

} // anonymous namespace

using Diligent::TextureSubResData;

TextureRef createTexture( const Surface8u &surface, const Diligent::TextureDesc &desc )
{
	return createTexture( app::getRenderDevice(), surface, desc );
}

TextureRef createTexture( const Channel8u &channel, const Diligent::TextureDesc &desc )
{
	return createTexture( app::getRenderDevice(), channel, desc );
}

TextureRef createTexture( const Surface16u &surface, const Diligent::TextureDesc &desc )
{
	return createTexture( app::getRenderDevice(), surface, desc );
}

TextureRef createTexture( const Channel16u &channel, const Diligent::TextureDesc &desc )
{
	return createTexture( app::getRenderDevice(), channel, desc );
}

TextureRef createTexture( const Surface32f &surface, const Diligent::TextureDesc &desc )
{
	return createTexture( app::getRenderDevice(), surface, desc );
}

TextureRef createTexture( const Channel32f &channel, const Diligent::TextureDesc &desc )
{
	return createTexture( app::getRenderDevice(), channel, desc );
}

TextureRef createTexture( ImageSourceRef imageSource, const Diligent::TextureDesc &desc )
{
	return createTexture( app::getRenderDevice(), imageSource, desc );
}

TextureRef createTextureFromKtx( const DataSourceRef &dataSource, const Diligent::TextureDesc &desc )
{
	return createTextureFromKtx( app::getRenderDevice(), dataSource, desc );
}

TextureRef createTextureFromDds( const DataSourceRef &dataSource, const Diligent::TextureDesc &desc )
{
	return createTextureFromDds( app::getRenderDevice(), dataSource, desc );
}

TextureRef createTexture( RenderDevice* device, const Surface8u &surface, const Diligent::TextureDesc &desc )
{
	return createTextureFromSurface( device, surface, desc );
}

TextureRef createTexture( RenderDevice* device, const Channel8u &channel, const Diligent::TextureDesc &desc )
{
	TextureRef texture;
	return texture;
}

TextureRef createTexture( RenderDevice* device, const Surface16u &surface, const Diligent::TextureDesc &desc )
{
	return createTextureFromSurface( device, surface, desc );
}

TextureRef createTexture( RenderDevice* device, const Channel16u &channel, const Diligent::TextureDesc &desc )
{
	TextureRef texture;
	return texture;
}

TextureRef createTexture( RenderDevice* device, const Surface32f &surface, const Diligent::TextureDesc &desc )
{
	return createTextureFromSurface( device, surface, desc );
}

TextureRef createTexture( RenderDevice* device, const Channel32f &channel, const Diligent::TextureDesc &desc )
{
	TextureRef texture;
	return texture;
}

TextureRef createTexture( RenderDevice* device, ImageSourceRef imageSource, const Diligent::TextureDesc &desc )
{
	shared_ptr<BasicImageTarget> imageTarget = make_shared<BasicImageTarget>( imageSource );
	imageSource->load( imageTarget );
	TextureSubResData subData[] = { { imageTarget->getData(), (uint32_t) imageSource->getRowBytes() } };
	TextureData data = { subData, 1 };
	TextureRef texture;
	device->CreateTexture( getTextureDescFromImageSource( imageSource, desc ), &data, &texture );
	return texture;
}

TextureRef createTextureFromKtx( RenderDevice* device, const DataSourceRef &dataSource, const Diligent::TextureDesc &desc )
{
	TextureRef texture;
	return texture;
}

TextureRef createTextureFromDds( RenderDevice* device, const DataSourceRef &dataSource, const Diligent::TextureDesc &desc )
{
	TextureRef texture;
	return texture;
}

}

namespace gx = graphics;
} // namespace cinder::graphics