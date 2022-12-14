#pragma once
#include "IRenderer/Utils.h"
#include "IRenderer/BlendState.h"
#include "IRenderer/DepthStencilState.h"
#include "IRenderer/RasterizerState.h"
#include "IRenderer/SamplerState.h"
#include "PipelineState.h"

namespace Renderer
{
#define MAX_TEXTURE_SAMPLERS		16
#define MAX_VERTEXTEXTURE_SAMPLERS	4
#define MAX_TOTAL_SAMPLERS		(MAX_TEXTURE_SAMPLERS + MAX_VERTEXTEXTURE_SAMPLERS)

#define MAX_VERTEX_ATTRIBUTES		16
#define MAX_BOUND_VERTEX_BUFFERS	16

#define MAX_RENDERTARGET_BINDINGS	8

    struct IRenderer
    {
        /* Creates a rendering context for use on the calling thread.
         *
         * presentationParameters:	The initial device/backbuffer settings.
         * debugMode:			Enable debugging and backend validation features
         *				at the cost of reduced overall performance.
         *
         * Returns a device ready for use. Be sure to only call device functions from
         * the thread that it was created on!
         */
        IRenderer(const PresentationParameters& presentationParameters, uint8_t debugMode)
        {
        };
        IRenderer(const IRenderer&) = delete;
        IRenderer(const IRenderer&&) = delete;

        virtual void ResizeBackbuffer(const Size2D& parameters) = 0;
        virtual void ResizeMainViewport(const Size2D& parameters) = 0;
        /* Destroys a rendering context previously returned by CreateDevice. */
        virtual ~IRenderer()
        {
        };


        /* After your window is created, call this to check for high-DPI support.
         *
         * window:	The OS window handle, typically an SDL_Window*.
         * w:		Filled with the width of the window's drawable canvas.
         * h:		Filled with the height of the window's drawable canvas.
         */

        virtual void GetDrawableSize(void* window, int32_t* w, int32_t* h) = 0;

        /* Presentation */

        /* Presents the backbuffer to the window. */
        virtual void RunVM() = 0;
        virtual void SetBB() = 0;
        virtual void SwapBuffers() = 0;

        /* Drawing */

        /* Clears the active draw buffers of any previous contents.
         *
         * options:	Bitflags to specify color/depth/stencil buffers for clearing.
         * color:	The new value of the cleared color buffer. It is STRONGLY
         *		recommended to use 0.0f and 1.0f for all color channels!
         * depth:	The new value of the cleared depth buffer.
         * stencil:	The new value of the cleared stencil buffer.
         */
        virtual void Clear(ClearOptions options, FColor color, float depth, int32_t stencil) = 0;

        /* Draws data from vertex/index buffers.
         *
         * primitiveType:	The primitive topology of the vertex data.
         * baseVertex:		The starting offset to read from the vertex buffer.
         * minVertexIndex:	The lowest index value expected from the index buffer.
         * numVertices:		The highest offset expected from the index buffer.
         * startIndex:		The starting offset to read from the index buffer.
         * primitiveCount:	The number of primitives to draw.
         * indices:		The index buffer to bind for this draw call.
         * indexElementSize:	The size of the index type for this index buffer.
         */
        virtual void DrawIndexedPrimitives(
            PrimitiveType primitiveType,
            int32_t baseVertex,
            int32_t minVertexIndex,
            int32_t numVertices,
            int32_t startIndex,
            int32_t primitiveCount
        ) = 0;

        /* Draws data from vertex/index buffers with instancing enabled.
         *
         * primitiveType:	The primitive topology of the vertex data.
         * baseVertex:		The starting offset to read from the vertex buffer.
         * minVertexIndex:	The lowest index value expected from the index buffer.
         * numVertices:		The highest offset expected from the index buffer.
         * startIndex:		The starting offset to read from the index buffer.
         * primitiveCount:	The number of primitives to draw.
         * instanceCount:	The number of instances that will be drawn.
         * indices:		The index buffer to bind for this draw call.
         * indexElementSize:	The size of the index type for this index buffer.
         */
        virtual void DrawInstancedPrimitives(
            PrimitiveType primitiveType,
            int32_t baseVertex,
            int32_t minVertexIndex,
            int32_t numVertices,
            int32_t startIndex,
            int32_t primitiveCount,
            int32_t instanceCount
        ) = 0;

