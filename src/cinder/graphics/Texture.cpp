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

#include "DiligentCore/Graphics/GraphicsAccessories/interface/GraphicsAccessories.hpp"
#include "DiligentCore/Graphics/GraphicsTools/interface/GraphicsUtilities.h"

using namespace std;
using namespace ci::app;

namespace cinder {
namespace graphics {

TextureDesc::TextureDesc() 
	: Diligent::TextureDesc(),
	mSrgb( true ),
	mGenerateMips( true ),
	mDefaultUsage( true ),
	mDefaultMips( true )
{
}

TextureDesc::TextureDesc( const TextureDesc &other ) 
	: Diligent::TextureDesc( other ),
	mName( other.mName ), mSrgb( other.mSrgb ),
	mGenerateMips( other.mGenerateMips ), mDefaultUsage( other.mDefaultUsage ),
	mDefaultMips( other.mDefaultMips )
{
	updatePtrs();
}

TextureDesc::TextureDesc( const Diligent::TextureDesc &other )
	: Diligent::TextureDesc( other )
{
}

TextureDesc::TextureDesc( TextureDesc &&other ) noexcept
	: TextureDesc()
{
	other.swap( *this );
	updatePtrs();
}

TextureDesc& TextureDesc::operator=( const TextureDesc &other )
{
	TextureDesc( other ).swap( *this );
	updatePtrs();
	return *this;
}

TextureDesc& TextureDesc::operator=( TextureDesc &&other ) noexcept
{
	other.swap( *this );
	updatePtrs();
	return *this;
}

void TextureDesc::swap( TextureDesc &other ) noexcept
{
    std::swap( mName, other.mName );
	std::swap( mSrgb, other.mSrgb );
	std::swap( mGenerateMips, other.mGenerateMips );
	std::swap( mDefaultUsage, other.mDefaultUsage );
	std::swap( mDefaultMips, other.mDefaultMips );
	std::swap( Type, other.Type );
	std::swap( Width, other.Width );
	std::swap( Height, other.Height );
	std::swap( ArraySize, other.ArraySize );
	std::swap( Format, other.Format );
	std::swap( MipLevels, other.MipLevels );
	std::swap( SampleCount, other.SampleCount );
	std::swap( Usage, other.Usage );
	std::swap( BindFlags, other.BindFlags );
	std::swap( CPUAccessFlags, other.CPUAccessFlags );
	std::swap( MiscFlags, other.MiscFlags );
	std::swap( ClearValue, other.ClearValue );
	std::swap( CommandQueueMask, other.CommandQueueMask );
}

void TextureDesc::updatePtrs() noexcept
{
	if( ! mName.empty() ) Name = mName.c_str();
}

TextureRef createTexture( const TextureDesc &texDesc, const Diligent::TextureData* data )
{
	return createTexture( getRenderDevice(), texDesc, data );
}

TextureRef createTexture( RenderDevice* device, const TextureDesc &texDesc, const Diligent::TextureData* data )
{
	TextureRef texture;
	device->CreateTexture( texDesc, data, &texture );
	return texture;
}

namespace {
	template <typename ChannelType>
	void RGBToRGBA( const void* pRGBData, Uint32 RGBStride, void* pRGBAData, Uint32 RGBAStride, Uint32 Width, Uint32 Height )
	{
		for( size_t row = 0; row < size_t{ Height }; ++row )
			for( size_t col = 0; col < size_t{ Width }; ++col ) {
				for( int c = 0; c < 3; ++c ) {
					reinterpret_cast<ChannelType*>( ( reinterpret_cast<Uint8*>( pRGBAData ) + size_t{ RGBAStride } * row ) )[col * 4 + c] =
						reinterpret_cast<const ChannelType*>( ( reinterpret_cast<const Uint8*>( pRGBData ) + size_t{ RGBStride } * row ) )[col * 3 + c];
				}
				reinterpret_cast<ChannelType*>( ( reinterpret_cast<Uint8*>( pRGBAData ) + size_t{ RGBAStride } * row ) )[col * 4 + 3] = std::numeric_limits<ChannelType>::max();
			}
	}

