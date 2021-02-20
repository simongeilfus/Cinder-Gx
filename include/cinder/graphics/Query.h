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

#include "DiligentCore/Graphics/GraphicsEngine/interface/Query.h"
#include "DiligentCore/Graphics/GraphicsTools/interface/ScopedQueryHelper.hpp"
#include "DiligentCore/Graphics/GraphicsTools/interface/DurationQueryHelper.hpp"

namespace cinder { namespace graphics {

/// Query description.
struct CI_API QueryDesc : public Diligent::QueryDesc {
	//! Specifies the Query type, see Diligent::QUERY_TYPE.
	QueryDesc& type( QUERY_TYPE type ) { Type = type; return *this; }
    //! Speficies the object's name.
	QueryDesc& name( const char* name ) { Name = name; return *this; }

	explicit QueryDesc( QUERY_TYPE type, const char* name = nullptr ) noexcept 
		: Diligent::QueryDesc( type ) 
	{
		Name = name;
	}
	QueryDesc() = default;
};

}

namespace gx = graphics;
} // namespace cinder::graphics