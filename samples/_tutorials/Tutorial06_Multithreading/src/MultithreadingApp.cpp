#include "cinder/app/App.h"
#include "cinder/app/RendererGx.h"

#include "cinder/graphics/wrapper.h"
#include "cinder/graphics/Buffer.h"
#include "cinder/graphics/DeviceContext.h"
#include "cinder/graphics/PipelineState.h"
#include "cinder/graphics/Shader.h"
#include "cinder/graphics/Texture.h"

#include "cinder/CinderDiligentImGui.h"

#include "TexturedCube.h"
#include "Utils.h"
#include "DiligentCore/Common/interface/ThreadSignal.hpp"

#include <random>

using namespace ci;
using namespace ci::app;
using namespace std;

namespace ui = ImGui;

struct InstanceData {
    glm::mat4 matrix;
    int       textureInd;
};

class MultithreadingApp : public App {
public:
    void setup() override;
    void update() override;
    void draw() override;

    void createPipelineState( std::vector<gx::StateTransitionDesc> &barriers );
    void populateInstanceData();
    void loadTextures( std::vector<gx::StateTransitionDesc> &barriers );

    void startWorkerThreads( size_t NumThreads );
    void stopWorkerThreads();

    void renderSubset( gx::DeviceContext* pCtx, uint32_t subset );

    static void workerThreadFunc( MultithreadingApp* app, uint32_t threadNum );
    static void prepareEngine( gx::RENDER_DEVICE_TYPE, gx::EngineCreateInfo*, gx::SwapChainDesc* );

    void updateUI();

    ThreadingTools::Signal   mRenderSubsetSignal;
    ThreadingTools::Signal   mExecuteCommandListsSignal;
    ThreadingTools::Signal   mGotoNextFrameSignal;
    std::atomic_int          mNumThreadsCompleted;
    std::atomic_int          mNumThreadsReady;
    std::vector<std::thread> mWorkerThreads;

    std::vector<gx::CommandListRef> mCmdLists;
    std::vector<gx::CommandList*>   mCmdListPtrs;

    gx::PipelineStateRef         mPipelineState;
    gx::BufferRef                mCubeVertexBuffer;
    gx::BufferRef                mCubeIndexBuffer;
    gx::BufferRef                mInstanceBuffer;
    gx::BufferRef                mVSConstants;
    gx::BufferRef                mInstanceConstants;

    static constexpr int sNumTextures = 4;

    gx::ShaderResourceBindingRef mSRB[sNumTextures];
    gx::TextureViewRef           mTextureSRV[sNumTextures];

    int mMaxThreads = 8;
    int mNumWorkerThreads = 4;

    std::vector<InstanceData>    mInstanceData;

    mat4                         mViewProjMatrix;
    mat4                         mRotationMatrix;

    int                  mGridSize = 5;
    static constexpr int sMaxGridSize = 32;
    static constexpr int sMaxInstances = sMaxGridSize * sMaxGridSize * sMaxGridSize;
};

void MultithreadingApp::setup()
{
    ui::DiligentInitialize();
    disableFrameRate();

    mMaxThreads = static_cast<int>( getDeferredContextsCount() );
    mNumWorkerThreads = std::min( 4, mMaxThreads );

    std::vector<gx::StateTransitionDesc> barriers;
    createPipelineState( barriers );

    // Load textured cube
    mCubeVertexBuffer = TexturedCube::createVertexBuffer( getRenderDevice() );
    mCubeIndexBuffer = TexturedCube::createIndexBuffer( getRenderDevice() );
    // Explicitly transition vertex and index buffers to required states
    barriers.emplace_back( mCubeVertexBuffer, gx::RESOURCE_STATE_UNKNOWN, gx::RESOURCE_STATE_VERTEX_BUFFER, gx::STATE_TRANSITION_FLAG_UPDATE_STATE );
    barriers.emplace_back( mCubeIndexBuffer, gx::RESOURCE_STATE_UNKNOWN, gx::RESOURCE_STATE_INDEX_BUFFER, gx::STATE_TRANSITION_FLAG_UPDATE_STATE );
    loadTextures( barriers );

    // Execute all barriers
    gx::transitionResourceStates( static_cast<uint32_t>( barriers.size() ), barriers.data() );

    populateInstanceData();

    startWorkerThreads( mNumWorkerThreads );
}

