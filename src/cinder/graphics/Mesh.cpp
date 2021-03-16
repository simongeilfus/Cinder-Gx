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

#include "cinder/graphics/Mesh.h"
#include "cinder/app/RendererGx.h"
#include "cinder/Log.h"

using namespace std;

namespace cinder { namespace graphics {

class MeshGeomTarget : public geom::Target {
  public:
	struct BufferData {
		BufferData( const Mesh::BufferInfo &info, uint8_t *data, size_t dataSize )
			: mInfo( info ), mData( data ), mDataSize( dataSize )
		{}
		BufferData( BufferData &&rhs )
			: mInfo( rhs.mInfo ), mData( std::move( rhs.mData ) ), mDataSize( rhs.mDataSize )
		{}
	
		Mesh::BufferInfo			mInfo;
		std::unique_ptr<uint8_t[]>	mData;
		size_t						mDataSize;
	};

	MeshGeomTarget( Mesh *mesh, const geom::Source &source, const std::vector<Mesh::BufferInfo> &bufferInfos );
	
	uint8_t	getAttribDims( geom::Attrib attr ) const override;
	void	copyAttrib( geom::Attrib attr, uint8_t dims, size_t strideBytes, const float *srcData, size_t count ) override;
	void	copyIndices( geom::Primitive primitive, const uint32_t *source, size_t numIndices, uint8_t requiredBytesPerIndex ) override;

	const std::vector<BufferData>& getBufferData() const { return mBufferData; }
	size_t getNumVertices() const { return mNumVertices; }
	
