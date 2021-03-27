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

#include <random>

using namespace ci;
using namespace ci::app;
using namespace std;

namespace ui = ImGui;

struct InstanceData {
    glm::mat4   matrix;
    float       textureInd;
};

class TextureArrayApp : public App {
  public:
	void setup() override;
	void update() override;
	void draw() override;

    void createPipelineState();
    void createUniformBuffer();
    void createInstanceBuffer();
    void populateInstanceBuffer();
    void loadTextures();

    void updateUI();

    gx::PipelineStateRef         mPipelineState;
    gx::ShaderResourceBindingRef mSRB;
    gx::BufferRef                mCubeVertexBuffer;
    gx::BufferRef                mCubeIndexBuffer;
    gx::BufferRef                mInstanceBuffer;
    gx::BufferRef                mVSConstants;

    gx::TextureViewRef           mTextureSRV;
    mat4                         mWorldViewProjMatrix;
    mat4                         mRotationMatrix;

    int                  mGridSize = 5;
    static constexpr int sMaxGridSize = 32;
    static constexpr int sMaxInstances = sMaxGridSize * sMaxGridSize * sMaxGridSize;
    static constexpr int sNumTextures = 4;
};

void TextureArrayApp::setup()
{
    ui::DiligentInitialize();
    disableFrameRate();

    createUniformBuffer();
    createPipelineState();

    // Load textured cube
    mCubeVertexBuffer = TexturedCube::createVertexBuffer( getRenderDevice() );
    mCubeIndexBuffer = TexturedCube::createIndexBuffer( getRenderDevice() );

    createInstanceBuffer();
    loadTextures();
}

void TextureArrayApp::createUniformBuffer()
{
    // Create dynamic uniform buffer that will store our transformation matrix
    // Dynamic buffers can be frequently updated by the CPU
    mVSConstants = gx::createBuffer( gx::BufferDesc()
        .name( "VS constants CB" )
        .sizeInBytes( sizeof( mat4 ) * 2 )
        .usage( gx::USAGE_DYNAMIC )
        .bindFlags( gx::BIND_UNIFORM_BUFFER )
        .cpuAccessFlags( gx::CPU_ACCESS_WRITE )
    );
}

void TextureArrayApp::createPipelineState()
{
    // This tutorial uses two types of input: per-vertex data and per-instance data.
    vector<gx::LayoutElement> layoutElems = {
        // Per-vertex data - first buffer slot
        // Attribute 0 - vertex position
        { 0, 0, 3, gx::VT_FLOAT32, false },
        // Attribute 1 - texture coordinates
        { 1, 0, 2, gx::VT_FLOAT32, false },

        // Per-instance data - second buffer slot
        // We will use four attributes to encode instance-specific 4x4 transformation matrix
        // Attribute 2 - first row
        { 2, 1, 4, gx::VT_FLOAT32, false, gx::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE },
        // Attribute 3 - second row
        { 3, 1, 4, gx::VT_FLOAT32, false, gx::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE },
        // Attribute 4 - third row
        { 4, 1, 4, gx::VT_FLOAT32, false, gx::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE },
        // Attribute 5 - fourth row
        { 5, 1, 4, gx::VT_FLOAT32, false, gx::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE },
        // Attribute 6 - texture array index
        { 6, 1, 1, gx::VT_FLOAT32, false, gx::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE }
    };

    // Pipeline state object encompasses configuration of all GPU stages
    mPipelineState = TexturedCube::createPipelineState( getRenderDevice(), getSwapChainColorFormat(), getSwapChainDepthFormat(), getAssetPath( "cube_inst.vsh" ), getAssetPath( "cube_inst.psh" ), layoutElems );

    // Since we did not explcitly specify the type for 'Constants' variable, default
    // type (SHADER_RESOURCE_VARIABLE_TYPE_STATIC) will be used. Static variables never
    // change and are bound directly through the pipeline state object.
    mPipelineState->GetStaticVariableByName( gx::SHADER_TYPE_VERTEX, "Constants" )->Set( mVSConstants );
    // Create a shader resource binding object and bind all static resources in it
    mPipelineState->CreateShaderResourceBinding( &mSRB, true );
}

void TextureArrayApp::createInstanceBuffer()
{
    // Create instance data buffer that will store transformation matrices
    mInstanceBuffer = gx::createBuffer( gx::BufferDesc()
        .name( "Instance data buffer" )
        // Use default usage as this buffer will only be updated when grid size changes
        .usage( gx::USAGE_DEFAULT )
        .bindFlags( gx::BIND_VERTEX_BUFFER )
        .sizeInBytes( sizeof( InstanceData ) * sMaxInstances )
    );
    populateInstanceBuffer();
}