void MultithreadingApp::prepareEngine( gx::RENDER_DEVICE_TYPE deviceType, gx::EngineCreateInfo* engineCreateInfo, gx::SwapChainDesc* swapChainDesc ) 
{
    engineCreateInfo->NumDeferredContexts = std::max( std::thread::hardware_concurrency() - 1, 2u );
#if VULKAN_SUPPORTED
    if( deviceType == gx::RENDER_DEVICE_TYPE_VULKAN ) {
        gx::EngineVkCreateInfo* vkAttrs = static_cast<gx::EngineVkCreateInfo*>( engineCreateInfo );
        vkAttrs->DynamicHeapSize = 26 << 20; // Enough space for 32x32x32x256 bytes allocations for 3 frames
    }
#endif
}

void MultithreadingApp::createPipelineState( std::vector<gx::StateTransitionDesc> &barriers )
{
    // Pipeline state object encompasses configuration of all GPU stages
    mPipelineState = TexturedCube::createPipelineState( getRenderDevice(), getSwapChainColorFormat(), getSwapChainDepthFormat(), getAssetPath( "cube.vsh" ), getAssetPath( "cube.psh" ) );

    // Create dynamic uniform buffer that will store our transformation matrix
    // Dynamic buffers can be frequently updated by the CPU
    mVSConstants = gx::createBuffer( gx::BufferDesc()
        .name( "VS constants CB" )
        .size( sizeof( mat4 ) * 2 )
        .usage( gx::USAGE_DYNAMIC )
        .bindFlags( gx::BIND_UNIFORM_BUFFER )
        .cpuAccessFlags( gx::CPU_ACCESS_WRITE )
    );
    mInstanceConstants = gx::createBuffer( gx::BufferDesc()
        .name( "Instance constants CB" )
        .size( sizeof( mat4 ) )
        .usage( gx::USAGE_DYNAMIC )
        .bindFlags( gx::BIND_UNIFORM_BUFFER )
        .cpuAccessFlags( gx::CPU_ACCESS_WRITE )
    );

    // Explicitly transition the buffers to RESOURCE_STATE_CONSTANT_BUFFER state
    barriers.emplace_back( mVSConstants, gx::RESOURCE_STATE_UNKNOWN, gx::RESOURCE_STATE_CONSTANT_BUFFER, gx::STATE_TRANSITION_FLAG_UPDATE_STATE );
    barriers.emplace_back( mInstanceConstants, gx::RESOURCE_STATE_UNKNOWN, gx::RESOURCE_STATE_CONSTANT_BUFFER, gx::STATE_TRANSITION_FLAG_UPDATE_STATE );

    // Since we did not explcitly specify the type for 'Constants' and 'InstanceData' variables,
    // default type (SHADER_RESOURCE_VARIABLE_TYPE_STATIC) will be used. Static variables
    // never change and are bound directly to the pipeline state object.
    mPipelineState->GetStaticVariableByName( gx::SHADER_TYPE_VERTEX, "Constants" )->Set( mVSConstants );
    mPipelineState->GetStaticVariableByName( gx::SHADER_TYPE_VERTEX, "InstanceData" )->Set( mInstanceConstants );
}

