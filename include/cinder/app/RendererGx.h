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

#include "cinder/app/App.h"
#include "cinder/app/Renderer.h"

#include "cinder/graphics/platform.h"
#include "cinder/graphics/wrapper.h"

namespace cinder { namespace app {

typedef std::shared_ptr<class RendererGx>	RendererGxRef;

class CI_API RendererGx : public Renderer {
  public:
	struct CI_API Options {
	public:
		Options();
		//! Specifies the desired render device type. Might fall back to another one if not supported.
		Options& deviceType( gx::RENDER_DEVICE_TYPE type ) { mDeviceType = type; return *this; }
		//! Enables logging of an error of a given severity (or greater), such as \c gx::DEBUG_MESSAGE_SEVERITY_INFO. Default to DEBUG_MESSAGE_SEVERITY_INFO in debug mode and DEBUG_MESSAGE_SEVERITY_ERROR in release.
		Options& debugLog( gx::DEBUG_MESSAGE_SEVERITY severity ) { mDebugLogSeverity = severity; return *this; }
		//! Enables breaking on an error of a given severity (or greater), such as \c gx::DEBUG_MESSAGE_SEVERITY_ERROR. Default to DEBUG_MESSAGE_SEVERITY_FATAL_ERROR.
		Options& debugBreak( gx::DEBUG_MESSAGE_SEVERITY severity ) { mDebugBreakSeverity = severity; return *this; }
		//! Specifies the set of initial swapchain options
		Options& swapChainDesc( const gx::SwapChainDesc &swapChainDesc ) { mSwapChainDesc = swapChainDesc; return *this; }
		//! Specifies the set of initial device features
		Options& deviceFeatures( const gx::DeviceFeatures &deviceFeatures ) { mDeviceFeatures = deviceFeatures; return *this; }
		//! Specifies the number of deferred contexts to create when initializing the engine.
		Options& numDeferredContexts( uint32_t numDeferredContexts ) { mNumDeferredContexts = numDeferredContexts; return *this; }
		//! Enables validation and specifies the a validation level.
		Options& validationLevel( int validationLevel ) { mValidationLevel = validationLevel; return *this; }
		//! Enables vertical sync.
		Options& enableVerticalSync( bool enable = true ) { mVSync = enable; return *this; }

		using PrepareEngineFn = std::function<void( gx::RENDER_DEVICE_TYPE, gx::EngineCreateInfo*, gx::SwapChainDesc* )>;

		//! Specifies a function to be called once the backend has been chosen. Useful to change backend specific options at runtime.
		Options& prepareEngineFn( const PrepareEngineFn &prepareEngineFn ) { mPrepareEngineFn = prepareEngineFn; return *this; }
		//! Returns the PrepareEngine function
		const PrepareEngineFn& getPrepareEngineFn() const { return mPrepareEngineFn; }
	protected:
		gx::RENDER_DEVICE_TYPE mDeviceType;
		gx::DEBUG_MESSAGE_SEVERITY mDebugLogSeverity;
		gx::DEBUG_MESSAGE_SEVERITY mDebugBreakSeverity;
		gx::SwapChainDesc mSwapChainDesc;
		gx::DeviceFeatures mDeviceFeatures;
		PrepareEngineFn mPrepareEngineFn;
		int mValidationLevel;
		uint32_t mNumDeferredContexts;
		bool					mVSync;

		friend class CI_API RendererGx;
	};

	RendererGx( const Options &options = Options() );

	static RendererGxRef	create( const Options &options = Options() ){ return RendererGxRef( new RendererGx( options ) ); }
	RendererRef				clone() const override						{ return RendererGxRef( new RendererGx( *this ) ); }

#if defined( CINDER_MSW )
	virtual void	setup( WindowImplMsw *windowImpl, RendererRef sharedRenderer );
	virtual HWND	getHwnd() const override;
	virtual HDC		getDc() const override;
#endif
	virtual void	kill();
	virtual void	prepareToggleFullScreen();
	virtual void	finishToggleFullScreen();

	const Options&	getOptions() const { return mOptions; }

	void			startDraw() override;
	void			finishDraw() override;
	void			defaultResize() override;
	void			makeCurrentContext( bool force = false ) override;
	void			swapBuffers() override;
	Surface8u		copyWindowSurface( const Area &area, int32_t windowHeightPixels ) override;

