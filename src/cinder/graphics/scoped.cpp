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

#include "cinder/graphics/scoped.h"
#include "cinder/app/RendererGx.h"

using namespace std;
using namespace ci::app;

namespace cinder { namespace graphics {

ScopedDebugGroup::ScopedDebugGroup( DeviceContext* context, const std::string &name, const float* pColor )
	: mCtx( context )
{
	mCtx->BeginDebugGroup( name.c_str(), pColor );
}

ScopedDebugGroup::ScopedDebugGroup( DeviceContext* context, const char* name, const float* pColor )
	: ScopedDebugGroup( context, string( name ), pColor )
{
}

ScopedDebugGroup::ScopedDebugGroup( const std::string &name, const float* pColor )
	: ScopedDebugGroup( app::getImmediateContext(), name, pColor )
{
}

ScopedDebugGroup::ScopedDebugGroup( const char* name, const float* pColor )
	: ScopedDebugGroup( app::getImmediateContext(), string( name ), pColor )
{
}

ScopedDebugGroup::~ScopedDebugGroup()
{
	mCtx->EndDebugGroup();
}

}} // namespace cinder::graphics