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

#include "cinder/Rect.h"
#include "cinder/Camera.h"
#include "cinder/GeomIo.h"
#include "cinder/graphics/Buffer.h"
#include "cinder/graphics/PipelineState.h"

#include <unordered_map>

//#define IMGUI_DEGUG

// TODO:
//
// [ ] Make the bindless resource mode use batches of textures to overcome current limit
// [ ] Move color to Constants buffer instead of per-vertex attribute
// [ ] Detach transform only on the next commit to make sure to use all following transforms after the call to "detachTransform"

namespace cinder { namespace graphics {

class CI_API DrawContext {
public:
    DrawContext();

    //! Draws a geom::Source \a source at the origin.
    void draw( const geom::Source &source );
    //! Draws a solid rectangle \a r on the XY-plane
    void drawSolidRect( const Rectf &r, const vec2 &upperLeftTexCoord = vec2( 0, 1 ), const vec2 &lowerRightTexCoord = vec2( 1, 0 ) );
    //! Draws a solid rounded rectangle centered around \a rect, with a corner radius of \a cornerRadius
    void drawSolidRoundedRect( const Rectf &r, float cornerRadius, int numSegmentsPerCorner = 0 ) const;
    //! Draws a filled circle centered around \a center with a radius of \a radius. Default \a numSegments requests a conservative (high-quality but slow) number based on radius.
    void drawSolidCircle( const vec2 &center, float radius, int numSegments = -1 ) const;
    //! Draws a filled ellipse centered around \a center with an X-axis radius of \a radiusX and a Y-axis radius of \a radiusY. Default \a numSegments requests a conservative (high-quality but slow) number based on radius.
    void drawSolidEllipse( const vec2 &center, float radiusX, float radiusY, int numSegments = -1 ) const;

    //! Draws a line between points a and b
    void drawLine( const vec3 &a, const vec3 &b );
    //! Draws a line between points a and b
    void drawLine( const vec2 &a, const vec2 &b );

    //! Submits the current DrawContext on the app default RenderDevice and Immediate DeviceContext
    void submit( bool flushAfterSubmit = true );
    //! Submits the current DrawContext on a a\ RenderDevice and \a DeviceContext
    void submit( RenderDevice* device, DeviceContext* context, bool flushAfterSubmit = true );
    //! Bakes the current DrawContext into a CommandList, useful for submitting later or from a separate thread.
    gx::CommandListRef bake( DeviceContext* context );
    //! Bakes the current DrawContext into a CommandList, useful for submitting later or from a separate thread.
    gx::CommandListRef bake( RenderDevice* device, DeviceContext* context );
    //! Clears the current DrawContext in preparation for a new frame
    void flush();

#if defined( IMGUI_DEGUG )
    void debugSubmit( const char* label, bool* open = nullptr, bool flushAfterSubmit = true );
#endif