	void createTexture( IRenderDevice* pDevice, const TextureDesc& desc, uint32_t sourceNumComponents, uint32_t channelDepth, const void* data, uint32_t rowStride, ITexture** ppTexture )
	{
		TextureDesc textureDesc;
		textureDesc.Name      = desc.Name;
		textureDesc.Type      = RESOURCE_DIM_TEX_2D;
		textureDesc.Width     = desc.Width;
		textureDesc.Height    = desc.Height;

		textureDesc.MipLevels = ComputeMipLevelsCount(textureDesc.Width, textureDesc.Height);
		if( desc.MipLevels > 0 )
			textureDesc.MipLevels = std::min( textureDesc.MipLevels, desc.MipLevels );

		textureDesc.Usage          = desc.Usage;
		textureDesc.BindFlags      = desc.BindFlags;
		textureDesc.Format         = desc.Format;
		textureDesc.CPUAccessFlags = desc.CPUAccessFlags;


		uint32_t numComponents = sourceNumComponents == 3 ? 4 : sourceNumComponents;
		bool   isSRGB          = (sourceNumComponents >= 3 && channelDepth == 8) ? desc.isSrgb() : false;

		if( textureDesc.Format == TEX_FORMAT_UNKNOWN ) {
			if( channelDepth == 8 ) {
				switch( numComponents ) {
					case 1: textureDesc.Format = TEX_FORMAT_R8_UNORM; break;
					case 2: textureDesc.Format = TEX_FORMAT_RG8_UNORM; break;
					case 4: textureDesc.Format = isSRGB ? TEX_FORMAT_RGBA8_UNORM_SRGB : TEX_FORMAT_RGBA8_UNORM; break;
					default: throw TextureDataExc( "Unexpected number of color channels (" + to_string( sourceNumComponents ) + ")");
				}
			}
			else if( channelDepth == 16 ) {
				switch( numComponents ) {
					case 1: textureDesc.Format = TEX_FORMAT_R16_UNORM; break;
					case 2: textureDesc.Format = TEX_FORMAT_RG16_UNORM; break;
					case 4: textureDesc.Format = TEX_FORMAT_RGBA16_UNORM; break;
					default: throw TextureDataExc( "Unexpected number of color channels (" + to_string( sourceNumComponents ) + ")" );
				}
			}
			else if( channelDepth == 32 ) {
				switch( numComponents ) {
					case 1: textureDesc.Format = TEX_FORMAT_R32_FLOAT; break;
					case 2: textureDesc.Format = TEX_FORMAT_RG32_FLOAT; break;
					case 4: textureDesc.Format = TEX_FORMAT_RGBA32_FLOAT; break;
					default: throw TextureDataExc( "Unexpected number of color channels (" + to_string( sourceNumComponents ) + ")" );
				}
			}
			else {
				throw TextureDataExc( "Unsupported color channel depth (" + to_string( channelDepth ) + ")" );
			}
		}
		else {
			const auto& TexFmtDesc = GetTextureFormatAttribs( textureDesc.Format );
			if( TexFmtDesc.NumComponents != numComponents )
				throw TextureDataExc( "Incorrect number of components (" + to_string( sourceNumComponents ) + ") for texture format" );
			if( TexFmtDesc.ComponentSize != channelDepth / 8 )
				throw TextureDataExc( "Incorrect channel size (" + to_string( channelDepth ) + ") for texture format" );
		}

		std::vector<TextureSubResData>  subResources(textureDesc.MipLevels);
		std::vector<std::vector<Uint8>> mips(textureDesc.MipLevels);

		if( sourceNumComponents == 3 ) {
			VERIFY_EXPR( numComponents == 4 );
			uint32_t rgbaStride = desc.Width * numComponents * channelDepth / 8;
			rgbaStride = ( rgbaStride + 3 ) & (-4);
			mips[0].resize( size_t{ rgbaStride } * size_t{ desc.Height } );
			subResources[0].pData = mips[0].data();
			subResources[0].Stride = rgbaStride;
			if( channelDepth == 8 ) {
				RGBToRGBA<Uint8>( data, rowStride, mips[0].data(), rgbaStride, desc.Width, desc.Height);
			}
			else if (channelDepth == 16) {
				RGBToRGBA<Uint16>( data, rowStride, mips[0].data(), rgbaStride, desc.Width, desc.Height );
			}
		}
		else {
			subResources[0].pData  = data;
			subResources[0].Stride = rowStride;
		}

		uint32_t mipWidth  = textureDesc.Width;
		uint32_t mipHeight = textureDesc.Height;
		for( uint32_t m = 1; m < textureDesc.MipLevels; ++m )
		{
			uint32_t coarseMipWidth  = std::max(mipWidth / 2u, 1u);
			uint32_t coarseMipHeight = std::max(mipHeight / 2u, 1u);
			uint32_t coarseMipStride = coarseMipWidth * numComponents * channelDepth / 8;
			coarseMipStride      = ( coarseMipStride + 3) & (-4);
			mips[m].resize(size_t{ coarseMipStride } * size_t{ coarseMipHeight });

			if( desc.needsGenerateMips() ) {
				ComputeMipLevel( mipWidth, mipHeight, textureDesc.Format, subResources[m - 1].pData, subResources[m - 1].Stride, mips[m].data(), coarseMipStride );
			}

			subResources[m].pData  = mips[m].data();
			subResources[m].Stride = coarseMipStride;

			mipWidth  = coarseMipWidth;
			mipHeight = coarseMipHeight;
		}

		TextureData TexData;
		TexData.pSubResources   = subResources.data();
		TexData.NumSubresources = textureDesc.MipLevels;

		pDevice->CreateTexture(textureDesc, &TexData, ppTexture);
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

		int64_t componentSize = 0;
		if( imageSource->getDataType() == ImageIo::DataType::UINT8 ) {
			componentSize = sizeof( uint8_t );
		}
		else if( imageSource->getDataType() == ImageIo::DataType::UINT16 ) {
			componentSize = sizeof( uint16_t );
		}
		else if( imageSource->getDataType() == ImageIo::DataType::FLOAT32 ) {
			componentSize = sizeof( float );
		}

		if( imageSource->getColorModel() == ImageIo::ColorModel::CM_GRAY ) {
			mRowInc = componentSize * static_cast<int64_t>( imageSource->getWidth() ) * ( imageSource->hasAlpha() ? 2 : 1 );
		}
		else {
			mRowInc = componentSize * static_cast<int64_t>( imageSource->getWidth() ) * ( imageSource->hasAlpha() ? 4 : 3 );
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

	TextureDesc getDefaultTextureDesc( TextureDesc desc, uint32_t width, uint32_t height )
	{
		desc.Width = width;
		desc.Height = height;
		desc.Usage = desc.isUsageDefault() ? USAGE_IMMUTABLE : desc.Usage;
		desc.BindFlags = desc.BindFlags == BIND_NONE ? BIND_SHADER_RESOURCE : desc.BindFlags;
		desc.MipLevels = desc.isMipsDefault() ? 0 : desc.MipLevels;
		return desc;
	}

	template<typename T>
	TextureRef createTextureFromSurface( RenderDevice* renderDevice, const SurfaceT<T> &surface, const TextureDesc &desc )
	{
		TextureRef texture;
		TextureDesc textureDesc = getDefaultTextureDesc( desc, surface.getWidth(), surface.getHeight() );
		createTexture( renderDevice, textureDesc, surface.hasAlpha() ? 4u : 3u, sizeof(T) * 8, surface.getData(), static_cast<uint32_t>( surface.getRowBytes() ), &texture );

		return texture;
	}

	template<typename T>
	TextureRef createTextureFromChannel( RenderDevice* renderDevice, const ChannelT<T> &channel, const TextureDesc &desc )
	{
		TextureRef texture;
		TextureDesc textureDesc = getDefaultTextureDesc( desc, channel.getWidth(), channel.getHeight() );
		createTexture( renderDevice, textureDesc, 1u, sizeof(T) * 8, channel.getData(), static_cast<uint32_t>( channel.getRowBytes() ), &texture );

		return texture;
	}

	TextureRef createTextureFromImageSource( RenderDevice* renderDevice, ImageSourceRef imageSource, const TextureDesc &desc )
	{
		shared_ptr<BasicImageTarget> imageTarget = make_shared<BasicImageTarget>( imageSource );
		imageSource->load( imageTarget );

		TextureRef texture;
		TextureDesc textureDesc = getDefaultTextureDesc( desc, imageSource->getWidth(), imageSource->getHeight() );
		createTexture( renderDevice, textureDesc, ImageIo::channelOrderNumChannels( imageSource->getChannelOrder() ), ImageIo::dataTypeBytes( imageSource->getDataType() ) * 8, imageTarget->getData(), (uint32_t) imageSource->getRowBytes(), &texture );

		return texture;
	}

} // anonymous namespace

using Diligent::TextureSubResData;

TextureRef createTexture( const Surface8u &surface, const TextureDesc &desc )
{
	return createTexture( app::getRenderDevice(), surface, desc );
}

TextureRef createTexture( const Channel8u &channel, const TextureDesc &desc )
{
	return createTexture( app::getRenderDevice(), channel, desc );
}

TextureRef createTexture( const Surface16u &surface, const TextureDesc &desc )
{
	return createTexture( app::getRenderDevice(), surface, desc );
}

TextureRef createTexture( const Channel16u &channel, const TextureDesc &desc )
{
	return createTexture( app::getRenderDevice(), channel, desc );
}

TextureRef createTexture( const Surface32f &surface, const TextureDesc &desc )
{
	return createTexture( app::getRenderDevice(), surface, desc );
}

TextureRef createTexture( const Channel32f &channel, const TextureDesc &desc )
{
	return createTexture( app::getRenderDevice(), channel, desc );
}

TextureRef createTexture( ImageSourceRef imageSource, const TextureDesc &desc )
{
	return createTexture( app::getRenderDevice(), imageSource, desc );
}

TextureRef createTextureFromKtx( const DataSourceRef &dataSource, const TextureDesc &desc )
{
	return createTextureFromKtx( app::getRenderDevice(), dataSource, desc );
}

TextureRef createTextureFromDds( const DataSourceRef &dataSource, const TextureDesc &desc )
{
	return createTextureFromDds( app::getRenderDevice(), dataSource, desc );
}

TextureRef createTexture( RenderDevice* device, const Surface8u &surface, const TextureDesc &desc )
{
	return createTextureFromSurface( device, surface, desc );
}

TextureRef createTexture( RenderDevice* device, const Channel8u &channel, const TextureDesc &desc )
{
	return createTextureFromChannel( device, channel, desc );
}

TextureRef createTexture( RenderDevice* device, const Surface16u &surface, const TextureDesc &desc )
{
	return createTextureFromSurface( device, surface, desc );
}

TextureRef createTexture( RenderDevice* device, const Channel16u &channel, const TextureDesc &desc )
{
	return createTextureFromChannel( device, channel, desc );
}

TextureRef createTexture( RenderDevice* device, const Surface32f &surface, const TextureDesc &desc )
{
	return createTextureFromSurface( device, surface, desc );
}

TextureRef createTexture( RenderDevice* device, const Channel32f &channel, const TextureDesc &desc )
{
	return createTextureFromChannel( device, channel, desc );
}

TextureRef createTexture( RenderDevice* device, ImageSourceRef imageSource, const TextureDesc &desc )
{
	return createTextureFromImageSource( device, imageSource, desc );
}

TextureRef createTextureFromKtx( RenderDevice* device, const DataSourceRef &dataSource, const TextureDesc &desc )
{
	TextureRef texture;
	return texture;
}

TextureRef createTextureFromDds( RenderDevice* device, const DataSourceRef &dataSource, const TextureDesc &desc )
{
	TextureRef texture;
	return texture;
}

}

namespace gx = graphics;
} // namespace cinder::graphics