        /* Draws data from vertex buffers.
         * primitiveType:	The primitive topology of the vertex data.
         * vertexStart:		The starting offset to read from the vertex buffer.
         * primitiveCount:	The number of primitives to draw.
         */
        virtual void DrawPrimitives(
            PrimitiveType primitiveType,
            int32_t vertexStart,
            int32_t primitiveCount
        ) = 0;

        /* Mutable Render States */

        /* Sets the view dimensions for rendering, relative to the active render target.
         * It is required to call this at least once after calling SetRenderTargets, as
         * the renderer may need to adjust these dimensions to fit the backend's
         * potentially goofy coordinate systems.
         *
         * viewport: The new view dimensions for future draw calls.
         */
        virtual void SetViewport(const Viewport& viewport, uint32_t slot) = 0;

        /* Sets the scissor box for rendering, relative to the active render target.
         * It is required to call this at least once after calling SetRenderTargets, as
         * the renderer may need to adjust these dimensions to fit the backend's
         * potentially goofy coordinate systems.
         *
         * scissor: The new scissor box for future draw calls.
         */
        virtual void SetScissorRect(Rect scissor) = 0;

        /* Gets the blending factor used for current draw calls.
         *
         * blendFactor: Filled with color being used as the device blend factor.
         */
        virtual void GetBlendFactor(Color& blendFactor) = 0;

        /* Sets the blending factor used for future draw calls.
         *
         * blendFactor: The color to use as the device blend factor.
         */
        virtual void SetBlendFactor(Color blendFactor) = 0;

        /* Gets the mask from which multisample fragment data is sampled from.
         *
         * Returns the coverage mask used to determine sample locations. //TODO
         */
        virtual int32_t GetMultiSampleMask() = 0;

        /* Sets the mask with which multisample fragment data will be sampled from.
         *
         * mask: The new coverage mask to use for determining sample locations. //TODO
         */
        virtual void SetMultiSampleMask(int32_t mask) = 0;

        /* Gets the reference value used for certain types of stencil testing.
         *
         * Returns the stencil reference value.
         */
        //virtual int32_t GetReferenceStencil() = 0; //TODO

        /* Sets the reference value used for certain types of stencil testing.
         *
         * ref: The new stencil reference value. //TODO
         */
        //virtual void SetReferenceStencil(int32_t ref) = 0;

        /* Immutable Render States */

        /* Applies a blending state to use for future draw calls. This only needs to be
         * called when the state actually changes. Redundant calls may negatively affect
         * performance!
         *
         * blendState: The new parameters to use for color blending.
         */
        virtual void SetBlendState(const BlendState& blendState) = 0;

        /* Applies depth/stencil states to use for future draw calls. This only needs to
         * be called when the states actually change. Redundant calls may negatively
         * affect performance!
         *
         * depthStencilState: The new parameters to use for depth/stencil work.
         */
        virtual void SetDepthStencilState(const DepthStencilState& depthStencilState) = 0;

        /* Applies the rasterizing state to use for future draw calls.
         * It's generally a good idea to call this for each draw call, but if you really
         * wanted to you could try reducing it to when the state changes and when the
         * render target state changes.
         *
         * rasterizerState: The new parameters to use for rasterization work.
         */
        virtual void ApplyRasterizerState(const RasterizerState& rasterizerState) = 0;

        /* Updates a sampler slot with new texture/sampler data for future draw calls.
         * This should only be called on slots that have modified texture/sampler state.
         * Redundant calls may negatively affect performance!
         *
         * index:	The sampler slot to update.
         * texture:	The texture bound to this sampler.
         * sampler:	The new parameters to use for this slot's texture sampling.
         */
        virtual void VerifyPixelSampler(int32_t index, const SamplerState& sampler) = 0;
        virtual void VerifyPixelTexture(int32_t index, const Texture* texture) = 0;

        virtual void VerifyUATexture(int32_t index, const Texture* texture) = 0;

        /* Updates a vertex sampler slot with new texture/sampler data for future draw
         * calls. This should only be called on slots that have modified texture/sampler
         * state. Redundant calls may negatively affect performance!
         *
         * index:	The vertex sampler slot to update.
         * texture:	The texture bound to this sampler.
         * sampler:	The new parameters to use for this slot's texture sampling.
         */
        virtual void VerifyVertexSampler(int32_t index, const SamplerState& sampler) = 0;
        virtual void VerifyVertexTexture(int32_t index, const Texture* texture) = 0;