    //! Sets the viewport based on a pair<vec2,vec2> representing the position of the lower-left corner and the size, respectively
    inline void				viewport( const std::pair<vec2, vec2> &viewport ) { setViewport( viewport ); }
    //! Sets the viewport based on the position of the lower-left corner and the size
    inline void             viewport( float x, float y, float width, float height ) { setViewport( std::pair<vec2, vec2>( vec2( x, y ), vec2( width, height ) ) ); }
    //! Sets the viewport based on the position of the lower-left corner and the size
    inline void             viewport( const vec2 &position, const vec2 &size ) { setViewport( std::pair<vec2, vec2>( position, size ) ); }
    //! Sets the viewport based on the size
    inline void             viewport( const vec2 &size ) { setViewport( { vec2(), size } ); }
    //! Sets the viewport based on a pair<vec2,vec2> representing the position of the lower-left corner and the size, respectively
    void					setViewport( const std::pair<vec2, vec2> &viewport );
    //! Pushes the viewport based on a pair<vec2,vec2> representing the position of the lower-left corner and the size, respectively
    void					pushViewport( const std::pair<vec2, vec2> &viewport );
    //! Pushes the viewport based on a pair<vec2,vec2> representing the position of the lower-left corner and the size, respectively
    inline void             pushViewport( int x, int y, int width, int height ) { pushViewport( std::pair<vec2, vec2>( vec2( x, y ), vec2( width, height ) ) ); }
    //! Pushes the viewport based on a pair<vec2,vec2> representing the position of the lower-left corner and the size, respectively
    inline void             pushViewport( const vec2 &position, const vec2 &size ) { pushViewport( std::pair<vec2, vec2>( position, size ) ); }
    //! Pushes the viewport based on a pair<vec2,vec2> representing the position of the lower-left corner and the size, respectively
    inline void             pushViewport( const vec2 &size ) { pushViewport( vec2(), size ); }
    //! Duplicates and pushes the top of the Viewport stack
    void					pushViewport() { pushViewport( getViewport() ); }
    //! Pops the viewport. If \a forceRestore then redundancy checks are skipped and the hardware state is always set.
    void					popViewport( bool forceRestore = false );
    //! Returns a pair<vec2,vec2> representing the position of the lower-left corner and the size, respectively of the viewport
    std::pair<vec2, vec2>	getViewport();

    //! Sets the scissor box based on a pair<ivec2,ivec2> representing the position of the lower-left corner and the size, respectively	
    inline void             scissor( const std::pair<ivec2, ivec2> positionAndSize ) { setScissor( positionAndSize ); }
    //! Sets the scissor box the position of the lower-left corner and the size
    inline void             scissor( int x, int y, int width, int height ) { setScissor( std::pair<ivec2, ivec2>( ivec2( x, y ), ivec2( width, height ) ) ); }
    //! Sets the scissor box the position of the lower-left corner and the size
    inline void             scissor( const ivec2 &position, const ivec2 &size ) { setScissor( std::pair<ivec2, ivec2>( position, size ) ); }
    //! Sets the scissor box based on a pair<ivec2,ivec2> representing the position of the lower-left corner and the size, respectively	
    void					setScissor( const std::pair<ivec2, ivec2> &scissor );
    //! Pushes the scissor box based on a pair<ivec2,ivec2> representing the position of the lower-left corner and the size, respectively	
    void					pushScissor( const std::pair<ivec2, ivec2> &scissor );
    //! Duplicates and pushes the top of the Scissor box stack
    void					pushScissor();
    //! Pushes the scissor box based on a pair<ivec2,ivec2> representing the position of the lower-left corner and the size, respectively. If \a forceRestore then redundancy checks are skipped and the hardware state is always set.
    void					popScissor( bool forceRestore = false );
    //! Returns a pair<ivec2,ivec2> representing the position of the lower-left corner and the size, respectively of the scissor box
    std::pair<ivec2, ivec2>	getScissor();

    //! Enables or disables reading / testing from depth buffer.
    void enableDepthRead( bool enable = true );
    //! Enables or disables writing to depth buffer. Note that reading must also be enabled for writing to have any effect.
    void enableDepthWrite( bool enable = true );
    //! Enables or disables writing to and reading / testing from depth buffer.
    inline void enableDepth( bool enable = true ) { enableDepthRead( enable ); enableDepthWrite( enable ); }
    //! Disables reading / testing from the depth buffer.
    inline void disableDepthRead() { enableDepthRead( false ); }
    //! Disables writing to depth buffer.
    inline void disableDepthWrite() { enableDepthWrite( false ); }

    //! Enables or disables blending state but does not modify blend function.
    void enableBlending( bool enable = false );
    //! Disables blending state but does not modify blend function
    inline void disableBlending() { enableBlending( false ); }
    //! Enables blending and sets the blend function to unpremultiplied alpha blending when \p enable is \c true; otherwise disables blending without modifying the blend function.
    void enableAlphaBlending( bool enable = true );
    //! Enables blending and sets the blend function to premultiplied alpha blending
    void enableAlphaBlendingPremult();
    //! Enables \c GL_BLEND and sets the blend function to additive blending
    void enableAdditiveBlending();

