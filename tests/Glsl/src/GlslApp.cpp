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
    mPipelineState = gx::createGraphicsPipelineState( gx::GraphicsPipelineDesc()
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
    gx::clear( ColorA( 0.350f, 0.350f, 0.350f, 1.0f ) );
    gx::setPipelineState( mPipelineState );
    gx::draw( gx::DrawAttribs().numVertices( 3 ) );
}

CINDER_APP( GlslApp, RendererGx )