void MultithreadingApp::loadTextures( std::vector<gx::StateTransitionDesc> &barriers )
{
    // Load textures
    for( int tex = 0; tex < sNumTextures; ++tex ) {
        // Load current texture
        std::stringstream fileNameSS;
        fileNameSS << "DGLogo" << tex << ".png";
        std::string fileName = fileNameSS.str();
        gx::TextureRef srcTex = TexturedCube::loadTexture( getRenderDevice(), getAssetPath( fileName ) );
        // Get shader resource view from the texture
        mTextureSRV[tex] = srcTex->GetDefaultView( gx::TEXTURE_VIEW_SHADER_RESOURCE );
        // Transition textures to shader resource state
        barriers.emplace_back( srcTex, gx::RESOURCE_STATE_UNKNOWN, gx::RESOURCE_STATE_SHADER_RESOURCE, true );
    }

    // Set texture SRV in the SRB
    for( int tex = 0; tex < sNumTextures; ++tex ) {
        // Create one Shader Resource Binding for every texture
        // http://diligentgraphics.com/2016/03/23/resource-binding-model-in-diligent-engine-2-0/
        mPipelineState->CreateShaderResourceBinding( &mSRB[tex], true );
        mSRB[tex]->GetVariableByName( gx::SHADER_TYPE_PIXEL, "g_Texture" )->Set( mTextureSRV[tex] );
    }
}