    //! Specifies whether polygons are culled. Equivalent to calling cullFace( \c CULL_MODE_BACK ).
    void enableFaceCulling( bool enable = true );
    //! Specifies whether front or back-facing polygons are culled.
    void cullFace( CULL_MODE mode );

    //! Enables or disables the stencil test operation, which controls reading and writing to the stencil buffer.
    void enableStencilTest( bool enable = true );
    //! Disables the stencil test operation.
    inline void disableStencilTest() { enableStencilTest( false ); }
    //! Sets the value of the stencil read test mask
    void stencilReadMask( uint8_t mask );
    //! Sets the value of the stencil write test mask
    void stencilWriteMask( uint8_t mask );

    //! Sets the current polygon rasterization mode.
    void polygonMode( FILL_MODE mode );
    //! Enables wireframe drawing by setting the \c PolygonMode to \c GL_LINE.
    void enableWireframe() { setWireframeEnabled( true ); }
    //! Disables wireframe drawing.
    inline void disableWireframe() { setWireframeEnabled( false ); }
    //! Toggles wireframe drawing according to \a enable.
    void setWireframeEnabled( bool enable = true );

    //! Sets the current View and Projection matrix from a Camera \a camera
    void setMatrices( const ci::Camera &camera );
    //! Sets the current Model matrix
    void setModelMatrix( const ci::mat4 &m );
    //! Sets the current View matrix
    void setViewMatrix( const ci::mat4 &m );
    //! Sets the current Projection matrix
    void setProjectionMatrix( const ci::mat4 &m );
    //! Pushes Model matrix
    void pushModelMatrix();
    //! Pops Model matrix
    void popModelMatrix();
    //! Pushes View matrix
    void pushViewMatrix();
    //! Pops View matrix
    void popViewMatrix();
    //! Pushes Projection matrix
    void pushProjectionMatrix();
    //! Pops Projection matrix
    void popProjectionMatrix();
    //! Pushes Model and View matrices
    void pushModelView();
    //! Pops Model and View matrices
    void popModelView();
    //! Pushes Model, View and Projection matrices
    void pushMatrices();
    //! Pops Model, View and Projection matrices
    void popMatrices();

    //! Multiplies the current Model matrix by \a mtx
    void multModelMatrix( const ci::mat4 &mtx );
    //! Multiplies the current View matrix by \a mtx
    void multViewMatrix( const ci::mat4 &mtx );
    //! Multiplies the current Projection matrix by \a mtx
    void multProjectionMatrix( const ci::mat4 &mtx );

    //! Returns the current Model matrix
    mat4 getModelMatrix();
    //! Returns the current View matrix
    mat4 getViewMatrix();
    //! Returns the current Projection matrix
    mat4 getProjectionMatrix();
    //! Returns the current ModelView matrix
    mat4 getModelView();
    //! Returns the current ModelViewProjection matrix
    mat4 getModelViewProjection();
    //! Calculates and returns the inverse of the current View matrix
    mat4 calcViewMatrixInverse();
    //! Calculates and returns the inverse transpose of the current ModelView matrix
    mat3 calcModelMatrixInverseTranspose();
    //! Calculates and returns a normal matrix from the current View matrix
    mat3 calcNormalMatrix();
    //! Calculates and returns a viewport matrix from the current View matrix
    mat4 calcViewportMatrix();

