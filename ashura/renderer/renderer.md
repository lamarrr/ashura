# REQUIREMENTS

- Custom/Authored Materials support
- Custom/Authored Material Shaders (Fragment & Vertex) & Pipelines Support
- Automatic creation, caching, and management of resources: Framebuffer, renderpasses
- Automatic re-use of framebuffer attachments
- Automatic graph resource cleanup without stalling GPU every frame
- Custom/Authored Pass Support
- Custom/Authored Shader Parameters, Pass Parameters, Descriptor set descriptions, creation, binding, and, updating
- Batching and Instancing Support
- Core Passes:
    - Bloom
    - Blur
    - Chromatic Aberration
    - Ambient Occlusion
    - FXAA
    - MSAA
    - Bokeh Depth Of Field
    - Guassian Depth Of Field
    - GBuffer Pass (Sorted by Pipeline, S)
      - Opaque
      - Transparent
- Core Material Shaders
  - PBR
  - RRect
  - Highlight with Stencil
  - Fresnel-Effect Highlight


Required Samples
- Custom JSON-defined Imported Materials for Weapons
- Player or Object Highlighting




TODO(lamarrr): we need the mesh and object render-data is mostly
pre-configured or modified outside the renderer we just need to implement the
post-effects and render-orders and add other passes on top of the objects

quad + instanced + transformed + antialiased

Quad is an object without a type or specification. a unit square with just a
transformation matrix.

Quads don't need vertex/index buffers. only shaders + gl_Index into
shader-stored vertices

offscreen passes - run offscreen passes in the update function? what about
pass data and context? bool is_offscreen?

how will offscreen rendering work? separate scene? - it must not be a
separate scene, it is left to the pass to decide allocate own texture for
rendering, then call passes of the objects to be rendered onto the allocated
framebuffer void render(...); the objects will be added during render?
will also need separate coordinates? or transformed?
HOW TO REUSE PASSES IN OFFSCREEN PASSES

- temp-offscreen rendering: request scratch image with maximum
  size of the viewport size and specific format, not released until execution
  completes, the pass doesn't need to release it so other passes can re-use it
  if needed.

- offscreen pass will store the objects + their actual rendering pass and
  invoke the actual pass when rendering is needed. these are sorted by sub-pass
  again. we then invoke the actual passes with the frame buffer and location we
  need to render to? WROOONNGGGGG - the subpass will also need to store info
  and track data of the objects
  recursive offscreen?

GUI blur for example needs to capture the whole scene at one point and then
render to screen (Layer)

Store last capture z-index + area, if non-intersecting and re-usable re-use

todo(lamarrr): multiple framebuffers? should it be
stored here? since we are allocating scratch images, we would need to
recreate the framebuffers every frame [scene, pass] association cos we need
to be able to dispatch for several types of scenes (offscreen and
onscreen?)

i.e. blur on offscreen layer

store attachments for each scene in the scene group. prepare to render for
each.

TODO(lamarrr): clipping