  protected:
	std::vector<BufferData>	mBufferData;
	size_t					mNumVertices;
	Mesh					*mMesh;
};

MeshGeomTarget::MeshGeomTarget( Mesh *mesh, const geom::Source &source, const std::vector<Mesh::BufferInfo> &bufferInfos )
	: mMesh( mesh ),
	mNumVertices( source.getNumVertices() )
{	
	// if no buffer infos available deduce them from what is available in the source
	std::vector<Mesh::BufferInfo> infos;
	if( bufferInfos.empty() ) {
		// gather attribute infos and calculate the vertex stride
		// assume geom::Source creates only floating points attributes
		size_t totalByteSize = 0;
		vector<geom::AttribInfo> attribInfos;
		for( const auto &attrib : source.getAvailableAttribs() ) {
			uint8_t dims = source.getAttribDims( attrib );
			if( dims > 0 ) {
				attribInfos.push_back( { attrib, dims, 0, totalByteSize } );
				totalByteSize += dims * sizeof( float );
			}
		}
		// build a BufferInfo from the attribInfos that were just collected
		Mesh::BufferInfo interleavedInfo;
		for( const auto &attribInfo : attribInfos ) {
			interleavedInfo.append( attribInfo.getAttrib(), geom::DataType::FLOAT, attribInfo.getDims(), totalByteSize, attribInfo.getOffset() );
		}
		infos = { interleavedInfo };
	}
	// otherwise verify that the content of each BufferInfo inside bufferInfos matches the source and is accurate in terms of strides and offsets
	else {
		for( const auto &bufferInfo : bufferInfos ) {
			size_t totalByteSize = 0;
			vector<geom::AttribInfo> attribInfos;
			for( const auto &attribInfo : bufferInfo.getAttribs() ) {
				if( source.getAttribDims( attribInfo.getAttrib() ) ) {
					uint8_t dims = attribInfo.getDims() ? attribInfo.getDims() : source.getAttribDims( attribInfo.getAttrib() );
					attribInfos.push_back( { attribInfo.getAttrib(), dims, 0, totalByteSize } );
					size_t byteSize = attribInfo.getDataType() == geom::DataType::FLOAT ? sizeof( float ) : attribInfo.getDataType() == geom::DataType::INTEGER ? sizeof( int ) : sizeof( double );
					totalByteSize += dims * byteSize;
				}
			}
			Mesh::BufferInfo approvedBufferInfo = Mesh::BufferInfo().bindFlags( bufferInfo.mBindFlags ).cpuAccess( bufferInfo.mCPUAccessFlags ).usage( bufferInfo.mUsage ).mode( bufferInfo.mMode );
			for( const auto &attribInfo : attribInfos ) {
				approvedBufferInfo.append( attribInfo.getAttrib(), attribInfo.getDataType(), attribInfo.getDims(), totalByteSize, attribInfo.getOffset() );
			}
			infos.push_back( approvedBufferInfo );
		}
	}
	// allocate the list of BufferData
	for( const auto &info : infos ) {
		size_t requiredBytes = info.calcRequiredStorage( mNumVertices );
		mBufferData.push_back( BufferData( info, new uint8_t[requiredBytes], requiredBytes ) );
	}
}

uint8_t	MeshGeomTarget::getAttribDims( geom::Attrib attrib ) const
{
	for( const auto &data : mBufferData ) {
		if( data.mInfo.hasAttrib( attrib ) ) {
			return data.mInfo.getAttribDims( attrib );
		}
	}
	return 0;
}

void MeshGeomTarget::copyAttrib( geom::Attrib attr, uint8_t dims, size_t /*strideBytes*/, const float *srcData, size_t count )
{
	// if we don't have it we don't want it
	if( getAttribDims( attr ) == 0 )
		return;

	// we need to find which element of 'mBufferData' containts 'attr'
	uint8_t *dstData = nullptr;
	uint8_t dstDims;
	size_t dstStride, dstDataSize;
	for( const auto &bufferData : mBufferData ) {
		if( bufferData.mInfo.hasAttrib( attr ) ) {
			auto attrInfo = bufferData.mInfo.getAttribInfo( attr );
			dstDims = attrInfo.getDims();
			dstStride = attrInfo.getStride();
			dstData = bufferData.mData.get() + attrInfo.getOffset();
			dstDataSize = bufferData.mDataSize;
			break;
		}
	}
	CI_ASSERT( dstData );

	// verify we've been called with the number of vertices we were promised earlier
	if( count != mNumVertices ) {
		CI_LOG_E( "copyAttrib() called with " << count << " elements. " << mNumVertices << " expected." );
		return;
	}

	// verify we have room for this data
	auto testDstStride = dstStride ? dstStride : ( dstDims * sizeof( float ) );
	if( dstDataSize < count * testDstStride ) {
		CI_LOG_E( "copyAttrib() called with inadequate attrib data storage allocated" );
		return;
	}

	if( dstData )
		geom::copyData( dims, srcData, count, dstDims, dstStride, reinterpret_cast<float*>( dstData ) );
}

void MeshGeomTarget::copyIndices( geom::Primitive /*primitive*/, const uint32_t *source, size_t numIndices, uint8_t requiredBytesPerIndex )
{
	mMesh->mNumIndices = (uint32_t) numIndices;
	if( mMesh->mNumIndices == 0 ) {
		mMesh->mIndices = BufferRef();
	}
	else if( requiredBytesPerIndex <= 2 ) {
		mMesh->mIndexType = VT_UINT16;
		std::unique_ptr<uint16_t[]> indices( new uint16_t[numIndices] );
		copyIndexData( source, mMesh->mNumIndices, indices.get() );
		
		mMesh->mIndices.Release();
		gx::BufferData data = { indices.get(), mMesh->mNumIndices * sizeof( uint16_t ) };
		mMesh->mDevice->CreateBuffer( BufferDesc()
			.name( "Mesh Index Buffer" )
			.usage( USAGE_IMMUTABLE )
			.bindFlags( BIND_INDEX_BUFFER )
			.cpuAccessFlags( CPU_ACCESS_NONE )
			.sizeInBytes( mMesh->mNumIndices * sizeof( uint16_t ) ),
			&data, &mMesh->mIndices );
	}
	else {
		mMesh->mIndexType = VT_UINT32;
		std::unique_ptr<uint32_t[]> indices( new uint32_t[numIndices] );
		copyIndexData( source, mMesh->mNumIndices, indices.get() );

		mMesh->mIndices.Release();
		gx::BufferData data ={ indices.get(), mMesh->mNumIndices * sizeof( uint32_t ) };
		mMesh->mDevice->CreateBuffer( BufferDesc()
			.name( "Mesh Index Buffer" )
			.usage( USAGE_IMMUTABLE )
			.bindFlags( BIND_INDEX_BUFFER )
			.cpuAccessFlags( CPU_ACCESS_NONE )
			.sizeInBytes( mMesh->mNumIndices * sizeof( uint32_t ) ),
			&data, &mMesh->mIndices );
	}
}

Mesh::Mesh( const geom::Source &source )
	: Mesh( app::getRenderDevice(), source )
{
}

Mesh::Mesh( const geom::Source &source, const geom::AttribSet &requestedAttribs )
	: Mesh( app::getRenderDevice(), source, requestedAttribs )
{
}

Mesh::Mesh( const geom::Source &source, const std::vector<Mesh::BufferInfo> &bufferInfos )
	: Mesh( app::getRenderDevice(), source, bufferInfos )
{
}

Mesh::Mesh( uint32_t numVertices, const std::vector<std::pair<BufferInfo, BufferRef>> &vertexBuffers, geom::Primitive primitiveType )
	: Mesh( app::getRenderDevice(), numVertices, vertexBuffers, primitiveType )
{
}

Mesh::Mesh( uint32_t numVertices, const void* vertexData, uint32_t vertexDataSize, const BufferInfo &bufferInfo, geom::Primitive primitiveType )
	: Mesh( app::getRenderDevice(), numVertices, vertexData, vertexDataSize, bufferInfo, primitiveType )
{
}

Mesh::Mesh( const std::vector<std::pair<BufferInfo, BufferRef>> &vertexBuffers, uint32_t numIndices, const BufferRef &indexBuffer, VALUE_TYPE indexType, geom::Primitive primitiveType )
	: Mesh( app::getRenderDevice(), vertexBuffers, numIndices, indexBuffer, indexType, primitiveType )
{
}

Mesh::Mesh( const void* vertexData, uint32_t vertexDataSize, const BufferInfo &bufferInfo, uint32_t numIndices, const void *indexData, VALUE_TYPE indexType, geom::Primitive primitiveType )
	: Mesh( app::getRenderDevice(), vertexData, vertexDataSize, bufferInfo, numIndices, indexData, indexType, primitiveType )
{
}

namespace {
	vector<Mesh::BufferInfo> makeInterleavedBufferInfos( const geom::AttribSet &requestedAttribs )
	{
		if( ! requestedAttribs.empty() ) {
			// make an interleaved Mesh::Layout with 'requestedAttribs'
			Mesh::BufferInfo info;
			for( const auto &attrib : requestedAttribs )
				info.attrib( attrib, 0, 0, 0 ); // 0 dim implies querying the Source for its dimension
			return { info };
		}
		else {
			return {};
		}
	}