        ///* Updates the vertex attribute state to read from a set of vertex buffers. This
        // * should be the very last thing you call before making a draw call, as this
        // * does all the final prep work for the shader program before it's ready to use.
        // *
        // * bindings:		The vertex buffers and their attribute data.
        // * numBindings:		The number of elements in the bindings array.
        // * bindingsUpdated:	If the bindings array hasn't changed since the last
        // *			update, this can be false. We'll only update the shader
        // *			state, updating vertex attribute data only if we 100%
        // *			have to, for a tiny performance improvement.
        // * baseVertex:		This should be the same as the `baseVertex` parameter  //TODO
        // *			from your Draw*Primitives call, if applicable. Not every
        // *			rendering backend has native base vertex support, so we
        // *			work around it by passing this here.
        // */
        //virtual void ApplyVertexBufferBindings(
        //	VertexBufferBinding* bindings,
        //	int32_t numBindings,
        //	uint8_t bindingsUpdated,
        //	int32_t baseVertex
        //) = 0;

        /* Render Targets */

        /* Sets the color/depth/stencil buffers to write future draw calls to.
         *
         * renderTargets:	The targets to write to, or NULL for the backbuffer.
         * numRenderTargets:	The size of the renderTargets array (can be 0).
         * depthStencilBuffer:	The depth/stencil renderbuffer (can be NULL).
         * depthFormat:		The format of the depth/stencil renderbuffer.
         * preserveTargetContents:
         * 			Set this to 1 to store the color/depth/stencil contents
         * 			for future use. Most of the time you'll want to
         * 			keep this at 0 to not waste GPU bandwidth.
         */
        virtual void SetRenderTargets(
            RenderTargetBinding** renderTargets,
            int32_t numRenderTargets,
            Renderbuffer* depthStencilBuffer
        ) = 0;

        virtual void SetBackBufferViewport(
            const Viewport& viewports
        ) = 0;

        /* After unsetting a render target, call this to resolve multisample targets or
         * generate mipmap data for the final texture.
         *
         * target: The render target to resolve once rendering is complete.
         */
        virtual void ResolveTarget(const RenderTargetBinding& target) = 0;

        /* Backbuffer Functions */

        /* After modifying the OS window state, call this to reset the backbuffer to
         * match your window changes.
         *
         * presentationParameters: The new settings for the backbuffer.
         */
        virtual void ResetBackbuffer(const PresentationParameters& presentationParameters
        ) = 0;

        /* Read the backbuffer's contents directly into client memory. This function is
         * basically one giant CPU/GPU sync point, do NOT ever call this during any
         * performance-critical situation! Just use it for screenshots.
         *
         * x:		The x offset of the backbuffer region to read.
         * y:		The y offset of the backbuffer region to read.
         * w:		The width of the backbuffer region to read.
         * h:		The height of the backbuffer region to read.
         * data:	The pointer to read the backbuffer data into.
         * dataLength:	The size of the backbuffer data in bytes.
         */
        virtual void ReadBackbuffer(
            int32_t x,
            int32_t y,
            int32_t w,
            int32_t h,
            void* data,
            int32_t dataLength
        ) = 0;

        /* Gets the current dimensions of the backbuffer.
         *
         * w:	Filled with the backbuffer's width.
         * h:	Filled with the backbuffer's height.
         */
        virtual void GetBackbufferSize(
            uint32_t& w,
            uint32_t& h
        ) = 0;

        /* Gets the current dimensions of the viewport.
         *
         * w:	Filled with the backbuffer's width.
         * h:	Filled with the backbuffer's height.
         */
        virtual void GetMainViewportSize(
            uint32_t& w,
            uint32_t& h
        ) = 0;

        ///* Gets the current pixel format of the backbuffer.
        // *
        // * Returns the backbuffer's pixel format.
        // */
        //virtual SurfaceFormat GetBackbufferSurfaceFormat() = 0;

        /* Gets the format of the backbuffer's depth/stencil buffer.
         *
         * Returns the backbuffer's depth/stencil format.
         */
        virtual DepthFormat GetBackbufferDepthFormat() = 0;

        ///* Gets the multisample sample count of the backbuffer.
        // *
        // * Returns the backbuffer's multisample sample count.  //TODO
        // */
        //virtual int32_t GetBackbufferMultiSampleCount(Device* device) = 0;

        /* Textures */