	//! Overrides Renderer's start draw implementation for custom hooks. Only useful in advanced use cases.
	void setStartDrawFn( const std::function<void( Renderer* )>& function ) { mStartDrawFn = function; }
	//! Overrides Renderer's finish draw implementation for custom hooks. Only useful in advanced use cases.
	void setFinishDrawFn( const std::function<void( Renderer* )>& function ) { mFinishDrawFn = function; }

	void enableVerticalSync( bool enable = true ) { mVSync = enable; }
	bool isVerticalSyncEnabled() const { return mVSync; }

	gx::SwapChain*		getSwapChain() { return mSwapChain; }
	gx::TEXTURE_FORMAT	getSwapChainColorFormat() const { return mSwapChain->GetDesc().ColorBufferFormat; }
	gx::TEXTURE_FORMAT	getSwapChainDepthFormat() const { return mSwapChain->GetDesc().DepthBufferFormat; }
	gx::RenderDevice*	getRenderDevice() { return mDevice; }

	gx::DeviceContext*						 getImmediateContext() { return mImmediateContext; }
	const std::vector<gx::DeviceContextRef>& getDeferredContexts() const { return mDeferredContexts; }
	gx::DeviceContext*						 getDeferredContext( size_t index ) { return mDeferredContexts[index]; }
	size_t									 getDeferredContextsCount() const { return mDeferredContexts.size(); }

	gx::ShaderSourceInputStreamFactory* getDefaultShaderSourceStreamFactory() { return mShaderSourceInputStreamFactory; }
protected:
	void initializeDiligentEngine( const Diligent::NativeWindow* pWindow );

	Options		mOptions;

	gx::RENDER_DEVICE_TYPE				mDeviceType;
	gx::EngineFactoryRef				mEngineFactory;
	gx::RenderDeviceRef					mDevice;
	gx::DeviceContextRef				mImmediateContext;
	std::vector<gx::DeviceContextRef>	mDeferredContexts;
	gx::SwapChainRef					mSwapChain;
	gx::GraphicsAdapterInfo				mAdapterAttribs;
	std::vector<gx::DisplayModeAttribs>	mDisplayModes;

	int						mValidationLevel;
	uint32_t				mAdapterId;
	gx::ADAPTER_TYPE		mAdapterType;
	std::string				mAdapterDetailsString;
	int						mSelectedDisplayMode;
	bool					mVSync;
	bool					mFullScreenMode;
	bool					mForceNonSeprblProgs;
	uint32_t				mMaxFrameLatency = gx::SwapChainDesc{}.BufferCount;

	gx::SwapChainDesc mSwapChainInitDesc;

	gx::ShaderSourceInputStreamFactoryRef mShaderSourceInputStreamFactory;

	WindowImplMsw *mWindowImpl;

	std::function<void( Renderer* )> mStartDrawFn;
	std::function<void( Renderer* )> mFinishDrawFn;
};

inline gx::ISwapChain* getSwapChain() { return std::static_pointer_cast<RendererGx>( app::App::get()->getRenderer() )->getSwapChain(); }
inline gx::TEXTURE_FORMAT getSwapChainColorFormat() { return std::static_pointer_cast<RendererGx>( app::App::get()->getRenderer() )->getSwapChainColorFormat(); }
inline gx::TEXTURE_FORMAT getSwapChainDepthFormat() { return std::static_pointer_cast<RendererGx>( app::App::get()->getRenderer() )->getSwapChainDepthFormat(); }
inline gx::IRenderDevice* getRenderDevice() { return std::static_pointer_cast<RendererGx>( app::App::get()->getRenderer() )->getRenderDevice(); }
inline gx::IDeviceContext* getImmediateContext() { return std::static_pointer_cast<RendererGx>( app::App::get()->getRenderer() )->getImmediateContext(); }
inline const std::vector<gx::DeviceContextRef>& getDeferredContexts() { return std::static_pointer_cast<RendererGx>( app::App::get()->getRenderer() )->getDeferredContexts(); }
inline gx::DeviceContext* getDeferredContext( size_t index ) { return std::static_pointer_cast<RendererGx>( app::App::get()->getRenderer() )->getDeferredContext( index ); }
inline size_t getDeferredContextsCount() { return std::static_pointer_cast<RendererGx>( app::App::get()->getRenderer() )->getDeferredContextsCount(); }
inline gx::ShaderSourceInputStreamFactory* getDefaultShaderSourceStreamFactory() { return std::static_pointer_cast<RendererGx>( app::App::get()->getRenderer() )->getDefaultShaderSourceStreamFactory(); }

} } // namespace cinder::app
