
 TODO(lamarrr):
 sort by update frequency, per-frame updates, rare-updates

 resource manager
 static buffer + streaming
 dynamic buffers + streaming

 mapping of color and depth components?


 usage tracking
 - we can create a single image and just re-use it depending on the
 components/aspects we need to use for each type of pass

 UNIFORM COLOR Texture cache with image component swizzling. Only 1 white
 RGBA texture is needed.

 on frame begin, pending uploads are first performed

 Manages and uploads render resources to the GPU.




 TODO(lamarrr): how to automate and manage per-view resources across all views
 with deletion as well.



 TODO(lamarrr): how to cache the framebuffer and
 renderpass and not allocate it for every time the renderpass and
 framebuffers are requested

 TODO(lamarrr): multi-recursive passes, and how to know when to begin and end
 passes, i.e. begin render pass, end render pass


 TODO(lamarrr): view pass recursive render


 full-screen post-fx passes are full-screen quads with dependency
 determined by their z-indexes. HUD is a full-screen quad of a view-pass
 (another scene).

 world->[capture->world]->post-fx->hud->[capture->hud]
 how to project from object-space to full-screen space

 i.e. world scene pass -> post-fx pass -> HUD pass

 how to queue screen image resize




 TODO(lamarrr): we need the mesh and object render-data is mostly
 pre-configured or modified outside the renderer we just need to implement the
 post-effects and render-orders and add other passes on top of the objects

 TODO(lamarrr): each scene is rendered and composited onto one another? can
 this possibly work for portals?



 TODO(lamarrr): can we do more specialized clips?




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
  