        /* Creates a 2D texture to be applied to VerifyPixelSampler.
         *
         * format:		The pixel format of the texture data.
         * width:		The width of the texture image.
         * height:		The height of the texture image.
         * levelCount:		The number of mipmap levels to allocate.
         * isRenderTarget:	Set this to 1 when using this with SetRenderTargets.
         *
         * Returns an allocated Texture* object. Note that the contents of the
         * texture are undefined, so you must call SetData at least once before drawing!
         */
        virtual Texture* CreateTexture2D(
            SurfaceFormat format,
            int32_t width,
            int32_t height,
            int32_t levelCount,
            uint8_t isRenderTarget
        ) = 0;

        virtual Texture* CreateUATexture2D(
            SurfaceFormat format,
            int32_t width,
            int32_t height,
            int32_t levelCount
        ) = 0;

        virtual Texture* CreateTexture2D(
            //SurfaceFormat format,
            int32_t width,
            int32_t height,
            int32_t levelCount,
            int32_t subCount,
            uint8_t isRenderTarget
        ) = 0;

        ///* Creates a 3D texture to be applied to VerifyPixelSampler.
        // *
        // * format:		The pixel format of the texture data.
        // * width:		The width of the texture image.
        // * height:		The height of the texture image.
        // * depth:		The depth of the texture image.
        // * levelCount:		The number of mipmap levels to allocate.
        // *
        // * Returns an allocated Texture* object. Note that the contents of the
        // * texture are undefined, so you must call SetData at least once before drawing!
        // */
        //virtual Texture* CreateTexture3D(
        //	Device* device,
        //	SurfaceFormat format,
        //	int32_t width,
        //	int32_t height,
        //	int32_t depth,
        //	int32_t levelCount
        //) = 0;   //TODO

        /* Creates a texture cube to be applied to VerifyPixelSampler.
         *
         * format:		The pixel format of the texture data.
         * size:		The length of a single edge of the texture cube.
         * levelCount:		The number of mipmap levels to allocate.
         * isRenderTarget:	Set this to 1 when using this with SetRenderTargets.
         *
         * Returns an allocated Texture* object. Note that the contents of the
         * texture are undefined, so you must call SetData at least once before drawing!
         */
        virtual Texture* CreateTextureCube(
            SurfaceFormat format,
            int32_t size,
            int32_t levelCount,
            uint8_t isRenderTarget
        ) = 0; //TODO

        /* Sends a texture to be destroyed by the renderer. Note that we call it
         * "AddDispose" because it may not be immediately destroyed by the renderer if
         * this is not called from the main thread (for example, if a garbage collector
         * deletes the resource instead of the programmer).
         *
         * texture: The Texture to be destroyed.
         */
        virtual void AddDisposeTexture(Texture* texture) = 0;

        /* Uploads image data to a 2D texture object.
         *
         * texture:	The texture to be updated.
         * x:		The x offset of the subregion being updated.
         * y:		The y offset of the subregion being updated.
         * w:		The width of the subregion being updated.
         * h:		The height of the subregion being updated.
         * level:	The mipmap level being updated.
         * data:	A pointer to the image data.
         * dataLength:	The size of the image data in bytes.
         */
        virtual void SetTextureData2D(
            Texture* texture,
            int32_t x,
            int32_t y,
            int32_t w,
            int32_t h,
            int32_t level,
            void* data,
            int32_t dataLength
        ) = 0;

        ///* Uploads image data to a 3D texture object.
        // *
        // * texture:	The texture to be updated.
        // * x:		The x offset of the subregion being updated.
        // * y:		The y offset of the subregion being updated.
        // * z:		The z offset of the subregion being updated.
        // * w:		The width of the subregion being updated.
        // * h:		The height of the subregion being updated.
        // * d:		The depth of the subregion being updated.
        // * level:	The mipmap level being updated.
        // * data:	A pointer to the image data.
        // * dataLength:	The size of the image data in bytes.
        // */
        //virtual void SetTextureData3D(
        //	Texture* texture,
        //	int32_t x,
        //	int32_t y,
        //	int32_t z,
        //	int32_t w,
        //	int32_t h,
        //	int32_t d,
        //	int32_t level,
        //	void* data,
        //	int32_t dataLength
        //) = 0;
        //
        /* Uploads image data to a single face of a texture cube object.
         *
         * texture:	The texture to be updated.
         * x:		The x offset of the subregion being updated.
         * y:		The y offset of the subregion being updated.
         * w:		The width of the subregion being updated.
         * h:		The height of the subregion being updated.
         * cubeMapFace:	The face of the cube being updated.
         * level:	The mipmap level being updated.
         * data:	A pointer to the image data.
         * dataLength:	The size of the image data in bytes.
         */
        virtual void SetTextureDataCube(
            Texture* texture,
            int32_t x,
            int32_t y,
            int32_t w,
            int32_t h,
            int32_t cubeMapFace,
            int32_t level,
            void* data,
            int32_t dataLength
        ) = 0;

