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

#include "cinder/graphics/Buffer.h"

#include "cinder/TriMesh.h"
#include "cinder/GeomIo.h"

namespace cinder { namespace graphics {
	
class CI_API Mesh {
public:
	//! BufferInfo describes both the layout of a vertex buffer and its initial format
	class CI_API BufferInfo : public geom::BufferLayout {
	public:
		BufferInfo() : mBindFlags( BIND_VERTEX_BUFFER ), mUsage( USAGE_IMMUTABLE ), mCPUAccessFlags( CPU_ACCESS_NONE ), mMode( BUFFER_MODE_UNDEFINED ) {}
		BufferInfo( const std::vector<geom::AttribInfo> &attribs ) : geom::BufferLayout( attribs ), mBindFlags( BIND_VERTEX_BUFFER ), mUsage( USAGE_IMMUTABLE ), mCPUAccessFlags( CPU_ACCESS_NONE ), mMode( BUFFER_MODE_UNDEFINED ) {}

		//! Appends a new attribute to the layout
		BufferInfo& attrib( const geom::Attrib &attrib, uint8_t dims, size_t stride, size_t offset, uint32_t instanceDivisor = 0 ) { append( attrib, dims, stride, offset, instanceDivisor ); return *this; }
		//! Appends a new attribute to the layout
		BufferInfo& attrib( const geom::Attrib &attrib, geom::DataType dataType, uint8_t dims, size_t stride, size_t offset, uint32_t instanceDivisor = 0 ) { append( attrib, dataType, dims, stride, offset, instanceDivisor ); return *this; }
		//! Buffer bind flags, see Diligent::BIND_FLAGS for details
		BufferInfo& bindFlags( BIND_FLAGS bindFlags ) { mBindFlags = bindFlags; return *this; }
		//! Buffer usage, see Diligent::USAGE for details
		BufferInfo& usage( USAGE usage ) { mUsage = usage; return *this; }
		//! CPU access flags or 0 if no CPU access is allowed, see Diligent::CPU_ACCESS_FLAGS for details.
		BufferInfo& cpuAccess( CPU_ACCESS_FLAGS cpuAccessFlags ) { mCPUAccessFlags = cpuAccessFlags; return *this; }
		//! Buffer mode, see Diligent::BUFFER_MODE
		BufferInfo& mode( BUFFER_MODE mode ) { mMode = mode; return *this; }
		//! For signed and unsigned integer value types indicates if the value should be normalized to [-1,+1] or [0, 1] range respectively. For floating point types, this member is ignored.
		BufferInfo& normalized( bool normalized ) { mIsNormalized = normalized; return *this; }
		//! Specifies the Buffer name
		BufferInfo& name( const std::string &name ) { mName = name; return *this; }

		//! Returns Buffer bind flags, see Diligent::BIND_FLAGS for details
		BIND_FLAGS			getBindFlags() const { return mBindFlags; }
		//! Returns Buffer usage, see Diligent::USAGE for details
		USAGE				getUsage() const { return mUsage; }
		//! Returns CPU access flags or 0 if no CPU access is allowed, see Diligent::CPU_ACCESS_FLAGS for details.
		CPU_ACCESS_FLAGS	getCPUAccessFlags() const { return mCPUAccessFlags; }
		//! Returns Buffer mode, see Diligent::BUFFER_MODE
		BUFFER_MODE			getMode() const { return mMode; }
		//! Returns 
		bool				getIsNormalized() const { return mIsNormalized; }
		//! Returns the Buffer name
		std::string			getName() const { return mName; }
	protected:
		BIND_FLAGS			mBindFlags;
		USAGE				mUsage;
		CPU_ACCESS_FLAGS	mCPUAccessFlags;
		BUFFER_MODE			mMode;
		bool				mIsNormalized;
		std::string			mName;

		friend Mesh;
		friend class MeshGeomTarget;
	};
		
	Mesh() = default;

