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

#include "cinder/app/RendererGx.h"
#include "cinder/Log.h"
#include "cinder/Breakpoint.h"

#if defined( CINDER_MSW )
#include "cinder/app/msw/AppImplMsw.h"
#endif
#include "DiligentCore/Graphics/GraphicsEngineVulkan/interface/EngineFactoryVk.h"
#include "DiligentCore/Graphics/GraphicsEngineD3D11/interface/EngineFactoryD3D11.h"
#include "DiligentCore/Graphics/GraphicsEngineD3D12/interface/EngineFactoryD3D12.h"
#include "DiligentCore/Graphics/GraphicsEngineOpenGL/interface/EngineFactoryOpenGL.h"

using namespace std;
using namespace Diligent;
	
namespace cinder { namespace app {

RendererGx::Options::Options()
    : mDeviceType( gx::RENDER_DEVICE_TYPE_UNDEFINED ),
#if ! defined( NDEBUG )
    mDebugLogSeverity( gx::DEBUG_MESSAGE_SEVERITY_INFO ),
#else
    mDebugLogSeverity( gx::DEBUG_MESSAGE_SEVERITY_ERROR ),
#endif
    mDebugBreakSeverity( gx::DEBUG_MESSAGE_SEVERITY_FATAL_ERROR ),
    mValidationLevel( -1 ),
    mVSync( false ),
    mNumDeferredContexts( 0 )
{
}

static gx::DEBUG_MESSAGE_SEVERITY sDebugLogSeverity;
static gx::DEBUG_MESSAGE_SEVERITY sDebugBreakSeverity;

RendererGx::RendererGx( const RendererGx::Options &options )
	: Renderer(), 
    mOptions( options ), 
    mDeviceType( options.mDeviceType ),
    mValidationLevel( options.mValidationLevel ),
    mAdapterId( 0 ),
    mAdapterType( gx::ADAPTER_TYPE_UNKNOWN ),
    mSelectedDisplayMode( 0 ),
    mVSync( options.mVSync ),
    mFullScreenMode( false ),
    mForceNonSeprblProgs( false ),
    mMaxFrameLatency( gx::SwapChainDesc{}.BufferCount ),
    mSwapChainInitDesc( options.mSwapChainDesc )
{
    sDebugLogSeverity = options.mDebugLogSeverity;
    sDebugBreakSeverity = options.mDebugBreakSeverity;
}

void RendererGx::setup( WindowImplMsw *windowImpl, RendererRef sharedRenderer )
{
	mWindowImpl = windowImpl;

#if defined( CINDER_MSW )
    if( mDeviceType == gx::RENDER_DEVICE_TYPE_UNDEFINED ) {
        mDeviceType = gx::RENDER_DEVICE_TYPE_VULKAN;
    }
#endif

    unique_ptr<NativeWindow> pWindow = make_unique<NativeWindow>( mWindowImpl->getHwnd() );
    initializeDiligentEngine( pWindow.get() );  

    string searchDirectories = app::getAppPath().string() + ";";
    for( const auto &p : app::getAssetDirectories() ) {
        searchDirectories += p.string() + ";";
    }
    mEngineFactory->CreateDefaultShaderSourceStreamFactory( searchDirectories.c_str(), &mShaderSourceInputStreamFactory );
}

namespace {
    ci::log::Level severityToLevel( DEBUG_MESSAGE_SEVERITY severity )
    {
        log::Level level = log::LEVEL_INFO;
        switch( severity ) {
            case DEBUG_MESSAGE_SEVERITY_INFO: level = log::LEVEL_INFO; break;
            case DEBUG_MESSAGE_SEVERITY_WARNING: level = log::LEVEL_WARNING; break;
            case DEBUG_MESSAGE_SEVERITY_ERROR: level = log::LEVEL_ERROR; break;
            case DEBUG_MESSAGE_SEVERITY_FATAL_ERROR: level = log::LEVEL_FATAL; break;
        }
        return level;
    }
    void debugMessageCallback( enum DEBUG_MESSAGE_SEVERITY severity, const Diligent::Char* message, const Diligent::Char* function, const Diligent::Char* file, int line )
    {
        if( severity >= sDebugLogSeverity ) {
            log::Level    level    = severityToLevel( severity );
            log::Location location = log::Location( function ? function : "", file ? file : "", line );
            log::Entry( level, location ) << message;
            /*Diligent::String formatedMessage = BasicPlatformDebug::FormatDebugMessage( severity, message, function, file, line );
            app::console() << formatedMessage;
            app::console().flush();*/
        }
        if( severity >= sDebugBreakSeverity ) {
            CI_BREAKPOINT();
        }
    }
} // anonymous namespace

void RendererGx::initializeDiligentEngine( const Diligent::NativeWindow* pWindow )
{
    //if( mScreenCaptureInfo.AllowCapture )
    //    mSwapChainInitDesc.Usage |= SWAP_CHAIN_USAGE_COPY_SOURCE;

#if PLATFORM_MACOS
    // We need at least 3 buffers on Metal to avoid massive
    // peformance degradation in full screen mode.
    // https://github.com/KhronosGroup/MoltenVK/issues/808
    mSwapChainInitDesc.BufferCount = 3;
#endif

    std::vector<IDeviceContext*> ppContexts;
    switch( mDeviceType ) {
#if D3D11_SUPPORTED
    case RENDER_DEVICE_TYPE_D3D11:
    {
        EngineD3D11CreateInfo EngineCI;
        EngineCI.DebugMessageCallback = debugMessageCallback;
        EngineCI.Features = mOptions.mDeviceFeatures;
        EngineCI.NumDeferredContexts = mOptions.mNumDeferredContexts;

#    ifdef DILIGENT_DEVELOPMENT
        EngineCI.DebugFlags |=
            D3D11_DEBUG_FLAG_CREATE_DEBUG_DEVICE |
            D3D11_DEBUG_FLAG_VERIFY_COMMITTED_SHADER_RESOURCES;
#    endif
#    ifdef DILIGENT_DEBUG
        EngineCI.DebugFlags |= D3D11_DEBUG_FLAG_VERIFY_COMMITTED_RESOURCE_RELEVANCE;
#    endif

        if( mValidationLevel >= 1 ) {
            EngineCI.DebugFlags =
                D3D11_DEBUG_FLAG_CREATE_DEBUG_DEVICE |
                D3D11_DEBUG_FLAG_VERIFY_COMMITTED_SHADER_RESOURCES |
                D3D11_DEBUG_FLAG_VERIFY_COMMITTED_RESOURCE_RELEVANCE;
        }
        else if( mValidationLevel == 0 ) {
            EngineCI.DebugFlags = D3D11_DEBUG_FLAG_NONE;
        }

        if( const auto &fn = mOptions.getPrepareEngineFn() ) {
            fn( mDeviceType, &EngineCI, &mSwapChainInitDesc );
        }

#    if ENGINE_DLL
            // Load the dll and import GetEngineFactoryD3D11() function
        auto GetEngineFactoryD3D11 = LoadGraphicsEngineD3D11();
#    endif
        auto* pFactoryD3D11 = GetEngineFactoryD3D11();
        mEngineFactory = pFactoryD3D11;
        Uint32 NumAdapters = 0;
        pFactoryD3D11->EnumerateAdapters( EngineCI.MinimumFeatureLevel, NumAdapters, 0 );
        std::vector<GraphicsAdapterInfo> Adapters( NumAdapters );
        if( NumAdapters > 0 ) {
            pFactoryD3D11->EnumerateAdapters( EngineCI.MinimumFeatureLevel, NumAdapters, Adapters.data() );
        }
        else {
            LOG_ERROR_AND_THROW( "Failed to find Direct3D11-compatible hardware adapters" );
        }

        if( mAdapterType == ADAPTER_TYPE_SOFTWARE ) {
            for( Uint32 i = 0; i < Adapters.size(); ++i ) {
                if( Adapters[i].Type == mAdapterType ) {
                    mAdapterId = i;
                    LOG_INFO_MESSAGE( "Found software adapter '", Adapters[i].Description, "'" );
                    break;
                }
            }
        }

        mAdapterAttribs = Adapters[mAdapterId];
        if( mAdapterType != ADAPTER_TYPE_SOFTWARE ) {
            Uint32 NumDisplayModes = 0;
            pFactoryD3D11->EnumerateDisplayModes( EngineCI.MinimumFeatureLevel, mAdapterId, 0, TEX_FORMAT_RGBA8_UNORM_SRGB, NumDisplayModes, nullptr );
            mDisplayModes.resize( NumDisplayModes );
            pFactoryD3D11->EnumerateDisplayModes( EngineCI.MinimumFeatureLevel, mAdapterId, 0, TEX_FORMAT_RGBA8_UNORM_SRGB, NumDisplayModes, mDisplayModes.data() );
        }

        EngineCI.AdapterId = mAdapterId;
        ppContexts.resize( 1 + EngineCI.NumDeferredContexts );
        pFactoryD3D11->CreateDeviceAndContextsD3D11( EngineCI, &mDevice, ppContexts.data() );
        if( !mDevice ) {
            LOG_ERROR_AND_THROW( "Unable to initialize Diligent Engine in Direct3D11 mode. The API may not be available, "
                "or required features may not be supported by this GPU/driver/OS version." );
        }

        if( pWindow != nullptr )
            pFactoryD3D11->CreateSwapChainD3D11( mDevice, ppContexts[0], mSwapChainInitDesc, FullScreenModeDesc{}, *pWindow, &mSwapChain );
    }
    break;
#endif

#if D3D12_SUPPORTED
    case RENDER_DEVICE_TYPE_D3D12:
    {
        EngineD3D12CreateInfo EngineCI;
        EngineCI.DebugMessageCallback = debugMessageCallback;
        EngineCI.Features = mOptions.mDeviceFeatures;
        EngineCI.NumDeferredContexts = mOptions.mNumDeferredContexts;

#    ifdef DILIGENT_DEVELOPMENT
        EngineCI.EnableDebugLayer = true;
#    endif
        if( mValidationLevel >= 1 ) {
            EngineCI.EnableDebugLayer = true;
            if( mValidationLevel >= 2 )
                EngineCI.EnableGPUBasedValidation = true;
        }
        else if( mValidationLevel == 0 ) {
            EngineCI.EnableDebugLayer = false;
        }

        if( const auto &fn = mOptions.getPrepareEngineFn() ) {
            fn( mDeviceType, &EngineCI, &mSwapChainInitDesc );
        }

#    if ENGINE_DLL
            // Load the dll and import GetEngineFactoryD3D12() function
        auto GetEngineFactoryD3D12 = LoadGraphicsEngineD3D12();
#    endif
        auto* pFactoryD3D12 = GetEngineFactoryD3D12();
        if( !pFactoryD3D12->LoadD3D12() ) {
            LOG_ERROR_AND_THROW( "Failed to load Direct3D12" );
        }

        mEngineFactory = pFactoryD3D12;
        Uint32 NumAdapters = 0;
        pFactoryD3D12->EnumerateAdapters( EngineCI.MinimumFeatureLevel, NumAdapters, 0 );
        std::vector<GraphicsAdapterInfo> Adapters( NumAdapters );
        if( NumAdapters > 0 ) {
            pFactoryD3D12->EnumerateAdapters( EngineCI.MinimumFeatureLevel, NumAdapters, Adapters.data() );
        }
        else {
#    if D3D11_SUPPORTED
            LOG_ERROR_MESSAGE( "Failed to find Direct3D12-compatible hardware adapters. Attempting to initialize the engine in Direct3D11 mode." );
            mDeviceType = gx::RENDER_DEVICE_TYPE_D3D11;
            initializeDiligentEngine( pWindow );
            return;
#    else
            LOG_ERROR_AND_THROW( "Failed to find Direct3D12-compatible hardware adapters." );
#    endif
        }

        if( mAdapterType == ADAPTER_TYPE_SOFTWARE ) {
            for( Uint32 i = 0; i < Adapters.size(); ++i ) {
                if( Adapters[i].Type == mAdapterType ) {
                    mAdapterId = i;
                    LOG_INFO_MESSAGE( "Found software adapter '", Adapters[i].Description, "'" );
                    break;
                }
            }
        }

        mAdapterAttribs = Adapters[mAdapterId];
        if( mAdapterType != ADAPTER_TYPE_SOFTWARE ) {
            Uint32 NumDisplayModes = 0;
            pFactoryD3D12->EnumerateDisplayModes( EngineCI.MinimumFeatureLevel, mAdapterId, 0, TEX_FORMAT_RGBA8_UNORM_SRGB, NumDisplayModes, nullptr );
            mDisplayModes.resize( NumDisplayModes );
            pFactoryD3D12->EnumerateDisplayModes( EngineCI.MinimumFeatureLevel, mAdapterId, 0, TEX_FORMAT_RGBA8_UNORM_SRGB, NumDisplayModes, mDisplayModes.data() );
        }

        EngineCI.AdapterId = mAdapterId;
        ppContexts.resize( 1 + EngineCI.NumDeferredContexts );
        pFactoryD3D12->CreateDeviceAndContextsD3D12( EngineCI, &mDevice, ppContexts.data() );
        if( !mDevice ) {
            LOG_ERROR_AND_THROW( "Unable to initialize Diligent Engine in Direct3D12 mode. The API may not be available, "
                "or required features may not be supported by this GPU/driver/OS version." );
        }

        if( !mSwapChain && pWindow != nullptr )
            pFactoryD3D12->CreateSwapChainD3D12( mDevice, ppContexts[0], mSwapChainInitDesc, FullScreenModeDesc{}, *pWindow, &mSwapChain );
    }
    break;
#endif

#if GL_SUPPORTED || GLES_SUPPORTED
    case RENDER_DEVICE_TYPE_GL:
    case RENDER_DEVICE_TYPE_GLES:
    {
#    if !PLATFORM_MACOS
        VERIFY_EXPR( pWindow != nullptr );
#    endif
#    if EXPLICITLY_LOAD_ENGINE_GL_DLL
            // Load the dll and import GetEngineFactoryOpenGL() function
        auto GetEngineFactoryOpenGL = LoadGraphicsEngineOpenGL();
#    endif
        auto* pFactoryOpenGL = GetEngineFactoryOpenGL();
        mEngineFactory = pFactoryOpenGL;
        EngineGLCreateInfo EngineCI;
        EngineCI.DebugMessageCallback = debugMessageCallback;
        EngineCI.Features = mOptions.mDeviceFeatures;
        EngineCI.NumDeferredContexts = mOptions.mNumDeferredContexts;
        EngineCI.Window = *pWindow;

#    ifdef DILIGENT_DEVELOPMENT
        EngineCI.CreateDebugContext = true;
#    endif
        EngineCI.ForceNonSeparablePrograms = mForceNonSeprblProgs;

        if( mValidationLevel >= 1 ) {
            EngineCI.CreateDebugContext = true;
        }
        else if( mValidationLevel == 0 ) {
            EngineCI.CreateDebugContext = false;
        }

        if( const auto &fn = mOptions.getPrepareEngineFn() ) {
            fn( mDeviceType, &EngineCI, &mSwapChainInitDesc );
        }

        if( EngineCI.NumDeferredContexts != 0 ) {
            LOG_ERROR_MESSAGE( "Deferred contexts are not supported in OpenGL mode" );
            EngineCI.NumDeferredContexts = 0;
        }
        ppContexts.resize( 1 + EngineCI.NumDeferredContexts );
        pFactoryOpenGL->CreateDeviceAndSwapChainGL(
            EngineCI, &mDevice, ppContexts.data(), mSwapChainInitDesc, &mSwapChain );
        if( !mDevice ) {
            LOG_ERROR_AND_THROW( "Unable to initialize Diligent Engine in OpenGL mode. The API may not be available, "
                "or required features may not be supported by this GPU/driver/OS version." );
        }
    }
    break;
#endif

#if VULKAN_SUPPORTED
    case RENDER_DEVICE_TYPE_VULKAN:
    {
#    if EXPLICITLY_LOAD_ENGINE_VK_DLL
            // Load the dll and import GetEngineFactoryVk() function
        auto GetEngineFactoryVk = LoadGraphicsEngineVk();
#    endif
        EngineVkCreateInfo EngVkAttribs;
        EngVkAttribs.DebugMessageCallback = debugMessageCallback;
        EngVkAttribs.Features = mOptions.mDeviceFeatures;
        EngVkAttribs.NumDeferredContexts = mOptions.mNumDeferredContexts;
#    ifdef DILIGENT_DEVELOPMENT
        EngVkAttribs.EnableValidation = true;
#    endif
        if( mValidationLevel >= 1 ) {
            EngVkAttribs.EnableValidation = true;
        }
        else if( mValidationLevel == 0 ) {
            EngVkAttribs.EnableValidation = false;
        }

        if( const auto &fn = mOptions.getPrepareEngineFn() ) {
            fn( mDeviceType, &EngVkAttribs, &mSwapChainInitDesc );
        }

        ppContexts.resize( 1 + EngVkAttribs.NumDeferredContexts );
        auto* pFactoryVk = GetEngineFactoryVk();
        mEngineFactory = pFactoryVk;
        pFactoryVk->CreateDeviceAndContextsVk( EngVkAttribs, &mDevice, ppContexts.data() );
        if( !mDevice ) {
            LOG_ERROR_AND_THROW( "Unable to initialize Diligent Engine in Vulkan mode. The API may not be available, "
                "or required features may not be supported by this GPU/driver/OS version." );
        }

        if( !mSwapChain && pWindow != nullptr )
            pFactoryVk->CreateSwapChainVk( mDevice, ppContexts[0], mSwapChainInitDesc, *pWindow, &mSwapChain );
    }
    break;
#endif


#if METAL_SUPPORTED
    case RENDER_DEVICE_TYPE_METAL:
    {
        EngineMtlCreateInfo MtlAttribs;
        MtlAttribs.DebugMessageCallback = debugMessageCallback;
        MtlAttribs.Features = mOptions.mDeviceFeatures;
        MtlAttribs.NumDeferredContexts = mOptions.mNumDeferredContexts;

        if( const auto &fn = mOptions.getPrepareEngineFn() ) {
            fn( mDeviceType, &MtlAttribs, &mSwapChainInitDesc );
        }

        ppContexts.resize( 1 + MtlAttribs.NumDeferredContexts );
        auto* pFactoryMtl = GetEngineFactoryMtl();
        mEngineFactory = pFactoryMtl;
        pFactoryMtl->CreateDeviceAndContextsMtl( MtlAttribs, &mDevice, ppContexts.data() );
        if( !mDevice ) {
            LOG_ERROR_AND_THROW( "Unable to initialize Diligent Engine in Metal mode. The API may not be available, "
                "or required features may not be supported by this GPU/driver/OS version." );
        }

        if( !mSwapChain && pWindow != nullptr )
            pFactoryMtl->CreateSwapChainMtl( mDevice, ppContexts[0], mSwapChainInitDesc, *pWindow, &mSwapChain );
    }
    break;
#endif

    default:
        LOG_ERROR_AND_THROW( "Unknown device type" );
        break;
    }

    switch( mDeviceType ) {
    case gx::RENDER_DEVICE_TYPE_D3D11:  mWindowImpl->setTitle( mWindowImpl->getTitle() + " (D3D11)" );    break;
    case gx::RENDER_DEVICE_TYPE_D3D12:  mWindowImpl->setTitle( mWindowImpl->getTitle() + " (D3D12)" );    break;
    case gx::RENDER_DEVICE_TYPE_GL:     mWindowImpl->setTitle( mWindowImpl->getTitle() + " (OpenGL)" );   break;
    case gx::RENDER_DEVICE_TYPE_GLES:   mWindowImpl->setTitle( mWindowImpl->getTitle() + " (OpenGLES)" ); break;
    case gx::RENDER_DEVICE_TYPE_VULKAN: mWindowImpl->setTitle( mWindowImpl->getTitle() + " (Vulkan)" );   break;
    case gx::RENDER_DEVICE_TYPE_METAL:  mWindowImpl->setTitle( mWindowImpl->getTitle() + " (Metal)" );    break;
    default: UNEXPECTED( "Unknown/unsupported device type" );
        // clang-format on
    }

    mImmediateContext.Attach( ppContexts[0] );
    auto NumDeferredCtx = ppContexts.size() - 1;
    mDeferredContexts.resize( NumDeferredCtx );
    for( Uint32 ctx = 0; ctx < NumDeferredCtx; ++ctx )
        mDeferredContexts[ctx].Attach( ppContexts[1 + ctx] );

    //if( mScreenCaptureInfo.AllowCapture ) {
    //    if( mGoldenImgMode != GoldenImageMode::None ) {
    //        // Capture only one frame
    //        mScreenCaptureInfo.FramesToCapture = 1;
    //    }

    //    mScreenCapture.reset( new ScreenCapture( mDevice ) );
    //}
}

HWND RendererGx::getHwnd() const
{
	return mWindowImpl->getHwnd();
}

HDC RendererGx::getDc() const
{
	return mWindowImpl->getDc();
}

void RendererGx::kill()
{
}

void RendererGx::prepareToggleFullScreen()
{
}

void RendererGx::finishToggleFullScreen()
{
    if( ! mFullScreenMode ) {
        mFullScreenMode = true;
        DisplayModeAttribs DisplayMode;
        // todo
        mSwapChain->SetFullscreenMode( DisplayMode );
    }
    else {
        mFullScreenMode = false;
        mSwapChain->SetWindowedMode();
    }
}

void RendererGx::startDraw()
{
	if( mStartDrawFn )
		mStartDrawFn( this );

    if( ! mImmediateContext || ! mSwapChain )
        return;

    ITextureView* pRTV = mSwapChain->GetCurrentBackBufferRTV();
    ITextureView* pDSV = mSwapChain->GetDepthBufferDSV();
    mImmediateContext->SetRenderTargets( 1, &pRTV, pDSV, RESOURCE_STATE_TRANSITION_MODE_TRANSITION );
}

void RendererGx::makeCurrentContext( bool force )
{
}

void RendererGx::swapBuffers()
{
}

void RendererGx::finishDraw()
{
	if( mFinishDrawFn )
		mFinishDrawFn( this );
    
    if( ! mImmediateContext || ! mSwapChain )
        return;

    // Restore default render target in case the sample has changed it
    ITextureView* pRTV = mSwapChain->GetCurrentBackBufferRTV();
    ITextureView* pDSV = mSwapChain->GetDepthBufferDSV();
    mImmediateContext->SetRenderTargets( 1, &pRTV, pDSV, RESOURCE_STATE_TRANSITION_MODE_TRANSITION );

    mSwapChain->Present( mVSync ? 1 : 0 );
}

void RendererGx::defaultResize()
{
    mSwapChain->Resize( getWindowWidth(), getWindowHeight() );

    startDraw();
    finishDraw();
}

Surface	RendererGx::copyWindowSurface( const Area &area, int32_t windowHeightPixels )
{
	return Surface();
}

} } // namespace cinder::app
