#pragma once

#include "cinder/graphics/wrapper.h"
#include "cinder/graphics/Buffer.h"
#include "cinder/graphics/Texture.h"

namespace TexturedCube {

ci::gx::BufferRef   createVertexBuffer( ci::gx::RenderDevice* renderDevice );
ci::gx::BufferRef   createIndexBuffer( ci::gx::RenderDevice* renderDevice );
ci::gx::TextureRef  loadTexture( ci::gx::RenderDevice* renderDevice, const ci::fs::path &path );

ci::gx::PipelineStateRef createPipelineState( ci::gx::RenderDevice* pDevice, ci::gx::TEXTURE_FORMAT rtvFormat, ci::gx::TEXTURE_FORMAT dsvFormat, const ci::fs::path &vsFilePath, const ci::fs::path &psFilePath, std::vector<ci::gx::LayoutElement> layoutElements = {}, uint8_t sampleCount = 1 );

} // namespace TexturedCube