        ///* Uploads YUV image data to three ALPHA8 texture objects.
        // *
        // * y:		The texture storing the Y data.
        // * u:		The texture storing the U (Cb) data.
        // * v:		The texture storing the V (Cr) data.
        // * yWidth:	The width of the Y plane.
        // * yHeight:	The height of the Y plane.
        // * uvWidth:	The width of the U/V planes.
        // * uvHeight:	The height of the U/V planes.
        // * data:	A pointer to the raw YUV image data.
        // * dataLength:	The size of the image data in bytes.
        // */
        //virtual void SetTextureDataYUV(
        //	Texture* y,
        //	Texture* u,
        //	Texture* v,
        //	int32_t yWidth,
        //	int32_t yHeight,
        //	int32_t uvWidth,
        //	int32_t uvHeight,
        //	void* data,
        //	int32_t dataLength
        //) = 0;

        /* Pulls image data from a 2D texture into client memory. Like any GetData,
         * this is generally asking for a massive CPU/GPU sync point, don't call this
         * unless there's absolutely no other way to use the image data!
         *
         * texture:	The texture object being read.
         * x:		The x offset of the subregion being read.
         * y:		The y offset of the subregion being read.
         * w:		The width of the subregion being read.
         * h:		The height of the subregion being read.
         * level:	The mipmap level being read.
         * data:	The pointer being filled with the image data.
         * dataLength:	The size of the image data in bytes.
         */
        virtual void GetTextureData2D(
            const Texture* texture,
            int32_t x,
            int32_t y,
            int32_t w,
            int32_t h,
            int32_t level,
            void* data,
            int32_t dataLength
        ) = 0;

        virtual void* GetNativeTexture(Texture* Texture) = 0;

        ///* Pulls image data from a 3D texture into client memory. Like any GetData,
        // * this is generally asking for a massive CPU/GPU sync point, don't call this
        // * unless there's absolutely no other way to use the image data!
        // *
        // * texture:	The texture object being read.
        // * x:		The x offset of the subregion being read.
        // * y:		The y offset of the subregion being read.
        // * z:		The z offset of the subregion being read.
        // * w:		The width of the subregion being read.
        // * h:		The height of the subregion being read.
        // * d:		The depth of the subregion being read.
        // * level:	The mipmap level being read.
        // * data:	The pointer being filled with the image data.
        // * dataLength:	The size of the image data in bytes.
        // */
        //virtual void GetTextureData3D(
        //	Device* device,
        //	Texture* texture,
        //	int32_t x,
        //	int32_t y,
        //	int32_t z,
        //	int32_t w,
        //	int32_t h,
        //	int32_t d,
        //	int32_t level,
        //	void* data,
        //	int32_t dataLength
        //) = 0;

        ///* Pulls image data from a single face of a texture cube object into client
        // * memory. Like any GetData, this is generally asking for a massive CPU/GPU sync
        // * point, don't call this unless there's absolutely no other way to use the
        // * image data!
        // *
        // * texture:	The texture object being read.
        // * x:		The x offset of the subregion being read.
        // * y:		The y offset of the subregion being read.
        // * w:		The width of the subregion being read.
        // * h:		The height of the subregion being read.
        // * cubeMapFace:	The face of the cube being read.
        // * level:	The mipmap level being read.
        // * data:	The pointer being filled with the image data.
        // * dataLength:	The size of the image data in bytes.
        // */
        //virtual void GetTextureDataCube(
        //	Device* device,
        //	Texture* texture,
        //	int32_t x,
        //	int32_t y,
        //	int32_t w,
        //	int32_t h,
        //	CubeMapFace cubeMapFace,
        //	int32_t level,
        //	void* data,
        //	int32_t dataLength
        //) = 0;

        /* Renderbuffers */