    //! Sets the Model, View and projection matrices to match the screen 
    void setMatricesWindowPersp( int screenWidth, int screenHeight, float fovDegrees = 60.0f, float nearPlane = 1.0f, float farPlane = 1000.0f, bool originUpperLeft = true );
    //! Sets the Model, View and projection matrices to match the screen
    void setMatricesWindowPersp( const ci::ivec2 &screenSize, float fovDegrees = 60.0f, float nearPlane = 1.0f, float farPlane = 1000.0f, bool originUpperLeft = true );
    //! Sets the Model, View and projection matrices to match the screen
    void setMatricesWindow( int screenWidth, int screenHeight, bool originUpperLeft = true );
    //! Sets the Model, View and projection matrices to match the screen
    void setMatricesWindow( const ci::ivec2 &screenSize, bool originUpperLeft = true );

    //! Rotates the Model matrix by \a quat
    void rotate( const quat &quat );
    //! Rotates the Model matrix by \a angleRadians around the \a axis
    void rotate( float angleRadians, const ci::vec3 &axis );
    //! Rotates the Model matrix by \a angleRadians around the axis (\a x,\a y,\a z)
    inline void rotate( float angleRadians, float xAxis, float yAxis, float zAxis ) { rotate( angleRadians, ci::vec3( xAxis, yAxis, zAxis ) ); }
    //! Rotates the Model matrix by \a zRadians around the z-axis
    inline void rotate( float zRadians ) { rotate( zRadians, vec3( 0, 0, 1 ) ); }

    //! Scales the Model matrix by \a v
    void scale( const ci::vec3 &v );
    //! Scales the Model matrix by (\a x,\a y, \a z)
    inline void scale( float x, float y, float z ) { scale( vec3( x, y, z ) ); }
    //! Scales the Model matrix by \a v
    inline void scale( const ci::vec2 &v ) { scale( vec3( v.x, v.y, 1 ) ); }
    //! Scales the Model matrix by (\a x,\a y, 1)
    inline void scale( float x, float y ) { scale( vec3( x, y, 1 ) ); }
    //! Scales the Model matrix by \a v
    inline void scale( float s ) { scale( vec3( s, s, s ) ); }

    //! Translates the Model matrix by \a v
    void translate( const ci::vec3 &v );
    //! Translates the Model matrix by (\a x,\a y,\a z )
    inline void translate( float x, float y, float z ) { translate( vec3( x, y, z ) ); }
    //! Translates the Model matrix by \a v
    inline void translate( const ci::vec2 &v ) { translate( vec3( v, 0 ) ); }
    //! Translates the Model matrix by (\a x,\a y)
    inline void translate( float x, float y ) { translate( vec3( x, y, 0 ) ); }

    //! Returns a reference to the stack of Model matrices
    std::vector<mat4>&			getModelMatrixStack() { return mModelMatrixStack; }
    //! Returns a const reference to the stack of Model matrices
    const std::vector<mat4>&	getModelMatrixStack() const { return mModelMatrixStack; }
    //! Returns a reference to the stack of View matrices
    std::vector<mat4>&			getViewMatrixStack() { return mViewMatrixStack; }
    //! Returns a const reference to the stack of Model matrices
    const std::vector<mat4>&	getViewMatrixStack() const { return mViewMatrixStack; }
    //! Returns a reference to the stack of Projection matrices
    std::vector<mat4>&			getProjectionMatrixStack() { return mProjectionMatrixStack; }
    //! Returns a const reference to the stack of Projection matrices
    const std::vector<mat4>&	getProjectionMatrixStack() const { return mProjectionMatrixStack; }

    void color( float r, float g, float b );
    void color( float r, float g, float b, float a );
    void color( const ci::Color &c );
    void color( const ci::ColorA &c );
    void color( const ci::Color8u &c );
    void color( const ci::ColorA8u &c );

    //! Returns the current active color
    const ColorAf&		getCurrentColor() const;
    //! Sets the current active color
    void				setCurrentColor( const ColorAf &color );

    //! Sets the current active TextureView
    void bindTexture( TextureViewRef &texture );
    //! Resets the active TextureView to the default
    void unbindTexture();

