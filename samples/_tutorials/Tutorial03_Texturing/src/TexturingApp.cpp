#include "cinder/app/App.h"
#include "cinder/app/RendererGx.h"

#include "cinder/graphics/wrapper.h"
#include "cinder/graphics/Buffer.h"
#include "cinder/graphics/PipelineState.h"
#include "cinder/graphics/Shader.h"
#include "cinder/graphics/Texture.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class TexturingApp : public App {
public:
    void setup() override;
    void update() override;
    void draw() override;

    void createPipelineState();
    void createVertexBuffer();
    void createIndexBuffer();
    void createUniformBuffer();
    void loadTexture();

    gx::PipelineStateRef         mPipelineState;
    gx::ShaderResourceBindingRef mSRB;
    gx::BufferRef                mCubeVertexBuffer;
    gx::BufferRef                mCubeIndexBuffer;
    gx::BufferRef                mVSConstants;
    
    gx::TextureViewRef           mTextureSRV;
    mat4                         mWorldViewProjMatrix;
};

void TexturingApp::setup()
{
    createUniformBuffer();
    createPipelineState();
    createVertexBuffer();
    createIndexBuffer();

    loadTexture();
}

void TexturingApp::createUniformBuffer()
{
    // Create dynamic uniform buffer that will store our transformation matrix
    // Dynamic buffers can be frequently updated by the CPU
    mVSConstants = gx::createBuffer( gx::BufferDesc()
        .name( "VS constants CB" )
        .size( sizeof( mat4 ) )
        .usage( gx::USAGE_DYNAMIC )
        .bindFlags( gx::BIND_UNIFORM_BUFFER )
        .cpuAccessFlags( gx::CPU_ACCESS_WRITE )
    );
}

void TexturingApp::createPipelineState()
{
    // Pipeline state object encompasses configuration of all GPU stages
    mPipelineState = gx::createGraphicsPipelineState( gx::GraphicsPipelineCreateInfo()
        // Pipeline state name is used by the engine to report issues. It is always a good idea to give objects descriptive names.
        .name( "Cube PSO" )
        // Define vertex shader input layout
        .inputLayout( {
            // Attribute 0 - vertex position
            { 0, 0, 3, gx::VT_FLOAT32, false },
            // Attribute 1 - texture coordinates
            { 1, 0, 2, gx::VT_FLOAT32, false }
         } )
        // Create a vertex shader
        .vertexShader( gx::createShader( gx::ShaderCreateInfo()
            .name( "Cube VS" )
            // Tell the system that the shader source code is in HLSL. For OpenGL, the engine will convert this into GLSL under the hood.
            .sourceLanguage( gx::SHADER_SOURCE_LANGUAGE_HLSL )
            // OpenGL backend requires emulated combined HLSL texture samplers (g_Texture + g_Texture_sampler combination)
            .useCombinedTextureSamplers( true )
            .shaderType( gx::SHADER_TYPE_VERTEX )
            .filePath( getAssetPath( "cube.vsh" ) )
        ) )
        // Create a pixel shader
        .pixelShader( gx::createShader( gx::ShaderCreateInfo()
            .name( "Cube PS" )
            // Tell the system that the shader source code is in HLSL. For OpenGL, the engine will convert this into GLSL under the hood.
            .sourceLanguage( gx::SHADER_SOURCE_LANGUAGE_HLSL )
            // OpenGL backend requires emulated combined HLSL texture samplers (g_Texture + g_Texture_sampler combination)
            .useCombinedTextureSamplers( true )
            .shaderType( gx::SHADER_TYPE_PIXEL )
            .filePath( getAssetPath( "cube.psh" ) )
        ) )
        // Define variable type that will be used by default
        .variables( { { gx::SHADER_TYPE_PIXEL, "g_Texture", gx::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE } } )
        // Define immutable sampler for g_Texture. Immutable samplers should be used whenever possible
        .immutableSamplers( { { gx::SHADER_TYPE_PIXEL, "g_Texture", Diligent::SamplerDesc() } } )
    );

    // Since we did not explcitly specify the type for 'Constants' variable, default
    // type (SHADER_RESOURCE_VARIABLE_TYPE_STATIC) will be used. Static variables never
    // change and are bound directly through the pipeline state object.
    mPipelineState->GetStaticVariableByName( gx::SHADER_TYPE_VERTEX, "Constants" )->Set( mVSConstants );
    // Create a shader resource binding object and bind all static resources in it
    mPipelineState->CreateShaderResourceBinding( &mSRB, true );
}