	PRIMITIVE_TOPOLOGY convertPrimitiveType( geom::Primitive prim )
	{
		switch( prim ) {
		case geom::Primitive::LINES:
			return PRIMITIVE_TOPOLOGY_LINE_LIST;
			break;
		case geom::Primitive::LINE_STRIP:
			return PRIMITIVE_TOPOLOGY_LINE_STRIP;
			break;
		case geom::Primitive::TRIANGLES:
			return PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			break;
		case geom::Primitive::TRIANGLE_STRIP:
			return PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
			break;
		case geom::Primitive::TRIANGLE_FAN:
			CI_LOG_E( "geom::Primitive::TRIANGLE_FAN not supported" );
			return PRIMITIVE_TOPOLOGY_UNDEFINED;
		default:
			return PRIMITIVE_TOPOLOGY_UNDEFINED; // no clear right choice here
		}
	}

	geom::Primitive convertPrimitiveType( PRIMITIVE_TOPOLOGY prim )
	{
		switch( prim ) {
		case PRIMITIVE_TOPOLOGY_LINE_LIST:
			return geom::Primitive::LINES;
			break;
		case PRIMITIVE_TOPOLOGY_LINE_STRIP:
			return geom::Primitive::LINE_STRIP;
			break;
		case PRIMITIVE_TOPOLOGY_TRIANGLE_LIST:
			return geom::Primitive::TRIANGLES;
			break;
		case PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP:
			return geom::Primitive::TRIANGLE_STRIP;
			break;
		case PRIMITIVE_TOPOLOGY_UNDEFINED:
			CI_LOG_E( "geom::Primitive::TRIANGLE_FAN not supported" );
			return geom::Primitive::TRIANGLE_FAN;
		default:
			return geom::Primitive::TRIANGLES;
		}
	}
} // anonymous namespace

Mesh::Mesh( RenderDevice* device, const geom::Source &source )
	: Mesh( device, source, geom::AttribSet() )
{
}

Mesh::Mesh( RenderDevice* device, const geom::Source &source, const geom::AttribSet &requestedAttribs )
	: Mesh( device, source, makeInterleavedBufferInfos( requestedAttribs ) )
{
}

Mesh::Mesh( RenderDevice* device, const geom::Source &source, const std::vector<BufferInfo> &bufferInfos )
	: mDevice( device ),
	mNumVertices( 0 ),
	mNumIndices( 0 ),
	mPrimitiveTopology( convertPrimitiveType( source.getPrimitive() ) )
{
	// determine set of attributes to request from the source
	geom::AttribSet requestedAttribs;
	if( ! bufferInfos.empty() ) {
		for( const auto &layout : bufferInfos ) {
			for( const auto &attribInfo : layout.getAttribs() ) {
				requestedAttribs.insert( attribInfo.getAttrib() );
			}
		}
	}
	else {
		requestedAttribs = source.getAvailableAttribs();
	}
	// load vertices and indices from the source
	MeshGeomTarget target( this, source, bufferInfos );
	source.loadInto( &target, requestedAttribs );

	// allocate vertex buffers and build LayoutElement data 
	uint32_t bufferSlot = 0;
	uint32_t inputIndex = 0;
	for( const auto &bufferData : target.getBufferData() ) {
		BufferRef buffer;
		BufferData data = { bufferData.mData.get(), static_cast<uint32_t>( bufferData.mDataSize ) };
		device->CreateBuffer( BufferDesc()
				.name( bufferData.mInfo.mName.c_str() )
				.usage( bufferData.mInfo.mUsage )
				.bindFlags( bufferData.mInfo.mBindFlags )
				.cpuAccessFlags( bufferData.mInfo.mCPUAccessFlags )
				.sizeInBytes( static_cast<uint32_t>( bufferData.mDataSize ) ),
			&data, &buffer );
		mVertexBuffers.push_back( buffer );
		mVertexBuffersInfos.push_back( bufferData.mInfo );
		for( const auto &attribInfo : bufferData.mInfo.getAttribs() ) {
			LayoutElement layout;
			layout.InputIndex = inputIndex;
			layout.BufferSlot = bufferSlot;
			layout.NumComponents = static_cast<uint32_t>( attribInfo.getDims() );
			layout.ValueType = VT_FLOAT32;
			layout.IsNormalized = bufferData.mInfo.mIsNormalized; //TODO: wrong! should be per attrib
			mVertexLayoutElements.push_back( layout );
			inputIndex++;
		}
		bufferSlot++;
	}

	mNumVertices = static_cast<uint32_t>( target.getNumVertices() );
}

namespace {
	std::pair<Mesh::BufferInfo, BufferRef> makeBufferInfoPair( RenderDevice* device, const void* vertexData, uint32_t vertexDataSize, const Mesh::BufferInfo &bufferInfo )
	{
		gx::BufferData data = { vertexData, vertexDataSize };
		gx::BufferRef buffer;
		device->CreateBuffer( gx::BufferDesc()
			.name( bufferInfo.getName().c_str() )
			.usage( bufferInfo.getUsage() )
			.bindFlags( bufferInfo.getBindFlags() )
			.sizeInBytes( vertexDataSize ),
			&data, &buffer
		);

		return { bufferInfo, buffer };
	}