    //! Returns whether the DrawContext has any commands
    bool empty() const { return mCommands.empty(); }

    //! Dynamic Transform prototype
    class Transform {
    public:
        Transform();
        void operator=( const ci::mat4 &transform );
    protected:
        bool mActive;
        std::string mName;
        uint32_t mTargetIndex;
        ci::mat4 mParentTransform;
        ci::mat4 mTransform;
        DrawContext* mParent;
        friend class DrawContext;
    };

    //! Returns the dynamic Transform associated with the \a name
    Transform& operator[]( const std::string &name );
    //! Inserts a dynamic transform in the current transform stack. 
    //! Note: Children of the current position in the stack won't be affected by changes to the dynamic transform. 
    //! Any other transform between this and the next call to a draw function will be overriden by modification to the Transform.
    void detachTransform( Transform &transform, const ci::mat4 &initialValue = {} );

protected:
    //! Returns \c true if \a value is different from the previous top of the stack
    template<typename T>
    bool		pushStackState( std::vector<T> &stack, T value );
    //! Returns \c true if the new top of \a stack is different from the previous top, or the stack is empty
    template<typename T>
    bool		popStackState( std::vector<T> &stack );
    //! Returns \c true if \a value is different from the previous top of the stack
    template<typename T>
    bool		setStackState( std::vector<T> &stack, T value );
    //! Returns \c true if \a result is valid; will return \c false when \a stack was empty
    template<typename T>
    bool		getStackState( std::vector<T> &stack, T *result );

    BufferRef                mIndexBuffer;
    BufferRef                mVertexBuffer;
    BufferRef                mConstantsBuffer;
    BufferView*              mConstantsBufferSRV;

    uint32_t                 mIndexBufferSize;
    uint32_t                 mVertexBufferSize;
    uint32_t                 mConstantsBufferSize;

    struct Pipeline {
        PipelineStateRef         pso;
        ShaderResourceBindingRef srb;
    };

    std::unordered_map<size_t, Pipeline> mPipelines;

    using Index = uint32_t;

    struct Vertex {
        vec3     position;
        vec2     uv;
        vec4     color;

        uint32_t constantsIndex;
        uint32_t textureIndex;

        void setPosition( const vec2 &p ) { position = vec3( p, 0.0f ); }
        void setPosition( const vec3 &p ) { position = p; }
        void setPosition( float x, float y, float z = 0.0f ) { position.x = x; position.y = y; position.z = z; }
        void setUv( const vec2 &v ) { uv = v; }
        void setUv( float x, float y ) { uv.x = x; uv.y = y; }
        void setColor( const vec4 &c ) { color = c; }
        void setColor( float r, float g, float b, float a = 1.0f ) { color.r = r; color.g = g; color.b = b; color.a = a; }
        void setConstantsIndex( uint32_t index ) { constantsIndex = index; }
        void setTextureIndex( uint32_t index ) { textureIndex = index; }
    };

    struct Constants {
        glm::mat4 transform;
    };

    struct Resources {
        uint32_t textureIndex;
    };

    bool        mVerifyDeviceFeatures;
    bool        mBindlessResources;

    uint32_t    mTextureIndex;
    uint32_t    mTextureCount;
    uint32_t    mMaxBindlessTextures;

    std::vector<IDeviceObject*> mTextures;
    TextureViewRef              mBaseTexture;

    uint32_t getTextureIndex( TextureViewRef &texture );
    IDeviceObject* getTextureAt( uint32_t index ) const;

    Transform& getTransform( const std::string &name );
    std::unordered_map<std::string, Transform> mTransforms;