void MultithreadingApp::updateUI()
{
    ui::SetNextWindowPos( ImVec2( 10, 10 ), ImGuiCond_FirstUseEver );
    if( ui::Begin( "Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) ) {
        if( ui::SliderInt( "Grid Size", &mGridSize, 1, 32 ) ) {
            populateInstanceData();
        }
        {
            //ui::ScopedDisabler Disable( m_MaxThreads == 0 );
            if( ui::SliderInt( "Worker Threads", &mNumWorkerThreads, 0, mMaxThreads ) ) {
                stopWorkerThreads();
                startWorkerThreads( mNumWorkerThreads );
            }
        }
    }

    ui::End();
}

void MultithreadingApp::populateInstanceData()
{
    mInstanceData.resize( mGridSize * mGridSize * mGridSize );

    float fGridSize = static_cast<float>( mGridSize );

    std::mt19937 gen; // Standard mersenne_twister_engine. Use default seed
                      // to generate consistent distribution.

    std::uniform_real_distribution<float> scale_distr( 0.3f, 1.0f );
    std::uniform_real_distribution<float> offset_distr( -0.15f, +0.15f );
    std::uniform_real_distribution<float> rot_distr( -glm::pi<float>(), +glm::pi<float>() );
    std::uniform_int_distribution<int32_t>  tex_distr( 0, sNumTextures - 1 );

    float BaseScale = 0.6f / fGridSize;
    int   instId = 0;
    for( int x = 0; x < mGridSize; ++x ) {
        for( int y = 0; y < mGridSize; ++y ) {
            for( int z = 0; z < mGridSize; ++z ) {
                // Add random offset from central position in the grid
                float xOffset = 2.f * ( x + 0.5f + offset_distr( gen ) ) / fGridSize - 1.f;
                float yOffset = 2.f * ( y + 0.5f + offset_distr( gen ) ) / fGridSize - 1.f;
                float zOffset = 2.f * ( z + 0.5f + offset_distr( gen ) ) / fGridSize - 1.f;
                // Random scale
                float scale = BaseScale * scale_distr( gen );
                // Random rotation
                mat4 rotation = glm::rotate( rot_distr( gen ), vec3( 1.0f, 0.0f, 0.0f ) ) * glm::rotate( rot_distr( gen ), vec3( 0.0f, 1.0f, 0.0f ) ) * glm::rotate( rot_distr( gen ), vec3( 0.0f, 0.0f, 1.0f ) );
                // Combine rotation, scale and translation
                mat4 matrix = glm::translate( vec3( xOffset, yOffset, zOffset ) ) * glm::scale( vec3( scale ) ) * rotation;

                auto&    currInst = mInstanceData[instId++];
                currInst.matrix = matrix;
                // Texture array index
                currInst.textureInd = tex_distr( gen );
            }
        }
    }
}

void MultithreadingApp::startWorkerThreads( size_t numThreads )
{
    mWorkerThreads.resize( numThreads );
    for( uint32_t t = 0; t < mWorkerThreads.size(); ++t ) {
        mWorkerThreads[t] = std::thread( workerThreadFunc, this, t );
    }
    mCmdLists.resize( numThreads );
}

void MultithreadingApp::stopWorkerThreads()
{
    mRenderSubsetSignal.Trigger( true, -1 );

    for( auto& thread : mWorkerThreads ) {
        thread.join();
    }
    mRenderSubsetSignal.Reset();
    mWorkerThreads.clear();
    mCmdLists.clear();
}

void MultithreadingApp::workerThreadFunc( MultithreadingApp* app, uint32_t threadNum )
{
    // Every thread should use its own deferred context
    gx::DeviceContext* deferredCtx = app::getDeferredContext( threadNum );
    const int numWorkerThreads = static_cast<int>( app->mWorkerThreads.size() );
    for( ;; ) {
        // Wait for the signal
        auto SignaledValue = app->mRenderSubsetSignal.Wait( true, numWorkerThreads );
        if( SignaledValue < 0 )
            return;

        // Render current subset using the deferred context
        app->renderSubset( deferredCtx, 1 + threadNum );

        // Finish command list
        gx::CommandListRef cmdList;
        deferredCtx->FinishCommandList( &cmdList );
        app->mCmdLists[threadNum] = cmdList;

        {
            // Atomically increment the number of completed threads
            const auto NumThreadsCompleted = app->mNumThreadsCompleted.fetch_add( 1 ) + 1;
            if( NumThreadsCompleted == numWorkerThreads )
                app->mExecuteCommandListsSignal.Trigger();
        }

        app->mGotoNextFrameSignal.Wait( true, numWorkerThreads );

        // Call FinishFrame() to release dynamic resources allocated by deferred contexts
        // IMPORTANT: we must wait until the command lists are submitted for execution
        //            because FinishFrame() invalidates all dynamic resources.
        // IMPORTANT: In Metal backend FinishFrame must be called from the same
        //            thread that issued rendering commands.
        deferredCtx->FinishFrame();

        app->mNumThreadsReady.fetch_add( 1 );
        // We must wait until all threads reach this point, because
        // mGotoNextFrameSignal must be unsignaled before we proceed to
        // RenderSubsetSignal to avoid one thread going through the loop twice in
        // a row.
        while( app->mNumThreadsReady.load() < numWorkerThreads )
            std::this_thread::yield();
        VERIFY_EXPR( !app->mGotoNextFrameSignal.IsTriggered() );
    }
}

void MultithreadingApp::renderSubset( gx::DeviceContext* pCtx, uint32_t subset )
{
    // Deferred contexts start in default state. We must bind everything to the context.
    // Render targets are set and transitioned to correct states by the main thread, here we only verify the states.
    auto* pRTV = getSwapChain()->GetCurrentBackBufferRTV();
    pCtx->SetRenderTargets( 1, &pRTV, getSwapChain()->GetDepthBufferDSV(), gx::RESOURCE_STATE_TRANSITION_MODE_VERIFY );

    {
        // Map the buffer and write current world-view-projection matrix

        // Since this is a dynamic buffer, it must be mapped in every context before
        // it can be used even though the matrices are the same.
        gx::MapHelper<mat4> CBConstants( pCtx, mVSConstants, gx::MAP_WRITE, gx::MAP_FLAG_DISCARD );
        CBConstants[0] = glm::transpose( mViewProjMatrix );
        CBConstants[1] = glm::transpose( mRotationMatrix );
    }

    // Bind vertex and index buffers. This must be done for every context
    uint64_t   offsets[] = { 0, 0 };
    gx::Buffer* buffers[] = { mCubeVertexBuffer };
    pCtx->SetVertexBuffers( 0, _countof( buffers ), buffers, offsets, gx::RESOURCE_STATE_TRANSITION_MODE_VERIFY, gx::SET_VERTEX_BUFFERS_FLAG_RESET );
    pCtx->SetIndexBuffer( mCubeIndexBuffer, 0, gx::RESOURCE_STATE_TRANSITION_MODE_VERIFY );

    gx::DrawIndexedAttribs DrawAttrs;     // This is an indexed draw call
    DrawAttrs.IndexType = gx::VT_UINT32; // Index type
    DrawAttrs.NumIndices = 36;
    DrawAttrs.Flags = gx::DRAW_FLAG_VERIFY_ALL;

    // Set the pipeline state
    pCtx->SetPipelineState( mPipelineState );
    uint32_t NumSubsets = uint32_t{ 1 } + static_cast<uint32_t>( mWorkerThreads.size() );
    uint32_t NumInstances = static_cast<uint32_t>( mInstanceData.size() );
    uint32_t SusbsetSize = NumInstances / NumSubsets;
    uint32_t StartInst = SusbsetSize * subset;
    uint32_t EndInst = ( subset < NumSubsets - 1 ) ? SusbsetSize * ( subset + 1 ) : NumInstances;
    for( size_t inst = StartInst; inst < EndInst; ++inst ) {
        const auto& CurrInstData = mInstanceData[inst];
        // Shader resources have been explicitly transitioned to correct states, so
        // RESOURCE_STATE_TRANSITION_MODE_TRANSITION mode is not needed.
        // Instead, we use RESOURCE_STATE_TRANSITION_MODE_VERIFY mode to
        // verify that all resources are in correct states. This mode only has effect
        // in debug and development builds.
        pCtx->CommitShaderResources( mSRB[CurrInstData.textureInd], gx::RESOURCE_STATE_TRANSITION_MODE_VERIFY );

        {
            // Map the buffer and write current world-view-projection matrix
            gx::MapHelper<mat4> InstData( pCtx, mInstanceConstants, gx::MAP_WRITE, gx::MAP_FLAG_DISCARD );
            if( InstData == nullptr ) {
                LOG_ERROR_MESSAGE( "Failed to map instance data buffer" );
                break;
            }
            *InstData = glm::transpose( CurrInstData.matrix );
        }

        pCtx->DrawIndexed( DrawAttrs );
    }
}

void MultithreadingApp::update()
{
    updateUI();
    utils::updateWindowTitle();

    // Set cube view matrix
    mat4 view = glm::lookAt( glm::vec3( 0.0f, 2.0f, 4.0f ), vec3( 0.0f ), vec3( 0.0f, 1.0f, 0.0f ) );
    // Get projection matrix adjusted to the current screen orientation
    mat4 proj = glm::perspective( glm::pi<float>() / 4.0f, getWindowAspectRatio(), 0.1f, 100.f );
    // Compute world-view-projection matrix
    mViewProjMatrix = proj * view;
    // Global rotation matrix
    mRotationMatrix = glm::rotate( -static_cast<float>( getElapsedSeconds() ) * 0.25f, vec3( 1.0f, 0.0f, 0.0f ) ) * glm::rotate( static_cast<float>( getElapsedSeconds() ) * 1.0f, vec3( 0.0f, 1.0f, 0.0f ) );
}

void MultithreadingApp::draw()
{
    // Clear the back buffer
    gx::clear( ColorA( 0.350f, 0.350f, 0.350f, 1.0f ) );


    if( ! mWorkerThreads.empty() ) {
        mNumThreadsCompleted.store( 0 );
        mRenderSubsetSignal.Trigger( true );
    }

    renderSubset( getImmediateContext(), 0 );

    if( ! mWorkerThreads.empty() ) {
        mExecuteCommandListsSignal.Wait( true, 1 );

        mCmdListPtrs.resize( mCmdLists.size() );
        for( uint32_t i = 0; i < mCmdLists.size(); ++i )
            mCmdListPtrs[i] = mCmdLists[i];

        gx::executeCommandLists( static_cast<uint32_t>( mCmdListPtrs.size() ), mCmdListPtrs.data() );

        for( auto& cmdList : mCmdLists ) {
            // Release command lists now to release all outstanding references.
            // In d3d11 mode, command lists hold references to the swap chain's back buffer
            // that cause swap chain resize to fail.
            cmdList.Release();
        }

        mNumThreadsReady.store( 0 );
        mGotoNextFrameSignal.Trigger( true );
    }
}

CINDER_APP( MultithreadingApp, RendererGx( RendererGx::Options().prepareEngineFn( MultithreadingApp::prepareEngine ) ) )