void TextureArrayApp::loadTextures()
{
    // Load a texture array
    gx::TextureRef texArray;
    for( int tex = 0; tex < sNumTextures; ++tex ) {
        // Load current texture
        std::stringstream fileNameSS;
        fileNameSS << "DGLogo" << tex << ".png";
        std::string fileName = fileNameSS.str();
        gx::TextureRef srcTex = TexturedCube::loadTexture( getRenderDevice(), getAssetPath( fileName ) );
        const auto& texDesc = srcTex->GetDesc();
        if( texArray == nullptr ) {
            //	Create texture array
            texArray = gx::createTexture( gx::TextureDesc( texDesc )
                .arraySize( sNumTextures )
                .type( gx::RESOURCE_DIM_TEX_2D_ARRAY )
                .usage( gx::USAGE_DEFAULT )
                .bindFlags( gx::BIND_SHADER_RESOURCE ) );
        }
        // Copy current texture into the texture array
        for( uint32_t mip = 0; mip < texDesc.MipLevels; ++mip ) {
            gx::copyTexture( gx::CopyTextureAttribs( srcTex, gx::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, texArray, gx::RESOURCE_STATE_TRANSITION_MODE_TRANSITION )
                .srcMipLevel( mip )
                .dstMipLevel( mip )
                .dstSlice( tex ) 
            );
        }
    }

    // Get shader resource view from the texture array
    mTextureSRV = texArray->GetDefaultView( gx::TEXTURE_VIEW_SHADER_RESOURCE );
    // Set texture SRV in the SRB
    mSRB->GetVariableByName( gx::SHADER_TYPE_PIXEL, "g_Texture" )->Set( mTextureSRV );
}

void TextureArrayApp::updateUI()
{
    ui::SetNextWindowPos( ImVec2( 10, 10 ), ImGuiCond_FirstUseEver );
    if( ui::Begin( "Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) ) {
        if( ui::SliderInt( "Grid Size", &mGridSize, 1, 32 ) ) {
            populateInstanceBuffer();
        }
    }
    ui::End();
}

void TextureArrayApp::populateInstanceBuffer()
{
    // Populate instance data buffer
    std::vector<InstanceData> InstanceData( mGridSize * mGridSize * mGridSize );

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

                auto&    currInst = InstanceData[instId++];
                currInst.matrix = matrix;
                // Texture array index
                currInst.textureInd = static_cast<float>( tex_distr( gen ) );
            }
        }
    }
    // Update instance data buffer
    uint32_t DataSize = static_cast<uint32_t>( sizeof( InstanceData[0] ) * InstanceData.size() );
    gx::updateBuffer( mInstanceBuffer, 0, DataSize, InstanceData.data(), gx::RESOURCE_STATE_TRANSITION_MODE_TRANSITION );
}

void TextureArrayApp::update()
{
    updateUI();
    utils::updateWindowTitle();

    // Set cube view matrix
    mat4 view = glm::lookAt( glm::vec3( 0.0f, 2.0f, 4.0f ), vec3( 0.0f ), vec3( 0.0f, 1.0f, 0.0f ) );
    // Get projection matrix adjusted to the current screen orientation
    mat4 proj = glm::perspective( glm::pi<float>() / 4.0f, getWindowAspectRatio(), 0.1f, 100.f );
    // Compute world-view-projection matrix
    mWorldViewProjMatrix = proj * view;
    // Global rotation matrix
    mRotationMatrix = glm::rotate( -static_cast<float>( getElapsedSeconds() ) * 0.25f, vec3( 1.0f, 0.0f, 0.0f ) ) * glm::rotate( static_cast<float>( getElapsedSeconds() ) * 1.0f, vec3( 0.0f, 1.0f, 0.0f ) );
}

void TextureArrayApp::draw()
{
    // Clear the back buffer
    gx::clear( ColorA( 0.350f, 0.350f, 0.350f, 1.0f ) );

    {   // Map the buffer and write current world-view-projection matrix
        gx::MapHelper<mat4> constants( getImmediateContext(), mVSConstants, gx::MAP_WRITE, gx::MAP_FLAG_DISCARD );
        constants[0] = glm::transpose( mWorldViewProjMatrix );
        constants[1] = glm::transpose( mRotationMatrix );
    }

    // Bind vertex, instance and index buffers
    uint32_t offsets[] = { 0, 0 };
    gx::Buffer* buffers[] = { mCubeVertexBuffer, mInstanceBuffer };
    gx::setVertexBuffers( 0, 2, buffers, offsets, gx::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, gx::SET_VERTEX_BUFFERS_FLAG_RESET );
    gx::setIndexBuffer( mCubeIndexBuffer, 0, gx::RESOURCE_STATE_TRANSITION_MODE_TRANSITION );
    // Set the pipeline state
    gx::setPipelineState( mPipelineState );
    // Commit shader resources. RESOURCE_STATE_TRANSITION_MODE_TRANSITION mode makes sure that resources are transitioned to required states.
    gx::commitShaderResources( mSRB, gx::RESOURCE_STATE_TRANSITION_MODE_TRANSITION );
    // This is an indexed draw call. 
    gx::drawIndexed( gx::DrawIndexedAttribs()
        .indexType( gx::VT_UINT32 )
        .numIndices( 36 )
        // The number of instances
        .numInstances( mGridSize * mGridSize * mGridSize )
        // Verify the state of vertex and index buffers
        .flags( gx::DRAW_FLAG_VERIFY_ALL )
    );
}

CINDER_APP( TextureArrayApp, RendererGx )