void TexturingApp::createVertexBuffer()
{
    // Layout of this structure matches the one we defined in the pipeline state
    struct Vertex {
        vec3 pos;
        vec2 uv;
    };

    // Cube vertices
    //    5-(-1,+1,+1)________________6-(+1,+1,+1)
    //               /|              /|
    //              / |             / |
    //             /  |            /  |
    //            /   |           /   |
    //4(-1,-1,+1)/____|__________/7-(+1,-1,+1)
    //           |    |__________|____|
    //           |   /1(-1,+1,-1)|    /2-(+1,+1,-1)
    //           |  /            |   /
    //           | /             |  /
    //           |/              | /
    //           /_______________|/
    //        0-(-1,-1,-1)    3-(+1,-1,-1)
    //

    // This time we have to duplicate verices because texture coordinates cannot be shared
    Vertex cubeVerts[] = {
        { vec3( -1,-1,-1 ), vec2( 0,1 ) },
        { vec3( -1,+1,-1 ), vec2( 0,0 ) },
        { vec3( +1,+1,-1 ), vec2( 1,0 ) },
        { vec3( +1,-1,-1 ), vec2( 1,1 ) },

        { vec3( -1,-1,-1 ), vec2( 0,1 ) },
        { vec3( -1,-1,+1 ), vec2( 0,0 ) },
        { vec3( +1,-1,+1 ), vec2( 1,0 ) },
        { vec3( +1,-1,-1 ), vec2( 1,1 ) },

        { vec3( +1,-1,-1 ), vec2( 0,1 ) },
        { vec3( +1,-1,+1 ), vec2( 1,1 ) },
        { vec3( +1,+1,+1 ), vec2( 1,0 ) },
        { vec3( +1,+1,-1 ), vec2( 0,0 ) },

        { vec3( +1,+1,-1 ), vec2( 0,1 ) },
        { vec3( +1,+1,+1 ), vec2( 0,0 ) },
        { vec3( -1,+1,+1 ), vec2( 1,0 ) },
        { vec3( -1,+1,-1 ), vec2( 1,1 ) },

        { vec3( -1,+1,-1 ), vec2( 1,0 ) },
        { vec3( -1,+1,+1 ), vec2( 0,0 ) },
        { vec3( -1,-1,+1 ), vec2( 0,1 ) },
        { vec3( -1,-1,-1 ), vec2( 1,1 ) },

        { vec3( -1,-1,+1 ), vec2( 1,1 ) },
        { vec3( +1,-1,+1 ), vec2( 0,1 ) },
        { vec3( +1,+1,+1 ), vec2( 0,0 ) },
        { vec3( -1,+1,+1 ), vec2( 1,0 ) }
    };

    // Create a vertex buffer that stores cube vertices
    gx::BufferData data = { cubeVerts, sizeof( cubeVerts ) };
    mCubeVertexBuffer = gx::createBuffer( gx::BufferDesc()
        .name( "Cube vertex buffer" )
        .usage( gx::USAGE_IMMUTABLE )
        .bindFlags( gx::BIND_VERTEX_BUFFER )
        .size( sizeof( cubeVerts ) ),
        &data
    );
}

void TexturingApp::createIndexBuffer()
{
    uint32_t indices[] = {
        2,0,1,    2,3,0,
        4,6,5,    4,7,6,
        8,10,9,   8,11,10,
        12,14,13, 12,15,14,
        16,18,17, 16,19,18,
        20,21,22, 20,22,23
    };

    gx::BufferData data = { indices, sizeof( indices ) };
    mCubeIndexBuffer = gx::createBuffer( gx::BufferDesc()
        .name( "Cube index buffer" )
        .usage( gx::USAGE_IMMUTABLE )
        .bindFlags( gx::BIND_INDEX_BUFFER )
        .size( sizeof( indices ) ),
        &data
    );
}

void TexturingApp::loadTexture()
{
    gx::TextureRef tex = gx::createTexture( loadImage( loadAsset( "DGLogo.png" ) ) );
    mTextureSRV = tex->GetDefaultView( gx::TEXTURE_VIEW_SHADER_RESOURCE );

    // Set texture SRV in the SRB
    mSRB->GetVariableByName( gx::SHADER_TYPE_PIXEL, "g_Texture" )->Set( mTextureSRV );
}

void TexturingApp::update()
{
    // Apply rotation
    mat4 cubeModelTransform = glm::rotate( glm::pi<float>() * 0.1f, vec3( 1.0f, 0.0f, 0.0f ) ) * glm::rotate( (float) getElapsedSeconds(), vec3( 0.0f, 1.0f, 0.0f ) );
    // Camera is at (0, 0, -5) looking along the Z axis
    mat4 view = glm::lookAt( glm::vec3( 0.f, 0.0f, 5.0f ), vec3( 0.0f ), vec3( 0.0f, 1.0f, 0.0f ) );
    // Get projection matrix adjusted to the current screen orientation
    mat4 proj = glm::perspective( glm::pi<float>() / 4.0f, getWindowAspectRatio(), 0.1f, 100.f );
    // Compute world-view-projection matrix
    mWorldViewProjMatrix = proj * view * cubeModelTransform;
}

void TexturingApp::draw()
{
    // Clear the back buffer
    gx::clear( ColorA( 0.350f, 0.350f, 0.350f, 1.0f ) );

    {   // Map the buffer and write current world-view-projection matrix
        gx::MapHelper<mat4> constants( getImmediateContext(), mVSConstants, gx::MAP_WRITE, gx::MAP_FLAG_DISCARD );
        *constants = glm::transpose( mWorldViewProjMatrix );
    }

    // Bind vertex and index buffers
    gx::setVertexBuffer( mCubeVertexBuffer, gx::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, gx::SET_VERTEX_BUFFERS_FLAG_RESET );
    gx::setIndexBuffer( mCubeIndexBuffer, 0, gx::RESOURCE_STATE_TRANSITION_MODE_TRANSITION );
    // Set the pipeline state
    gx::setPipelineState( mPipelineState );
    // Commit shader resources. RESOURCE_STATE_TRANSITION_MODE_TRANSITION mode makes sure that resources are transitioned to required states.
    gx::commitShaderResources( mSRB, gx::RESOURCE_STATE_TRANSITION_MODE_TRANSITION );
    // This is an indexed draw call. Verify the state of vertex and index buffers
    gx::drawIndexed( gx::DrawIndexedAttribs().indexType( gx::VT_UINT32 ).numIndices( 36 ).flags( gx::DRAW_FLAG_VERIFY_ALL ) );
}

CINDER_APP( TexturingApp, RendererGx )