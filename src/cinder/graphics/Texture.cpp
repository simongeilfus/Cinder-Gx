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
#include "DiligentCore/Common/interface/Align.hpp"

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
			else if( channelDepth == 32 ) {
				RGBToRGBA<float>( data, rowStride, mips[0].data(), rgbaStride, desc.Width, desc.Height );
			}
		}
		else {
			subResources[0].pData  = data;
			subResources[0].Stride = rowStride;
		}

		uint32_t mipWidth  = textureDesc.Width;
		uint32_t mipHeight = textureDesc.Height;
		for( uint32_t mip = 1; mip < textureDesc.MipLevels; ++mip ) {
			uint32_t coarseMipWidth  = std::max(mipWidth / 2u, 1u);
			uint32_t coarseMipHeight = std::max(mipHeight / 2u, 1u);
			uint32_t coarseMipStride = coarseMipWidth * numComponents * channelDepth / 8;
			coarseMipStride      = ( coarseMipStride + 3) & (-4);
			mips[mip].resize(size_t{ coarseMipStride } * size_t{ coarseMipHeight });

			if( desc.needsGenerateMips() ) {
				ComputeMipLevel( mipWidth, mipHeight, textureDesc.Format, subResources[mip - 1].pData, subResources[mip - 1].Stride, mips[mip].data(), coarseMipStride );
			}

			subResources[mip].pData  = mips[mip].data();
			subResources[mip].Stride = coarseMipStride;

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

TextureRef createTextureCubeMap( const ImageSourceRef &imageSource, const TextureDesc &desc )
{
	return createTextureCubeMap( app::getRenderDevice(), imageSource, desc );
}

TextureRef createTextureCubeMap( const ImageSourceRef images[6], const TextureDesc &desc )
{
	return createTextureCubeMap( app::getRenderDevice(), images, desc );
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

namespace {
	#define GL_RGBA32F            0x8814
	#define GL_RGBA32UI           0x8D70
	#define GL_RGBA32I            0x8D82
	#define GL_RGB32F             0x8815
	#define GL_RGB32UI            0x8D71
	#define GL_RGB32I             0x8D83
	#define GL_RGBA16F            0x881A
	#define GL_RGBA16             0x805B
	#define GL_RGBA16UI           0x8D76
	#define GL_RGBA16_SNORM       0x8F9B
	#define GL_RGBA16I            0x8D88
	#define GL_RG32F              0x8230
	#define GL_RG32UI             0x823C
	#define GL_RG32I              0x823B
	#define GL_DEPTH32F_STENCIL8  0x8CAD
	#define GL_RGB10_A2           0x8059
	#define GL_RGB10_A2UI         0x906F
	#define GL_R11F_G11F_B10F     0x8C3A
	#define GL_RGBA8              0x8058
	#define GL_RGBA8UI            0x8D7C
	#define GL_RGBA8_SNORM        0x8F97
	#define GL_RGBA8I             0x8D8E
	#define GL_RG16F              0x822F
	#define GL_RG16               0x822C
	#define GL_RG16UI             0x823A
	#define GL_RG16_SNORM         0x8F99
	#define GL_RG16I              0x8239
	#define GL_R32F               0x822E
	#define GL_DEPTH_COMPONENT32F 0x8CAC
	#define GL_R32UI              0x8236
	#define GL_R32I               0x8235
	#define GL_DEPTH24_STENCIL8   0x88F0
	#define GL_RG8                0x822B
	#define GL_RG8UI              0x8238
	#define GL_RG8_SNORM          0x8F95
	#define GL_RG8I               0x8237
	#define GL_R16F               0x822D
	#define GL_DEPTH_COMPONENT16  0x81A5
	#define GL_R16                0x822A
	#define GL_R16UI              0x8234
	#define GL_R16_SNORM          0x8F98
	#define GL_R16I               0x8233
	#define GL_R8                 0x8229
	#define GL_R8UI               0x8232
	#define GL_R8_SNORM           0x8F94
	#define GL_R8I                0x8231
	#define GL_RGB9_E5            0x8C3D


	#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT        0x83F0
	#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT       0x83F1
	#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT       0x83F2
	#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT       0x83F3
	#define GL_COMPRESSED_SRGB_S3TC_DXT1_EXT       0x8C4C
	#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT 0x8C4D
	#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT 0x8C4E
	#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT 0x8C4F
	#define GL_COMPRESSED_RED_RGTC1                0x8DBB
	#define GL_COMPRESSED_SIGNED_RED_RGTC1         0x8DBC
	#define GL_COMPRESSED_RG_RGTC2                 0x8DBD
	#define GL_COMPRESSED_SIGNED_RG_RGTC2          0x8DBE
	#define GL_COMPRESSED_RGBA_BPTC_UNORM          0x8E8C
	#define GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM    0x8E8D
	#define GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT    0x8E8E
	#define GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT  0x8E8F

	TEXTURE_FORMAT FindDiligentTextureFormat(std::uint32_t GLInternalFormat)
	{
		switch (GLInternalFormat)
		{
			// clang-format off
			case GL_RGBA32F:        return TEX_FORMAT_RGBA32_FLOAT;
			case GL_RGBA32UI:       return TEX_FORMAT_RGBA32_UINT;
			case GL_RGBA32I:        return TEX_FORMAT_RGBA32_SINT;

			case GL_RGB32F:         return TEX_FORMAT_RGB32_FLOAT;
			case GL_RGB32UI:        return TEX_FORMAT_RGB32_UINT;
			case GL_RGB32I:         return TEX_FORMAT_RGB32_SINT;
        
			case GL_RGBA16F:        return TEX_FORMAT_RGBA16_FLOAT;
			case GL_RGBA16:         return TEX_FORMAT_RGBA16_UNORM;
			case GL_RGBA16UI:       return TEX_FORMAT_RGBA16_UINT;
			case GL_RGBA16_SNORM:   return TEX_FORMAT_RGBA16_SNORM;
			case GL_RGBA16I:        return TEX_FORMAT_RGBA16_SINT;
        
			case GL_RG32F:          return TEX_FORMAT_RG32_FLOAT;
			case GL_RG32UI:         return TEX_FORMAT_RG32_UINT;
			case GL_RG32I:          return TEX_FORMAT_RG32_SINT;

			case GL_DEPTH32F_STENCIL8: return TEX_FORMAT_D32_FLOAT_S8X24_UINT;

			case GL_RGB10_A2:       return TEX_FORMAT_RGB10A2_UNORM;
			case GL_RGB10_A2UI:     return TEX_FORMAT_RGB10A2_UINT;
			case GL_R11F_G11F_B10F: return TEX_FORMAT_R11G11B10_FLOAT;
    
			case GL_RGBA8:          return TEX_FORMAT_RGBA8_UNORM;
			case GL_RGBA8UI:        return TEX_FORMAT_RGBA8_UINT;
			case GL_RGBA8_SNORM:    return TEX_FORMAT_RGBA8_SNORM;
			case GL_RGBA8I:         return TEX_FORMAT_RGBA8_SINT;
        
			case GL_RG16F:          return TEX_FORMAT_RG16_FLOAT;
			case GL_RG16:           return TEX_FORMAT_RG16_UNORM;
			case GL_RG16UI:         return TEX_FORMAT_RG16_UINT;
			case GL_RG16_SNORM:     return TEX_FORMAT_RG16_SNORM;
			case GL_RG16I:          return TEX_FORMAT_RG16_SINT;
        
			case GL_R32F:               return TEX_FORMAT_R32_FLOAT;
			case GL_DEPTH_COMPONENT32F: return TEX_FORMAT_D32_FLOAT;
			case GL_R32UI:              return TEX_FORMAT_R32_UINT;
			case GL_R32I:               return TEX_FORMAT_R32_SINT;
        
			case GL_DEPTH24_STENCIL8: return TEX_FORMAT_D24_UNORM_S8_UINT;

			case GL_RG8:        return TEX_FORMAT_RG8_UNORM;
			case GL_RG8UI:      return TEX_FORMAT_RG8_UINT;
			case GL_RG8_SNORM:  return TEX_FORMAT_RG8_SNORM;
			case GL_RG8I:       return TEX_FORMAT_RG8_SINT;

			case GL_R16F:               return TEX_FORMAT_R16_FLOAT;
			case GL_DEPTH_COMPONENT16:  return TEX_FORMAT_D16_UNORM;
			case GL_R16:                return TEX_FORMAT_R16_UNORM;
			case GL_R16UI:              return TEX_FORMAT_R16_UINT;
			case GL_R16_SNORM:          return TEX_FORMAT_R16_SNORM;
			case GL_R16I:               return TEX_FORMAT_R16_SINT;
        
			case GL_R8:                 return TEX_FORMAT_R8_UNORM;
			case GL_R8UI:               return TEX_FORMAT_R8_UINT;
			case GL_R8_SNORM:           return TEX_FORMAT_R8_SNORM;
			case GL_R8I:                return TEX_FORMAT_R8_SINT;

			case GL_RGB9_E5:            return TEX_FORMAT_RGB9E5_SHAREDEXP;

			case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:        return TEX_FORMAT_BC1_UNORM;
			case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:       return TEX_FORMAT_BC1_UNORM_SRGB;
			case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:       return TEX_FORMAT_BC2_UNORM;
			case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT: return TEX_FORMAT_BC2_UNORM_SRGB;
			case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:       return TEX_FORMAT_BC3_UNORM;
			case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT: return TEX_FORMAT_BC3_UNORM_SRGB;
			case GL_COMPRESSED_RED_RGTC1:                return TEX_FORMAT_BC4_UNORM;
			case GL_COMPRESSED_SIGNED_RED_RGTC1:         return TEX_FORMAT_BC4_SNORM;
			case GL_COMPRESSED_RG_RGTC2:                 return TEX_FORMAT_BC5_UNORM;
			case GL_COMPRESSED_SIGNED_RG_RGTC2:          return TEX_FORMAT_BC5_SNORM;

			case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT: return TEX_FORMAT_BC6H_UF16;
			case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT:   return TEX_FORMAT_BC6H_SF16;
			case GL_COMPRESSED_RGBA_BPTC_UNORM:         return TEX_FORMAT_BC7_UNORM;
			case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM:   return TEX_FORMAT_BC7_UNORM_SRGB;
			// clang-format on
			default:
				UNSUPPORTED("Unsupported internal format");
				return TEX_FORMAT_UNKNOWN;
		}
	}
} // namespace


TextureRef createTextureFromKtx( RenderDevice* device, const DataSourceRef &dataSource, const TextureDesc &desc )
{
	typedef struct {
		uint8_t		identifier[12];
		uint32_t	endianness;
		uint32_t	glType;
		uint32_t	glTypeSize;
		uint32_t	glFormat;
		uint32_t	glInternalFormat;
		uint32_t	glBaseInternalFormat;
		uint32_t	pixelWidth;
		uint32_t	pixelHeight;
		uint32_t	pixelDepth;
		uint32_t	numberOfArrayElements;
		uint32_t	numberOfFaces;
		uint32_t	numberOfMipmapLevels;
		uint32_t	bytesOfKeyValueData;
	} KtxHeader;

	static const uint8_t FileIdentifier[12] = {
		0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A
	};
	
	ci::BufferRef dataBuffer = dataSource->getBuffer();
	size_t dataSize = dataBuffer->getSize();
	const uint8_t* data = reinterpret_cast<const uint8_t*>( dataBuffer->getData() );
	
	if( dataSize < sizeof(FileIdentifier) || memcmp( data, FileIdentifier, sizeof(FileIdentifier) ) )
		throw TextureDataExc( "File identifier mismatch" );			

	const KtxHeader &header = *reinterpret_cast<KtxHeader const*>( data );
	if( header.endianness != 0x04030201 )
		throw TextureDataExc( "Only little endian currently supported" );

	if( ( header.numberOfFaces != 1 ) && ( header.numberOfFaces != 6 ) )
		throw TextureDataExc( "Unsupported number of faces" );
			
	TextureDesc textureDesc = getDefaultTextureDesc( desc, header.pixelWidth, header.pixelHeight );
	textureDesc.Name      = desc.Name;
	textureDesc.Type      = RESOURCE_DIM_TEX_2D;
	textureDesc.Width     = header.pixelWidth;
	textureDesc.Height    = header.pixelHeight;
	textureDesc.Depth	  = header.pixelDepth;
	textureDesc.MipLevels = std::max( header.numberOfMipmapLevels, desc.MipLevels );
	textureDesc.Format    = FindDiligentTextureFormat( header.glInternalFormat );
	if( textureDesc.Format == TEX_FORMAT_UNKNOWN ) {
		throw TextureDataExc( "Failed to find appropriate Diligent format for internal gl format " + to_string( header.glInternalFormat ) );
	}

	uint32_t numFaces = std::max( header.numberOfFaces, 1u );
	if( numFaces == 6 ) {
		textureDesc.ArraySize = std::max( header.numberOfArrayElements, 1u ) * numFaces;
		textureDesc.Type      = textureDesc.ArraySize > 6 ? RESOURCE_DIM_TEX_CUBE_ARRAY : RESOURCE_DIM_TEX_CUBE;
	}
	else {
		if( textureDesc.Depth > 1 ) {
			textureDesc.ArraySize = 1;
			textureDesc.Type      = RESOURCE_DIM_TEX_3D;
		}
		else {
			textureDesc.ArraySize = std::max( header.numberOfArrayElements, 1u );
			textureDesc.Type      = textureDesc.ArraySize > 1 ? RESOURCE_DIM_TEX_2D_ARRAY : RESOURCE_DIM_TEX_2D;
		}
	}

	data += sizeof( KtxHeader );
	data += header.bytesOfKeyValueData;

	uint32_t arraySize = ( textureDesc.Type != RESOURCE_DIM_TEX_3D ? textureDesc.ArraySize : 1 );
	std::vector<TextureSubResData> subresData( textureDesc.MipLevels * arraySize );
	for( uint32_t mip = 0; mip < textureDesc.MipLevels; ++mip ) {
		data += sizeof(uint32_t);
		MipLevelProperties mipInfo = GetMipLevelProperties( textureDesc, mip );
		for( uint32_t layer = 0; layer < arraySize; ++layer ) {
			subresData[mip + layer * textureDesc.MipLevels] = TextureSubResData{ data, mipInfo.RowSize, mipInfo.DepthSliceSize };
			data += AlignUp( mipInfo.MipSize, 4u );
		}
	}

	TextureRef texture;
	TextureData textureData( subresData.data(), static_cast<uint32_t>( subresData.size() ) );
	device->CreateTexture( textureDesc, &textureData, &texture );

	return texture;
}

TextureRef createTextureFromDds( RenderDevice* device, const DataSourceRef &dataSource, const TextureDesc &desc )
{
	TextureRef texture;
	return texture;
}

namespace {
	struct CubeMapFaceRegion {
		Area		mArea;
		ivec2		mOffset;
		bool		mFlip; // horizontal + vertical
	};

	std::vector<CubeMapFaceRegion> calcCubeMapHorizontalCrossRegions( const ImageSourceRef &imageSource )
	{
		std::vector<CubeMapFaceRegion> result;

		ivec2 faceSize( imageSource->getWidth() / 4, imageSource->getHeight() / 3 );
		Area faceArea( 0, 0, faceSize.x, faceSize.y );

		Area area;
		ivec2 offset;

		// GL_TEXTURE_CUBE_MAP_POSITIVE_X
		area = faceArea + ivec2( faceSize.x * 2, faceSize.y * 1 );
		offset = -ivec2( faceSize.x * 2, faceSize.y * 1 );
		result.push_back( { area, offset, false } );
		// GL_TEXTURE_CUBE_MAP_NEGATIVE_X
		area = faceArea + ivec2( faceSize.x * 0, faceSize.y * 1 );
		offset = -ivec2( faceSize.x * 0, faceSize.y * 1 );
		result.push_back( { area, offset, false } );
		// GL_TEXTURE_CUBE_MAP_POSITIVE_Y
		area = faceArea + ivec2( faceSize.x * 1, faceSize.y * 0 );
		offset = -ivec2( faceSize.x * 1, faceSize.y * 0 );
		result.push_back( { area, offset, false } );
		// GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
		area = faceArea + ivec2( faceSize.x * 1, faceSize.y * 2 );
		offset = -ivec2( faceSize.x * 1, faceSize.y * 2 );
		result.push_back( { area, offset, false } );
		// GL_TEXTURE_CUBE_MAP_POSITIVE_Z
		area = faceArea + ivec2( faceSize.x * 1, faceSize.y * 1 );
		offset = -ivec2( faceSize.x * 1, faceSize.y * 1 );
		result.push_back( { area, offset, false } );
		// GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
		area = faceArea + ivec2( faceSize.x * 3, faceSize.y * 1 );
		offset = -ivec2( faceSize.x * 3, faceSize.y * 1 );
		result.push_back( { area, offset, false } );

		return result;
	};

	std::vector<CubeMapFaceRegion> calcCubeMapVerticalCrossRegions( const ImageSourceRef &imageSource )
	{
		std::vector<CubeMapFaceRegion> result;

		ivec2 faceSize( imageSource->getWidth() / 3, imageSource->getHeight() / 4 );
		Area faceArea( 0, 0, faceSize.x, faceSize.y );

		Area area;
		ivec2 offset;

		// GL_TEXTURE_CUBE_MAP_POSITIVE_X
		area = faceArea + ivec2( faceSize.x * 2, faceSize.y * 1 );
		offset = -ivec2( faceSize.x * 2, faceSize.y * 1 );
		result.push_back( { area, offset, false } );
		// GL_TEXTURE_CUBE_MAP_NEGATIVE_X
		area = faceArea + ivec2( faceSize.x * 0, faceSize.y * 1 );
		offset = -ivec2( faceSize.x * 0, faceSize.y * 1 );
		result.push_back( { area, offset, false } );
		// GL_TEXTURE_CUBE_MAP_POSITIVE_Y
		area = faceArea + ivec2( faceSize.x * 1, faceSize.y * 0 );
		offset = -ivec2( faceSize.x * 1, faceSize.y * 0 );
		result.push_back( { area, offset, false } );
		// GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
		area = faceArea + ivec2( faceSize.x * 1, faceSize.y * 2 );
		offset = -ivec2( faceSize.x * 1, faceSize.y * 2 );
		result.push_back( { area, offset, false } );
		// GL_TEXTURE_CUBE_MAP_POSITIVE_Z
		area = faceArea + ivec2( faceSize.x * 1, faceSize.y * 1 );
		offset = -ivec2( faceSize.x * 1, faceSize.y * 1 );
		result.push_back( { area, offset, false } );
		// GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
		area = faceArea + ivec2( faceSize.x * 1, faceSize.y * 3 );
		offset = -ivec2( faceSize.x * 1, faceSize.y * 3 );
		result.push_back( { area, offset, true } );

		return result;
	};

	std::vector<CubeMapFaceRegion> calcCubeMapHorizontalRegions( const ImageSourceRef &imageSource )
	{
		std::vector<CubeMapFaceRegion> result;
		ivec2 faceSize( imageSource->getHeight(), imageSource->getHeight() );

		for( uint8_t index = 0; index < 6; ++index ) {
			Area area( index * faceSize.x, 0, (index + 1) * faceSize.x, faceSize.y );
			ivec2 offset( -index * faceSize.x, 0 );
			result.push_back( { area, offset, false } );
		}

		return result;
	};

	std::vector<CubeMapFaceRegion> calcCubeMapVerticalRegions( const ImageSourceRef &imageSource )
	{
		std::vector<CubeMapFaceRegion> result;
		ivec2 faceSize( imageSource->getWidth(), imageSource->getWidth() );

		for( uint8_t index = 0; index < 6; ++index ) {
			Area area( 0, index * faceSize.x, faceSize.x, (index + 1) * faceSize.y );
			ivec2 offset( 0, -index * faceSize.y );
			result.push_back( { area, offset, false } );
		}

		return result;
	};

	TextureDesc getTextureCubeMapDesc( const TextureDesc &desc, uint32_t width, uint32_t height, uint32_t sourceNumComponents, uint32_t channelDepth, uint32_t rowStride )
	{
		uint32_t mipLevels = ComputeMipLevelsCount( width, height );
		if( desc.MipLevels > 0 )
			mipLevels = std::min( desc.MipLevels, mipLevels );

		TextureDesc textureDesc;
		textureDesc.Name      = desc.Name;
		textureDesc.Type      = RESOURCE_DIM_TEX_CUBE;
		textureDesc.Width     = width;
		textureDesc.Height    = height;
		textureDesc.ArraySize = 6;
		textureDesc.MipLevels = mipLevels;

		textureDesc.Usage          = desc.Usage;
		textureDesc.BindFlags      = desc.BindFlags;
		textureDesc.Format         = desc.Format;
		textureDesc.CPUAccessFlags = desc.CPUAccessFlags;

		uint32_t numComponents = sourceNumComponents == 3 ? 4 : sourceNumComponents;
		bool   isSRGB          = ( sourceNumComponents >= 3 && channelDepth == 8 ) ? desc.isSrgb() : false;

		if( textureDesc.Format == TEX_FORMAT_UNKNOWN ) {
			if( channelDepth == 8 ) {
				switch( numComponents ) {
				case 1: textureDesc.Format = TEX_FORMAT_R8_UNORM; break;
				case 2: textureDesc.Format = TEX_FORMAT_RG8_UNORM; break;
				case 4: textureDesc.Format = isSRGB ? TEX_FORMAT_RGBA8_UNORM_SRGB : TEX_FORMAT_RGBA8_UNORM; break;
				default: throw TextureDataExc( "Unexpected number of color channels (" + to_string( sourceNumComponents ) + ")" );
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

		return textureDesc;
	}
	
	void createTextureCubeMap( IRenderDevice* pDevice, const ImageSourceRef images[6], const TextureDesc &desc, ITexture** ppTexture )
	{
		uint32_t mipLevels = ComputeMipLevelsCount( images[0]->getWidth(), images[0]->getHeight() );
		if( desc.MipLevels > 0 )
			mipLevels = std::min( desc.MipLevels, mipLevels );

		std::vector<TextureSubResData>  subResources( mipLevels * 6u );
		std::vector<std::vector<Uint8>> mips( mipLevels * 6u );

		uint32_t sourceNumComponents = ImageIo::channelOrderNumChannels( images[0]->getChannelOrder() );
		uint32_t channelDepth = ImageIo::dataTypeBytes( images[0]->getDataType() ) * 8;
		uint32_t rowStride = static_cast<uint32_t>( images[0]->getRowBytes() );
		uint32_t numComponents = sourceNumComponents == 3 ? 4 : sourceNumComponents;

		TextureDesc textureDesc = getTextureCubeMapDesc( desc, images[0]->getWidth(), images[0]->getHeight(), sourceNumComponents, channelDepth, rowStride );

		for( size_t face = 0; face < 6; ++face ) {

			shared_ptr<BasicImageTarget> imageTarget = make_shared<BasicImageTarget>( images[face] );
			images[face]->load( imageTarget );

			const void* data = imageTarget->getData();					
			size_t faceSubResource = face * mipLevels;
			if( sourceNumComponents == 3 ) {
				VERIFY_EXPR( numComponents == 4 );
				uint32_t rgbaStride = desc.Width * numComponents * channelDepth / 8;
				rgbaStride = ( rgbaStride + 3 ) & ( -4 );
				mips[faceSubResource].resize( size_t { rgbaStride } * size_t { desc.Height } );
				subResources[faceSubResource].pData = mips[faceSubResource].data();
				subResources[faceSubResource].Stride = rgbaStride;
				if( channelDepth == 8 ) {
					RGBToRGBA<Uint8>( data, rowStride, mips[faceSubResource].data(), rgbaStride, textureDesc.Width, textureDesc.Height );
				}
				else if( channelDepth == 16 ) {
					RGBToRGBA<Uint16>( data, rowStride, mips[faceSubResource].data(), rgbaStride, textureDesc.Width, textureDesc.Height );
				}
				else if( channelDepth == 32 ) {
					RGBToRGBA<float>( data, rowStride, mips[faceSubResource].data(), rgbaStride, textureDesc.Width, textureDesc.Height );
				}
			}
			else {
				subResources[faceSubResource].pData  = data;
				subResources[faceSubResource].Stride = rowStride;
			}

			uint32_t mipWidth  = textureDesc.Width;
			uint32_t mipHeight = textureDesc.Height;
			for( uint32_t mip = 1; mip < textureDesc.MipLevels; ++mip ) {
				uint32_t coarseMipWidth  = std::max( mipWidth / 2u, 1u );
				uint32_t coarseMipHeight = std::max( mipHeight / 2u, 1u );
				uint32_t coarseMipStride = coarseMipWidth * numComponents * channelDepth / 8;
				coarseMipStride      = ( coarseMipStride + 3 ) & ( -4 );
				mips[faceSubResource+mip].resize( size_t { coarseMipStride } * size_t { coarseMipHeight } );

				if( desc.needsGenerateMips() ) {
					ComputeMipLevel( mipWidth, mipHeight, textureDesc.Format, subResources[faceSubResource+mip - 1].pData, subResources[faceSubResource+mip - 1].Stride, mips[faceSubResource+mip].data(), coarseMipStride );
				}

				subResources[faceSubResource+mip].pData  = mips[faceSubResource+mip].data();
				subResources[faceSubResource+mip].Stride = coarseMipStride;

				mipWidth  = coarseMipWidth;
				mipHeight = coarseMipHeight;
			}
		}

		TextureData texData;
		texData.pSubResources   = subResources.data();
		texData.NumSubresources = textureDesc.MipLevels * 6;

		pDevice->CreateTexture( textureDesc, &texData, ppTexture );
	}

	template<typename T>
	void createTextureCubeMap( IRenderDevice* pDevice, const SurfaceT<T> surfaces[6], const TextureDesc &desc, ITexture** ppTexture )
	{
		uint32_t mipLevels = ComputeMipLevelsCount( surfaces[0].getWidth(), surfaces[0].getHeight() );
		if( desc.MipLevels > 0 )
			mipLevels = std::min( desc.MipLevels, mipLevels );

		std::vector<TextureSubResData>  subResources( mipLevels * 6 );
		std::vector<std::vector<Uint8>> mips( mipLevels * 6 );
		
		uint32_t sourceNumComponents = surfaces[0].hasAlpha() ? 4 : 3;
		uint32_t channelDepth = sizeof( T ) * 8;
		uint32_t rowStride = static_cast<uint32_t>( surfaces[0].getRowBytes() );
		uint32_t numComponents = sourceNumComponents == 3 ? 4 : sourceNumComponents;

		TextureDesc textureDesc = getTextureCubeMapDesc( desc, surfaces[0].getWidth(), surfaces[0].getHeight(), sourceNumComponents, channelDepth, rowStride );

		for( size_t face = 0; face < 6; ++face ) {

			const void* data = surfaces[face].getData();
			size_t faceSubResource = face * mipLevels;
			if( sourceNumComponents == 3 ) {
				VERIFY_EXPR( numComponents == 4 );
				uint32_t rgbaStride = desc.Width * numComponents * channelDepth / 8;
				rgbaStride = ( rgbaStride + 3 ) & ( -4 );
				mips[faceSubResource].resize( size_t { rgbaStride } * size_t { desc.Height } );
				subResources[faceSubResource].pData = mips[faceSubResource].data();
				subResources[faceSubResource].Stride = rgbaStride;
				if( channelDepth == 8 ) {
					RGBToRGBA<Uint8>( data, rowStride, mips[faceSubResource].data(), rgbaStride, textureDesc.Width, textureDesc.Height );
				}
				else if( channelDepth == 16 ) {
					RGBToRGBA<Uint16>( data, rowStride, mips[faceSubResource].data(), rgbaStride, textureDesc.Width, textureDesc.Height );
				}
				else if( channelDepth == 32 ) {
					RGBToRGBA<float>( data, rowStride, mips[faceSubResource].data(), rgbaStride, textureDesc.Width, textureDesc.Height );
				}
			}
			else {
				subResources[faceSubResource].pData  = data;
				subResources[faceSubResource].Stride = rowStride;
			}

			uint32_t mipWidth  = textureDesc.Width;
			uint32_t mipHeight = textureDesc.Height;
			for( uint32_t mip = 1; mip < textureDesc.MipLevels; ++mip ) {
				uint32_t coarseMipWidth  = std::max( mipWidth / 2u, 1u );
				uint32_t coarseMipHeight = std::max( mipHeight / 2u, 1u );
				uint32_t coarseMipStride = coarseMipWidth * numComponents * channelDepth / 8;
				coarseMipStride      = ( coarseMipStride + 3 ) & ( -4 );
				mips[faceSubResource+mip].resize( size_t { coarseMipStride } * size_t { coarseMipHeight } );

				if( desc.needsGenerateMips() ) {
					ComputeMipLevel( mipWidth, mipHeight, textureDesc.Format, subResources[faceSubResource+mip - 1].pData, subResources[faceSubResource+mip - 1].Stride, mips[faceSubResource+mip].data(), coarseMipStride );
				}

				subResources[faceSubResource+mip].pData  = mips[faceSubResource+mip].data();
				subResources[faceSubResource+mip].Stride = coarseMipStride;

				mipWidth  = coarseMipWidth;
				mipHeight = coarseMipHeight;
			}
		}

		TextureData texData;
		texData.pSubResources   = subResources.data();
		texData.NumSubresources = textureDesc.MipLevels * 6;

		pDevice->CreateTexture( textureDesc, &texData, ppTexture );
	}

	static inline void latLongFromVec( float& _u, float& _v, const float _vec[3] )
	{
		const float phi = atan2f( _vec[0], _vec[2] );
		const float theta = acosf( _vec[1] );

		_u = ( glm::pi<float>() + phi ) * ( 0.5f * glm::one_over_pi<float>() );
		_v = theta * glm::one_over_pi<float>();
	}
	
    static const float s_faceUvVectors[6][3][3] =
    {
        { // +x face
            {  0.0f,  0.0f, -1.0f }, // u -> -z
            {  0.0f, -1.0f,  0.0f }, // v -> -y
            {  1.0f,  0.0f,  0.0f }, // +x face
        },
        { // -x face
            {  0.0f,  0.0f,  1.0f }, // u -> +z
            {  0.0f, -1.0f,  0.0f }, // v -> -y
            { -1.0f,  0.0f,  0.0f }, // -x face
        },
        { // +y face
            {  1.0f,  0.0f,  0.0f }, // u -> +x
            {  0.0f,  0.0f,  1.0f }, // v -> +z
            {  0.0f,  1.0f,  0.0f }, // +y face
        },
        { // -y face
            {  1.0f,  0.0f,  0.0f }, // u -> +x
            {  0.0f,  0.0f, -1.0f }, // v -> -z
            {  0.0f, -1.0f,  0.0f }, // -y face
        },
        { // +z face
            {  1.0f,  0.0f,  0.0f }, // u -> +x
            {  0.0f, -1.0f,  0.0f }, // v -> -y
            {  0.0f,  0.0f,  1.0f }, // +z face
        },
        { // -z face
            { -1.0f,  0.0f,  0.0f }, // u -> -x
            {  0.0f, -1.0f,  0.0f }, // v -> -y
            {  0.0f,  0.0f, -1.0f }, // -z face
        }
    };

	/// _u and _v should be center adressing and in [-1.0+invSize..1.0-invSize] range.
	static inline void texelCoordToVec( float* _out3f, float _u, float _v, uint8_t _faceId )
	{
		// out = u * s_faceUv[0] + v * s_faceUv[1] + s_faceUv[2].
		_out3f[0] = s_faceUvVectors[_faceId][0][0] * _u + s_faceUvVectors[_faceId][1][0] * _v + s_faceUvVectors[_faceId][2][0];
		_out3f[1] = s_faceUvVectors[_faceId][0][1] * _u + s_faceUvVectors[_faceId][1][1] * _v + s_faceUvVectors[_faceId][2][1];
		_out3f[2] = s_faceUvVectors[_faceId][0][2] * _u + s_faceUvVectors[_faceId][1][2] * _v + s_faceUvVectors[_faceId][2][2];

		// Normalize.
		const float invLen = 1.0f/sqrtf( _out3f[0]*_out3f[0] + _out3f[1]*_out3f[1] + _out3f[2]*_out3f[2] );
		_out3f[0] *= invLen;
		_out3f[1] *= invLen;
		_out3f[2] *= invLen;
	}


	template<typename T>
	void createTextureCubeMapLatLong( IRenderDevice* pDevice, const ImageSourceRef &imageSource, const TextureDesc &desc, bool bilinearInterpolation, ITexture** ppTexture )
	{
		uint32_t faceSize = ( imageSource->getHeight() + 1 ) / 2;
		uint32_t srcNumComponents = ImageIo::channelOrderNumChannels( imageSource->getChannelOrder() );
		uint32_t srcRowStride = static_cast<uint32_t>( imageSource->getRowBytes() );
		uint32_t srcPixelBytes = sizeof( T ) * srcNumComponents;
		uint32_t channelDepth = ImageIo::dataTypeBytes( imageSource->getDataType() ) * 8;
		float faceSizeInv = 1.0f / float( faceSize );

		const float srcWidthMinusOne  = float( imageSource->getWidth() - 1 );
		const float srcHeightMinusOne = float( imageSource->getHeight() - 1 );

		shared_ptr<BasicImageTarget> imageTarget = make_shared<BasicImageTarget>( imageSource );
		imageSource->load( imageTarget );

		const void* srcData = imageTarget->getData();

		SurfaceT<T> images[6];
		for( size_t face = 0; face < 6; ++face ) {
			images[face] = SurfaceT<T>( faceSize, faceSize, true, SurfaceConstraints() );

			for( uint32_t y = 0; y < faceSize; ++y ) {
				T *dstRowPtr = reinterpret_cast<T*>( reinterpret_cast<uint8_t*>( images[face].getData() ) + y * images[face].getRowBytes() );
				for( uint32_t x = 0; x < faceSize; ++x ) {
					T *dstPixelPtr = reinterpret_cast<T*>( reinterpret_cast<uint8_t*>( dstRowPtr ) + x * images[face].getPixelBytes() );

					// Cubemap (u,v) on current face.
					const float uu = 2.0f * x * faceSizeInv - 1.0f;
					const float vv = 2.0f * y * faceSizeInv - 1.0f;

					// Get cubemap vector (x,y,z) from (u,v,faceIdx).
					float vec[3];
					texelCoordToVec( vec, uu, vv, face );

					// Convert cubemap vector (x,y,z) to latlong (u,v).
					float xSrcf;
					float ySrcf;
					latLongFromVec( xSrcf, ySrcf, vec );

					// Convert from [0..1] to [0..(size-1)] range.
					xSrcf *= srcWidthMinusOne;
					ySrcf *= srcHeightMinusOne;

					if( bilinearInterpolation ) {
						const uint32_t x0 = uint32_t( int32_t( xSrcf ) );
						const uint32_t y0 = uint32_t( int32_t( ySrcf ) );
						const uint32_t x1 = min( x0 + 1, uint32_t( imageSource->getWidth() ) - 1 );
						const uint32_t y1 = min( y0 + 1, uint32_t( imageSource->getHeight() ) - 1 );

						const T *src0 = reinterpret_cast<const T*>( reinterpret_cast<const uint8_t*>( srcData ) + y0 * srcRowStride + x0 * srcPixelBytes );
						const T *src1 = reinterpret_cast<const T*>( reinterpret_cast<const uint8_t*>( srcData ) + y0 * srcRowStride + x1 * srcPixelBytes );
						const T *src2 = reinterpret_cast<const T*>( reinterpret_cast<const uint8_t*>( srcData ) + y1 * srcRowStride + x0 * srcPixelBytes );
						const T *src3 = reinterpret_cast<const T*>( reinterpret_cast<const uint8_t*>( srcData ) + y1 * srcRowStride + x1 * srcPixelBytes );

						const float tx = xSrcf - float( int32_t( x0 ) );
						const float ty = ySrcf - float( int32_t( y0 ) );
						const float invTx = 1.0f - tx;
						const float invTy = 1.0f - ty;

						const float w0 = invTx * invTy;
						const float w1 = tx * invTy;
						const float w2 = invTx * ty;
						const float w3 = tx * ty;

						T p0[4] = { src0[0] * w0, src0[1] * w0, src0[2] * w0, srcNumComponents > 3 ? src0[3] * w0 : CHANTRAIT<T>::max() * w0 };
						T p1[4] = { src1[0] * w1, src1[1] * w1, src1[2] * w1, srcNumComponents > 3 ? src1[3] * w1 : CHANTRAIT<T>::max() * w1 };
						T p2[4] = { src2[0] * w2, src2[1] * w2, src2[2] * w2, srcNumComponents > 3 ? src2[3] * w2 : CHANTRAIT<T>::max() * w2 };
						T p3[4] = { src3[0] * w3, src3[1] * w3, src3[2] * w3, srcNumComponents > 3 ? src3[3] * w3 : CHANTRAIT<T>::max() * w3 };

						const T rr = p0[0] + p1[0] + p2[0] + p3[0];
						const T gg = p0[1] + p1[1] + p2[1] + p3[1];
						const T bb = p0[2] + p1[2] + p2[2] + p3[2];
						const T aa = p0[3] + p1[3] + p2[3] + p3[3];

						dstPixelPtr[0] = rr;
						dstPixelPtr[1] = gg;
						dstPixelPtr[2] = bb;
						dstPixelPtr[3] = aa;
					}
					else {
						const uint32_t xSrc = uint32_t( int32_t( xSrcf ) );
						const uint32_t ySrc = uint32_t( int32_t( ySrcf ) );
						const T *srcPtr = reinterpret_cast<const T*>( reinterpret_cast<const uint8_t*>( srcData ) + ySrc * srcRowStride + xSrc * srcPixelBytes );

						dstPixelPtr[0] = srcPtr[0];
						dstPixelPtr[1] = srcPtr[1];
						dstPixelPtr[2] = srcPtr[2];
						dstPixelPtr[3] = srcNumComponents > 3 ? srcPtr[3] : CHANTRAIT<T>::max();
					}
				}
			}
		}

		createTextureCubeMap( pDevice, images, desc, ppTexture );
	}

	template<typename T>
	void createTextureCubeMap( IRenderDevice* pDevice, const ImageSourceRef &imageSource, const TextureDesc &desc, ITexture** ppTexture )
	{
		std::vector<CubeMapFaceRegion> faceRegions;

		// Infer the layout based on image aspect ratio
		ivec2 imageSize( imageSource->getWidth(), imageSource->getHeight() );
		float minDim = (float) std::min( imageSize.x, imageSize.y );
		float maxDim = (float) std::max( imageSize.x, imageSize.y );
		float aspect = minDim / maxDim;
		if( abs( aspect - 1.0f / 2.0f ) < 0.0001f ) { // latlong
			createTextureCubeMapLatLong<T>( pDevice, imageSource, desc, true, ppTexture );
		}
		else {
			if( abs( aspect - 1 / 6.0f ) < abs( aspect - 3 / 4.0f ) ) { // closer to 1:6 than to 4:3, so row or column
				faceRegions = ( imageSize.x > imageSize.y ) ? calcCubeMapHorizontalRegions( imageSource ) : calcCubeMapVerticalRegions( imageSource );
			}
			else { // else, horizontal or vertical cross
				faceRegions = ( imageSize.x > imageSize.y ) ? calcCubeMapHorizontalCrossRegions( imageSource ) : calcCubeMapVerticalCrossRegions( imageSource );
			}

			Area faceArea = faceRegions.front().mArea;
			ivec2 faceSize = faceArea.getSize();

			SurfaceT<T> masterSurface( imageSource, SurfaceConstraintsDefault() );
			SurfaceT<T> images[6];

			for( uint8_t f = 0; f < 6; ++f ) {
				images[f] = SurfaceT<T>( faceSize.x, faceSize.y, masterSurface.hasAlpha(), SurfaceConstraints() );
				images[f].copyFrom( masterSurface, faceRegions[f].mArea, faceRegions[f].mOffset );
				if( faceRegions[f].mFlip ) {
					//ip::flipVertical( &images[f] );
					//ip::flipHorizontal( &images[f] );
				}
			}

			createTextureCubeMap( pDevice, images, desc, ppTexture );
		}
	}

} // anonymous namespace

TextureRef createTextureCubeMap( RenderDevice* device, const ImageSourceRef &imageSource, const TextureDesc &desc )
{
	TextureRef texture;
	Surface s;
	s.getPixelBytes();
	TextureDesc textureDesc = getDefaultTextureDesc( desc, imageSource->getWidth(), imageSource->getHeight() );
	if( imageSource->getDataType() == ImageIo::UINT8 ) {
		createTextureCubeMap<uint8_t>( device, imageSource, textureDesc, &texture );
	}
	else if( imageSource->getDataType() == ImageIo::UINT16 ) {
		createTextureCubeMap<uint16_t>( device, imageSource, textureDesc, &texture );
	}
	else {
		createTextureCubeMap<float>( device, imageSource, textureDesc, &texture );
	}
	return texture;
}

TextureRef createTextureCubeMap( RenderDevice* device, const ImageSourceRef images[6], const TextureDesc &desc )
{
	TextureRef texture;
	TextureDesc textureDesc = getDefaultTextureDesc( desc, images[0]->getWidth(), images[0]->getHeight() );
	createTextureCubeMap( device, images, textureDesc, &texture );
	return texture;
}

}

namespace gx = graphics;
} // namespace cinder::graphics