    struct State {
        FILL_MODE           fillMode               = FILL_MODE_SOLID;
        CULL_MODE           cullMode               = CULL_MODE_BACK;
        bool                depthEnable            = true;
        bool                depthWriteEnable       = true;
        COMPARISON_FUNCTION depthFunc              = COMPARISON_FUNC_LESS;
        bool                stencilEnable          = false;
        uint8_t             stencilReadMask        = 0xFF;
        uint8_t             stencilWriteMask       = 0xFF;
        PRIMITIVE_TOPOLOGY  primitiveTopology      = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        bool                alphaToCoverageEnable  = false;
        bool                blendEnable            = false;
        BLEND_FACTOR        srcBlend               = BLEND_FACTOR_ONE;
        BLEND_FACTOR        destBlend              = BLEND_FACTOR_ZERO;
        BLEND_OPERATION     blendOp                = BLEND_OPERATION_ADD;
        BLEND_FACTOR        srcBlendAlpha          = BLEND_FACTOR_ONE;
        BLEND_FACTOR        destBlendAlpha         = BLEND_FACTOR_ZERO;
        BLEND_OPERATION     blendOpAlpha           = BLEND_OPERATION_ADD;

        size_t hash() const;
    };

    struct Command {
        uint32_t vertexOffset;
        uint32_t indexOffset;
        uint32_t indexCount;
        Resources resources;
        State state;
        vec4 viewport;
        ivec4 scissor;
    };

    Pipeline initializePipelineState( RenderDevice* device, const State &state );

    class DrawScope : private Noncopyable {
    public:
        ~DrawScope();

        Vertex* getVertices() const;
        Index*  getIndices() const;
        Index   getIndexOffset() const;
    private:
        DrawScope( DrawContext* context, uint32_t indexCount, uint32_t vertexCount );
        DrawContext* mContext;
        uint32_t mIndexCount;
        uint32_t mVertexCount;
        friend class DrawContext;
    };

    class GeomTarget : public geom::Target {
    public:
        GeomTarget( DrawContext* context, const geom::Source *source );
        ~GeomTarget();

        uint8_t	getAttribDims( geom::Attrib attr ) const override;
        void copyAttrib( geom::Attrib attr, uint8_t dims, size_t strideBytes, const float *sourceData, size_t count ) override;
        void copyIndices( geom::Primitive primitive, const uint32_t *sourceData, size_t numIndices, uint8_t requiredBytesPerIndex ) override;
    protected:
        Vertex* getVertices() const;
        Index*  getIndices() const;
        Index   getIndexOffset() const;
        const geom::Source* mSource;
        DrawContext*        mContext;
        uint32_t            mIndexCount;
        uint32_t            mVertexCount;
    };

    void        startDraw( uint32_t indexCount, uint32_t vertexCount );
    DrawScope   getDrawScope( uint32_t indexCount, uint32_t vertexCount );

    void commit();

    void invalidateTransform();
    void invalidateResources();
    void invalidateState();
    void invalidateViewport();
    void invalidateScissor();

    Vertex*   mVertex;
    Index*    mIndex;
    Index     mVertexIndex;
    uint32_t  mConstantIndex;
    uint32_t  mConstantCount;
    State     mState;

    bool mTransformValid;
    bool mStateValid;
    bool mViewportValid;
    bool mScissorValid;
    bool mResourcesValid;

    bool mGeomBuffersValid;
    bool mConstantBufferValid;
    bool mSRBsValid;
    bool mPSOsValid;

    bool mGeomBuffersImmutable;
    bool mConstantBufferImmutable;

    std::vector<Vertex>     mVertices;
    std::vector<Index>      mIndices;
    std::vector<Constants>  mConstants;
    std::vector<Command>    mCommands;

    std::vector<mat4>		 mModelMatrixStack;
    std::vector<mat4>		 mViewMatrixStack;
    std::vector<mat4>		 mProjectionMatrixStack;

    std::vector<std::pair<vec2, vec2>> mViewportStack;
    std::vector<std::pair<ivec2, ivec2>> mScissorStack;

    ci::ColorAf				 mColor;
};

}

namespace gx = graphics;
} // namespace cinder::graphics