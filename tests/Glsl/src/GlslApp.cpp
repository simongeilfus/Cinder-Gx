#include "cinder/app/App.h"
#include "cinder/app/RendererGx.h"

#include "cinder/graphics/wrapper.h"
#include "cinder/graphics/PipelineState.h"
#include "cinder/graphics/Shader.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class GlslApp : public App {
public:
    void setup() override;
    void draw() override;

    gx::PipelineStateRef mPipelineState;
};

void GlslApp::setup()
{
    // Pipeline state object encompasses configuration of all GPU stages
    mPipelineState = gx::createGraphicsPipelineState( gx::GraphicsPipelineStateCreateInfo()
        .vertexShader( gx::createShader( gx::ShaderCreateInfo()
            .shaderType( gx::SHADER_TYPE_VERTEX )
            .sourceLanguage( gx::SHADER_SOURCE_LANGUAGE_GLSL )
            .source( R"(
                layout(location = 0) out vec3 vColor;

                vec4 positions[3] = vec4[](
                    vec4(-0.5, -0.5, 0.0, 1.0),
                    vec4( 0.0, +0.5, 0.0, 1.0),
                    vec4(+0.5, -0.5, 0.0, 1.0)
                );

                vec3 colors[3] = vec3[](
                    vec3(1.0, 0.0, 0.0),
                    vec3(0.0, 1.0, 0.0),
                    vec3(0.0, 0.0, 1.0)
                );

                void main() {
                    gl_Position = positions[gl_VertexIndex];
                    vColor = colors[gl_VertexIndex];
                }
            )" )
        ) )
        // Pixel shader simply outputs interpolated vertex color
        .pixelShader( gx::createShader( gx::ShaderCreateInfo()
            .shaderType( gx::SHADER_TYPE_PIXEL )
            .sourceLanguage( gx::SHADER_SOURCE_LANGUAGE_GLSL )
            .source( R"(
                layout(location = 0) in vec3 vColor;
                layout(location = 0) out vec4 oColor;

                void main() {
                    oColor = vec4( vColor.rgb, 1.0 );
                }
            )" )
        ) )
    );
}

void GlslApp::draw()
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

CINDER_APP( GlslApp, RendererGx/*( RendererGx::Options().deviceType( gx::RENDER_DEVICE_TYPE_D3D12 ) )*/ )