	BufferRef makeIndexBuffer( RenderDevice* device, uint32_t numIndices, const void *indexData, VALUE_TYPE indexType )
	{
		uint32_t size = numIndices * static_cast<uint32_t>( indexType == VT_UINT16 ? sizeof( uint16_t ) : sizeof( uint32_t ) );
		gx::BufferData data = { indexData, size };
		gx::BufferRef buffer;
		device->CreateBuffer( gx::BufferDesc()
			.name( "Mesh index buffer" )
			.usage( gx::USAGE_IMMUTABLE )
			.bindFlags( gx::BIND_INDEX_BUFFER )
			.sizeInBytes( size ),
			&data, &buffer
		);

		return buffer;
	}
}

Mesh::Mesh( RenderDevice* device, uint32_t numVertices, const std::vector<std::pair<BufferInfo, BufferRef>> &vertexBuffers, geom::Primitive primitiveType )
	: Mesh( device, vertexBuffers, 0, BufferRef(), VT_UINT16, primitiveType )
{
	mNumVertices = numVertices;
}

Mesh::Mesh( RenderDevice* device, uint32_t numVertices, const void* vertexData, uint32_t vertexDataSize, const BufferInfo &bufferInfo, geom::Primitive primitiveType )
	: Mesh( device, { makeBufferInfoPair( device, vertexData, vertexDataSize, bufferInfo ) }, 0, BufferRef(), VT_UINT16, primitiveType )
{
	mNumVertices = numVertices;
}

Mesh::Mesh( RenderDevice* device, const std::vector<std::pair<BufferInfo, BufferRef>> &vertexBuffers, uint32_t numIndices, const BufferRef &indexBuffer, VALUE_TYPE indexType, geom::Primitive primitiveType )
	: mDevice( device ),
	mNumIndices( indexBuffer ? numIndices : 0 ),
	mIndexType( indexType ),
	mIndices( indexBuffer ),
	mPrimitiveTopology( convertPrimitiveType( primitiveType ) )
{
	// assign vertex buffers and build LayoutElement data 
	uint32_t bufferSlot = 0;
	uint32_t inputIndex = 0;
	for( const auto &vertexBuffer : vertexBuffers ) {
		mVertexBuffersInfos.push_back( vertexBuffer.first );
		mVertexBuffers.push_back( vertexBuffer.second );
		for( const auto &attribInfo : vertexBuffer.first.getAttribs() ) {
			LayoutElement layout;
			layout.InputIndex = inputIndex;
			layout.BufferSlot = bufferSlot;
			layout.NumComponents = static_cast<uint32_t>( attribInfo.getDims() );
			CI_ASSERT_MSG( attribInfo.getDataType() != geom::DataType::DOUBLE, "geom::DataType::DOUBLE not supported" );
			layout.ValueType = attribInfo.getDataType() == geom::DataType::INTEGER ? gx::VT_INT32 : gx::VT_FLOAT32;
			layout.IsNormalized = vertexBuffer.first.mIsNormalized; //TODO: wrong! should be per attrib
			mVertexLayoutElements.push_back( layout );
			inputIndex++;
		}
		bufferSlot++;
	}
}

Mesh::Mesh( RenderDevice* device, const void* vertexData, uint32_t vertexDataSize, const BufferInfo &bufferInfo, uint32_t numIndices, const void *indexData, VALUE_TYPE indexType, geom::Primitive primitiveType )
	: Mesh( device, { makeBufferInfoPair( device, vertexData, vertexDataSize, bufferInfo ) }, numIndices, makeIndexBuffer( device, numIndices, indexData, indexType ), indexType, primitiveType )
{
}


uint8_t	Mesh::getAttribDims( geom::Attrib attr ) const
{
	return 0;
}

geom::AttribSet	Mesh::getAttribs() const
{
	return {};
}

InputLayoutDesc Mesh::getInputLayoutDesc() const
{
	return { mVertexLayoutElements.data(), static_cast<uint32_t>( mVertexLayoutElements.size() ) };
}

void Mesh::draw( DRAW_FLAGS flags ) const
{
	draw( app::getImmediateContext(), flags );
}

void Mesh::draw( DeviceContext* context, DRAW_FLAGS flags ) const
{
	uint32_t offset = 0;
	vector<gx::Buffer*> buffers;
	for( auto buffer : getVertexBuffers() ) {
		buffers.push_back( buffer );
	}
	context->SetVertexBuffers( 0, buffers.size(), &buffers[0], &offset, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, SET_VERTEX_BUFFERS_FLAG_RESET );

	if( getNumIndices() ) {
		context->SetIndexBuffer( getIndexBuffer(), 0, gx::RESOURCE_STATE_TRANSITION_MODE_TRANSITION );
		context->DrawIndexed( gx::DrawIndexedAttribs().indexType( getIndexDataType() ).numIndices( getNumIndices() ).flags( flags ) );
	}
	else {
		context->Draw( gx::DrawAttribs().numVertices( getNumVertices() ).flags( flags ) );
	}
}

}

namespace gx = graphics;
} // namespace cinder::graphics