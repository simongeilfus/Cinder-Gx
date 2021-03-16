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

#include "cinder/graphics/Mesh.h"
#include "cinder/graphics/PipelineState.h"

namespace cinder { namespace graphics {

//! A Batch is a collection of Meshes, the GraphicsPipeline used to render them and the ShaderResourceBinding object associated with it
class Batch {
public:
	Batch() = default;
	Batch( const PipelineStateRef &pipelineState );
	Batch( const GraphicsPipelineStateCreateInfo &pipelineCreateInfo );
	Batch( const Mesh &mesh, const PipelineStateRef &pipelineState );
	Batch( const Mesh &mesh, const GraphicsPipelineStateCreateInfo &pipelineCreateInfo );
	Batch( const std::vector<Mesh> &meshes, const PipelineStateRef &pipelineState );
	Batch( const std::vector<Mesh> &meshes, const GraphicsPipelineStateCreateInfo &pipelineCreateInfo );

	Batch( RenderDevice* device, const PipelineStateRef &pipelineState );
	Batch( RenderDevice* device, const GraphicsPipelineStateCreateInfo &pipelineCreateInfo );
	Batch( RenderDevice* device, const Mesh &mesh, const PipelineStateRef &pipelineState );
	Batch( RenderDevice* device, const Mesh &mesh, const GraphicsPipelineStateCreateInfo &pipelineCreateInfo );
	Batch( RenderDevice* device, const std::vector<Mesh> &meshes, const PipelineStateRef &pipelineState );
	Batch( RenderDevice* device, const std::vector<Mesh> &meshes, const GraphicsPipelineStateCreateInfo &pipelineCreateInfo );

	//! Binds a static shader resource variable to Batch's ShaderResourceBinding object
	void setStaticVariable( SHADER_TYPE shaderType, const char* name, IDeviceObject* pObject );
	//! Binds a static shader resource variable to Batch's ShaderResourceBinding object
	void setStaticVariable( SHADER_TYPE shaderType, uint32_t index, IDeviceObject* pObject );

    //! Returns the number of static shader resource variables.
    uint32_t getStaticVariableCount( SHADER_TYPE shaderType ) const;
	//! Returns static shader resource variable. If the variable is not found, returns nullptr.
	ShaderResourceVariable* getStaticVariable( SHADER_TYPE shaderType, const char* name );
	//! Returns static shader resource variable by its index.
	ShaderResourceVariable* getStaticVariable( SHADER_TYPE shaderType, uint32_t index );

	//! Binds a  shader resource variable to Batch's ShaderResourceBinding object
	void setVariable( SHADER_TYPE shaderType, const char* name, IDeviceObject* pObject );
	//! Binds a  shader resource variable to Batch's ShaderResourceBinding object
	void setVariable( SHADER_TYPE shaderType, uint32_t index, IDeviceObject* pObject );

	//! Returns the number of  shader resource variables.
	uint32_t getVariableCount( SHADER_TYPE shaderType );
	//! Returns  shader resource variable. If the variable is not found, returns nullptr.
	ShaderResourceVariable* getVariable( SHADER_TYPE shaderType, const char* name );
	//! Returns  shader resource variable by its index.
	ShaderResourceVariable* getVariable( SHADER_TYPE shaderType, uint32_t index );

	//! Returns the batch's shader resource binding object 
	ShaderResourceBindingRef	getShaderResourceBinding();
	//! Returns the batch's vector of meshes
	const std::vector<Mesh>&	getMeshes() const { return mMeshes; }
	//! Returns the batch's PipelineState object
	PipelineStateRef			getPipelineState() const { return mPso; }

	void draw();
	void draw( DeviceContext* context );
	
protected:
	RenderDevice*				mDevice;
	std::vector<Mesh>			mMeshes;
	PipelineStateRef			mPso;
	ShaderResourceBindingRef	mSrb;
	friend class Device;
};

}

namespace gx = graphics;
} // namespace cinder::graphics
