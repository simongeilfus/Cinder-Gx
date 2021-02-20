#include "cinder/app/App.h"
#include "cinder/app/RendererGx.h"

#include "cinder/graphics/wrapper.h"
#include "cinder/graphics/PipelineState.h"
#include "cinder/graphics/Shader.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class HelloTriangleApp : public App {
  public:
	void setup() override;
    void draw() override;

	gx::PipelineStateRef mPipelineState;
};

void HelloTriangleApp::setup()
{
    // Pipeline state object encompasses configuration of all GPU stages
    mPipelineState = gx::createGraphicsPipelineState( gx::GraphicsPipelineStateCreateInfo()
        // Pipeline state name is used by the engine to report issues.
        // It is always a good idea to give objects descriptive names.
        .name( "Simple triangle PSO" )
        // No back face culling for this tutorial
        .rasterizerStateDesc( gx::RasterizerStateDesc().cullMode( gx::CULL_MODE_NONE ) )
        // Disable depth testing
        .depthStencilDesc( gx::DepthStencilStateDesc().depthEnable( false ) )
        // For this tutorial, we will use simple vertex shader
        // that creates a procedural triangle
        // Diligent Engine can use HLSL source on all supported platforms.
        // It will convert HLSL to GLSL in OpenGL mode, while Vulkan backend will compile it directly to SPIRV.
        .vertexShader( gx::createShader( gx::ShaderCreateInfo()
            .name( "Triangle vertex shader" )
            .shaderType( gx::SHADER_TYPE_VERTEX )
            // Tell the system that the shader source code is in HLSL.
            // For OpenGL, the engine will convert this into GLSL under the hood.
            .sourceLanguage( gx::SHADER_SOURCE_LANGUAGE_HLSL )
            .source( R"(
                struct PSInput 
                { 
                    float4 Pos   : SV_POSITION; 
                    float3 Color : COLOR; 
                };

                void main(in  uint    VertId : SV_VertexID,
                            out PSInput PSIn) 
                {
                    float4 Pos[3];
                    Pos[0] = float4(-0.5, -0.5, 0.0, 1.0);
                    Pos[1] = float4( 0.0, +0.5, 0.0, 1.0);
                    Pos[2] = float4(+0.5, -0.5, 0.0, 1.0);

                    float3 Col[3];
                    Col[0] = float3(1.0, 0.0, 0.0); // red
                    Col[1] = float3(0.0, 1.0, 0.0); // green
                    Col[2] = float3(0.0, 0.0, 1.0); // blue

                    PSIn.Pos   = Pos[VertId];
                    PSIn.Color = Col[VertId];
                }
            )" )
        ) )
        // Pixel shader simply outputs interpolated vertex color
        .pixelShader( gx::createShader( gx::ShaderCreateInfo()
            .name( "Triangle pixel shader" )
            .shaderType( gx::SHADER_TYPE_PIXEL )
            // Tell the system that the shader source code is in HLSL.
            // For OpenGL, the engine will convert this into GLSL under the hood.
            .sourceLanguage( gx::SHADER_SOURCE_LANGUAGE_HLSL )
            .source( R"(
                struct PSInput 
                { 
                    float4 Pos   : SV_POSITION; 
                    float3 Color : COLOR; 
                };

                struct PSOutput
                { 
                    float4 Color : SV_TARGET; 
                };

                void main(in  PSInput  PSIn,
                          out PSOutput PSOut)
                {
                    PSOut.Color = float4(PSIn.Color.rgb, 1.0);
                }
            )" )
        ) )
    );
}

void HelloTriangleApp::draw()
{
    // Clear the back buffer
    gx::clear( ColorA( 0.350f, 0.350f, 0.350f, 1.0f ) );
    // Set the pipeline state in the immediate context
    gx::setPipelineState( mPipelineState );
    // Typically we should now call CommitShaderResources(), however shaders in this example don't
    // use any resources.
    // We will render 3 vertices
    gx::draw( gx::DrawAttribs().numVertices( 3 ) );
}

CINDER_APP( HelloTriangleApp, RendererGx )