        /* Creates a color buffer to be used by SetRenderTargets/ResolveTarget.
         *
         * width:		The width of the color buffer.
         * height:		The height of the color buffer.
         * format:		The pixel format of the color buffer.
         * multiSampleCount:	The MSAA value for the color buffer.
         * texture:		The texture that this buffer will be resolving to.
         *
         * Returns a color Renderbuffer object.
         */
        virtual Renderbuffer* GenColorRenderbuffer(
            int32_t width,
            int32_t height,
            SurfaceFormat format,
            int32_t multiSampleCount,
            Texture* texture
        ) = 0;

        /* Creates a depth/stencil buffer to be used by SetRenderTargets.
         *
         * width:		The width of the depth/stencil buffer.
         * height:		The height of the depth/stencil buffer.
         * format:		The storage format of the depth/stencil buffer.
         * multiSampleCount:	The MSAA value for the depth/stencil buffer.
         *
         * Returns a depth/stencil Renderbuffer object.
         */
        virtual Renderbuffer* GenDepthStencilRenderbuffer(
            int32_t width,
            int32_t height,
            DepthFormat format,
            int32_t multiSampleCount
        ) = 0;

        /* Sends a renderbuffer to be destroyed by the renderer. Note that we call it
         * "AddDispose" because it may not be immediately destroyed by the renderer if
         * this is not called from the main thread (for example, if a garbage collector
         * deletes the resource instead of the programmer).
         *
         * renderbuffer: The Renderbuffer to be destroyed.
         */
        virtual void AddDisposeRenderbuffer(Renderbuffer* renderbuffer) = 0;

        /* Vertex Buffers */

        /* Creates a vertex buffer to be used by Draw*Primitives.
         *
         * dynamic:		Set to 1 if this buffer will be updated frequently.
         * usage:		Set to WRITEONLY if you do not intend to call GetData.
         * sizeInBytes:		The length of the vertex buffer.
         * 
         * Returns an allocated Buffer* object. Note that the contents of the
         * buffer are undefined, so you must call SetData at least once before drawing!
         */
        virtual Buffer* GenVertexBuffer(
            uint8_t dynamic,
            BufferUsage usage,
            int32_t sizeInBytes
        ) = 0;

        /* Sends a vertex buffer to be destroyed by the renderer. Note that we call it
         * "AddDispose" because it may not be immediately destroyed by the renderer if
         * this is not called from the main thread (for example, if a garbage collector
         * deletes the resource instead of the programmer).
         *
         * buffer: The Buffer to be destroyed.
         */
        virtual void AddDisposeVertexBuffer(Buffer* buffer) = 0;

        /* Sets a region of the vertex buffer with client data.
         *
         * buffer:		The vertex buffer to be updated.
         * offsetInBytes:	The starting offset of the buffer to write into.
         * data:		The client data to write into the buffer.
         * elementCount:	The number of elements from the client buffer to write.
         * elementSizeInBytes:	The size of each element in the client buffer.
         * vertexStride:	Try to set this to the same value as elementSizeInBytes.
         *			XNA has this ridiculous thing where if vertexStride is
         *			greater than elementSizeInBytes, it tries to do partial
         *			updates of each vertex with the client data's smaller
         *			elements. It's... just, really bad. Don't try to use it.
         *			You probably just want '1' for both parameters, so that
         *			elementCount can just be the buffer length in bytes.
         * options:		Try not to call NONE if this is a dynamic buffer!
         */
        virtual void SetVertexBufferData(
            Buffer* buffer,
            int32_t offsetInBytes,
            void* data,
            int32_t elementCount,
            int32_t elementSizeInBytes,
            int32_t vertexStride,
            SetDataOptions options
        ) = 0;

        /* Pulls data from a region of the vertex buffer into a client pointer.
         *
         * buffer:		The vertex buffer to be read from.
         * offsetInBytes:	The starting offset of the buffer to write into.
         * data:		The client data to write into from the buffer.
         * elementCount:	The number of elements from the client buffer to read.
         * elementSizeInBytes:	The size of each element in the client buffer.
         * vertexStride:	Try to set this to the same value as elementSizeInBytes.
         *			XNA has this ridiculous thing where if vertexStride is
         *			greater than elementSizeInBytes, it tries to do partial
         *			updates of each vertex with the client data's smaller
         *			elements. It's... just, really bad. Don't try to use it.
         *			You probably just want '1' for both parameters, so that
         *			elementCount can just be the buffer length in bytes.
         */
        virtual void GetVertexBufferData(
            const Buffer* buffer,
            int32_t offsetInBytes,
            void* data,
            int32_t elementCount,
            int32_t elementSizeInBytes,
            int32_t vertexStride
        ) = 0;

