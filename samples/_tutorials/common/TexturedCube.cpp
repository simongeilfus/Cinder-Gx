#include "TexturedCube.h"
#include "cinder/graphics/PipelineState.h"

using namespace std;
using namespace ci;

namespace TexturedCube {

ci::gx::BufferRef createVertexBuffer( ci::gx::RenderDevice* renderDevice ) 
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
    gx::BufferRef buffer;
    renderDevice->CreateBuffer( gx::BufferDesc()
        .name( "Cube vertex buffer" )
        .usage( gx::USAGE_IMMUTABLE )
        .bindFlags( gx::BIND_VERTEX_BUFFER )
        .sizeInBytes( sizeof( cubeVerts ) ),
        &data, &buffer
    );
    return buffer;
}

ci::gx::BufferRef createIndexBuffer( ci::gx::RenderDevice* renderDevice )
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
    gx::BufferRef buffer;
    renderDevice->CreateBuffer( gx::BufferDesc()
        .name( "Cube index buffer" )
        .usage( gx::USAGE_IMMUTABLE )
        .bindFlags( gx::BIND_INDEX_BUFFER )
        .sizeInBytes( sizeof( indices ) ),
        &data, &buffer
    );
    return buffer;
}

ci::gx::TextureRef  loadTexture( ci::gx::RenderDevice* renderDevice, const ci::fs::path &path )
{
    return gx::createTexture( renderDevice, loadImage( loadFile( path ) ) );
}

ci::gx::PipelineStateRef createPipelineState( ci::gx::RenderDevice* renderDevice, ci::gx::TEXTURE_FORMAT rtvFormat, ci::gx::TEXTURE_FORMAT dsvFormat, const ci::fs::path &vsFilePath, const ci::fs::path &psFilePath, std::vector<ci::gx::LayoutElement> layoutElements, uint8_t sampleCount )
{
    // Define default vertex shader input layout
    // This tutorial uses two types of input: per-vertex data and per-instance data.
    const vector<gx::LayoutElement> DefaultLayoutElems = {
        // Per-vertex data - first buffer slot
        // Attribute 0 - vertex position
        { 0, 0, 3, gx::VT_FLOAT32, false },
        // Attribute 1 - texture coordinates
        { 1, 0, 2, gx::VT_FLOAT32, false }
    };

    // Create a vertex shader
    gx::ShaderRef vs = gx::createShader( gx::ShaderCreateInfo()
        .name( "Cube VS" )
        // Tell the system that the shader source code is in HLSL. For OpenGL, the engine will convert this into GLSL under the hood.
        .sourceLanguage( gx::SHADER_SOURCE_LANGUAGE_HLSL )
        // OpenGL backend requires emulated combined HLSL texture samplers (g_Texture + g_Texture_sampler combination)
        .useCombinedTextureSamplers( true )
        .shaderType( gx::SHADER_TYPE_VERTEX )
        .filePath( vsFilePath )
    );

    // Create a pixel shader
    gx::ShaderRef ps = gx::createShader( gx::ShaderCreateInfo()
        .name( "Cube PS" )
        // Tell the system that the shader source code is in HLSL. For OpenGL, the engine will convert this into GLSL under the hood.
        .sourceLanguage( gx::SHADER_SOURCE_LANGUAGE_HLSL )
        // OpenGL backend requires emulated combined HLSL texture samplers (g_Texture + g_Texture_sampler combination)
        .useCombinedTextureSamplers( true )
        .shaderType( gx::SHADER_TYPE_PIXEL )
        .filePath( psFilePath )
    );

    gx::GraphicsPipelineCreateInfo psoCreateInfo = gx::GraphicsPipelineCreateInfo()
        // Pipeline state name is used by the engine to report issues. It is always a good idea to give objects descriptive names.
        .name( "Cube PSO" )
        // Set render target format which is the format of the swap chain's color buffer
        .rtvFormat( 0, rtvFormat )
        // Set depth buffer format which is the format of the swap chain's back buffer
        .dsvFormat( dsvFormat )
        // Set the desired number of samples
        .sampleDesc( gx::SampleDesc().count( sampleCount ) )
        // Define vertex shader input layout
        .inputLayout( gx::InputLayoutDesc( layoutElements.empty() ? DefaultLayoutElems.data() : layoutElements.data(), layoutElements.empty() ? DefaultLayoutElems.size() : layoutElements.size() ) )
        // Assign vertex shader
        .vertexShader( vs )
        // Assign pixel shader
        .pixelShader( ps )
        // Define variable type that will be used by default
        // Shader variables should typically be mutable, which means they are expected to change on a per-instance basis
        .variables( { { gx::SHADER_TYPE_PIXEL, "g_Texture", gx::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE } } )
        // Define immutable sampler for g_Texture. Immutable samplers should be used whenever possible
        .immutableSamplers( { { gx::SHADER_TYPE_PIXEL, "g_Texture", Diligent::SamplerDesc() } } );

    return gx::createGraphicsPipelineState( psoCreateInfo );
}

} // namespace TexturedCube