	Mesh( const geom::Source &source );
	Mesh( const geom::Source &source, const geom::AttribSet &requestedAttribs );
	Mesh( const geom::Source &source, const std::vector<Mesh::BufferInfo> &bufferInfos );
	Mesh( uint32_t numVertices, const std::vector<std::pair<BufferInfo, BufferRef>> &vertexBuffers, geom::Primitive primitiveType = geom::Primitive::TRIANGLES );
	Mesh( uint32_t numVertices, const void* vertexData, uint32_t vertexDataSize, const BufferInfo &bufferInfo, geom::Primitive primitiveType = geom::Primitive::TRIANGLES );
	Mesh( const std::vector<std::pair<BufferInfo, BufferRef>> &vertexBuffers, uint32_t numIndices, const BufferRef &indexBuffer, VALUE_TYPE indexType = VT_UINT16, geom::Primitive primitiveType = geom::Primitive::TRIANGLES );
	Mesh( const void* vertexData, uint32_t vertexDataSize, const BufferInfo &bufferInfo, uint32_t numIndices, const void *indexData, VALUE_TYPE indexType = VT_UINT16, geom::Primitive primitiveType = geom::Primitive::TRIANGLES );

	Mesh( RenderDevice* device, const geom::Source &source );
	Mesh( RenderDevice* device, const geom::Source &source, const geom::AttribSet &requestedAttribs );
	Mesh( RenderDevice* device, const geom::Source &source, const std::vector<Mesh::BufferInfo> &bufferInfos );
	Mesh( RenderDevice* device, uint32_t numVertices, const std::vector<std::pair<BufferInfo, BufferRef>> &vertexBuffers, geom::Primitive primitiveType = geom::Primitive::TRIANGLES );
	Mesh( RenderDevice* device, uint32_t numVertices, const void* vertexData, uint32_t vertexDataSize, const BufferInfo &bufferInfo, geom::Primitive primitiveType = geom::Primitive::TRIANGLES );
	Mesh( RenderDevice* device, const std::vector<std::pair<BufferInfo, BufferRef>> &vertexBuffers, uint32_t numIndices, const BufferRef &indexBuffer, VALUE_TYPE indexType = VT_UINT16, geom::Primitive primitiveType = geom::Primitive::TRIANGLES );
	Mesh( RenderDevice* device, const void* vertexData, uint32_t vertexDataSize, const BufferInfo &bufferInfo, uint32_t numIndices, const void *indexData, VALUE_TYPE indexType = VT_UINT16, geom::Primitive primitiveType = geom::Primitive::TRIANGLES );

	//! Returns the number of vertices in the mesh
	uint32_t				getNumVertices() const { return mNumVertices; }
	//! Returns the number of indices for indexed geometry, otherwise 0
	uint32_t				getNumIndices() const { return mNumIndices; }
	//! Returns the primitive topology
	PRIMITIVE_TOPOLOGY		getPrimitiveTopology() const { return mPrimitiveTopology; }
	//! Returns the data type of the indices contained in index vbo
	VALUE_TYPE				getIndexDataType() const { return mIndexType; }
	//! Returns 0 if \a attr is not present
	uint8_t					getAttribDims( geom::Attrib attr ) const;
	//! Returns AttribSet of geom::Attribs present in the Mesh
	geom::AttribSet			getAttribs() const;

	//! Returns the Buffer containing the indices of the mesh, or a NULL for non-indexed geometry
	BufferRef						  getIndexBuffer() const { return mIndices; }
	//! Returns the vector of BufferRefs for the vertex data of the mesh
	const std::vector<BufferRef>&	  getVertexBuffers() const { return mVertexBuffers; }
	//! Returns the vector of BufferRefs for the vertex data of the mesh
	const std::vector<BufferInfo>&	  getVertexBuffersInfo() const { return mVertexBuffersInfos; }
	//! Returns the vector of BufferRefs for the vertex data of the mesh
	const std::vector<LayoutElement>& getVertexLayoutElements() const { return mVertexLayoutElements; }
	//! Builds and returns a InputLayoutDesc from the Mesh vertex LayoutElements
	InputLayoutDesc					  getInputLayoutDesc() const;

	void draw( DRAW_FLAGS flags = DRAW_FLAG_NONE ) const;
	void draw( DeviceContext* context, DRAW_FLAGS flags = DRAW_FLAG_NONE ) const;
	
protected:
	uint32_t					mNumVertices;
	uint32_t					mNumIndices;
	PRIMITIVE_TOPOLOGY			mPrimitiveTopology;
	VALUE_TYPE					mIndexType;
	RenderDevice*				mDevice;
	std::vector<BufferRef>		mVertexBuffers;
	std::vector<BufferInfo>		mVertexBuffersInfos;
	std::vector<LayoutElement>	mVertexLayoutElements;
	BufferRef					mIndices;
	
	friend class MeshGeomTarget;
};

}

namespace gx = graphics;
} // namespace cinder::graphics