        /* Index Buffers */

        /* Creates an index buffer to be used by Draw*Primitives.
         *
         * dynamic:		Set to 1 if this buffer will be updated frequently.
         * usage:		Set to WRITEONLY if you do not intend to call GetData.
         * sizeInBytes:		The length of the vertex buffer.
         *
         * Returns an allocated Buffer* object. Note that the contents of the
         * buffer are undefined, so you must call SetData at least once before drawing!
         */
        virtual Buffer* GenIndexBuffer(
            uint8_t dynamic,
            BufferUsage usage,
            int32_t sizeInBytes
        ) = 0;

        /* Sends an index buffer to be destroyed by the renderer. Note that we call it
         * "AddDispose" because it may not be immediately destroyed by the renderer if
         * this is not called from the main thread (for example, if a garbage collector
         * deletes the resource instead of the programmer).
         *
         * buffer: The Buffer to be destroyed.
         */
        virtual void AddDisposeIndexBuffer(Buffer* buffer) = 0;

        /* Sets a region of the index buffer with client data.
         *
         * buffer:		The index buffer to be updated.
         * offsetInBytes:	The starting offset of the buffer to write into.
         * data:		The client data to write into the buffer.
         * dataLength:		The size (in bytes) of the client data.
         * options:		Try not to call NONE if this is a dynamic buffer!
         */
        virtual void SetIndexBufferData(
            Buffer* buffer,
            int32_t offsetInBytes,
            void* data,
            int32_t dataLength,
            SetDataOptions options
        ) = 0;

        /* Pulls data from a region of the index buffer into a client pointer.
         *
         * buffer:		The index buffer to be read from.
         * offsetInBytes:	The starting offset of the buffer to read from.
         * data:		The pointer to read buffer data into.
         * dataLength:		The size (in bytes) of the client data.
         */
        virtual void GetIndexBufferData(
            const Buffer* buffer,
            int32_t offsetInBytes,
            void* data,
            int32_t dataLength
        ) = 0;


        virtual PixelShader* CompilePixelShader(
          const ShaderCompileData& shaderData
      ) = 0;

        virtual ComputeShader* CompileComputeShader(
            const ShaderCompileData& shaderData
        ) = 0;


        virtual GeometryShader* CompileGeometryShader(
            const ShaderCompileData& shaderData
        ) = 0;

        virtual VertexShader* CompileVertexShader(
            const ShaderCompileData& shaderData,
            void* inpitLayout,
            size_t inputLayoutSize
        ) = 0;

        

        virtual void Dispatch(size_t x, size_t y, size_t z = 1) = 0;

        virtual void ApplyVertexBufferBinding(const VertexBufferBinding& bindings) = 0;
        virtual void ApplyIndexBufferBinding(const Buffer* indices, uint8_t indexElementSize) = 0;
        virtual void ApplyMeshBuffersBinding(const MeshBindings& bindings) = 0;


        virtual void ApplyPixelShader(PixelShader* pixelShader) = 0;
        virtual void ApplyVertexShader(VertexShader* vertexShader) = 0;
        virtual void ApplyGeometryShader(GeometryShader* vertexShader) = 0;
        virtual void ApplyComputeShader(ComputeShader* vertexShader) = 0;

        virtual void AddDisposePixelShader(PixelShader* pixelShader) = 0;
        virtual void AddDisposeVertexShader(VertexShader* vertexShader) = 0;
        virtual void AddDisposeGeometryShader(GeometryShader* pixelShader) = 0;
        virtual void AddDisposeComputeShader(ComputeShader* vertexShader) = 0;

        virtual ConstBuffer* CreateConstBuffer(size_t size) = 0;
        virtual void VerifyConstBuffer(ConstBuffer* constBuffer, size_t slot) = 0;
        virtual void SetConstBuffer(ConstBuffer* constBuffers, void* data) = 0;
        virtual void AddDisposeConstBuffer(ConstBuffer* constBuffers) = 0;

        virtual void ApplyPipelineState(PipelineState piplineState) = 0;
        virtual void Flush() = 0;

        virtual void BeginEvent(const char* name) = 0;
        virtual void EndEvent() = 0;
        virtual void SetMarker(const char* name) = 0;

        virtual void ClearState() = 0;
    };

 
};
