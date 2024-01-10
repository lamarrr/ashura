- HIGH-LEVEL RENDER & COMPUTE PIPELINE COMPONENTS (ABSTRACTION)
- EFFECTS & POST-PROCESSING
- MESH MANAGEMENT
- MESH BATCHING & INSTANCING
- MATERIAL MANAGEMENT
- RESOURCE MANAGEMENT
- CAMERA MANAGEMENT
- LIGHT MANAGEMENT
- SCENE GRAPH (SORTING, CULLING)
- MID-LEVEL RENDER & COMPUTE PIPELINE COMPONENTS
- RESOURCE SYNCHRONIZATION & MANAGEMENT (I.E. BARRIERS)
- TASK GRAPHS
- LOW-LEVEL RENDER & COMPUTE PIPELINE COMPONENTS (PLATFORM-SPECIFIC)
- RENDER PASSES
- COMPUTE PASSES
- PIPELINES
- SHADERS
- PSO & PSO CACHES




3D scene objects (certain effects? offscreen rendering? PBR)
- object + shader + pipeline +  (offscreen) pass + uniform data
- Screen-space rendered objects
- Meshes
- Splines & Curves
- 3D Path Rendering (+Splines & Curves)
- Lights
- Portals
- Reflections
- Shadows
- Reflection Probes
- Particle Effects (custom compute shader-based)

Screen-space objects
- object + (offscreen) pass, custom shaders
- 2D Path Rendering (+Splines & Curves)

post process effects
- TAA
- FXAA
- Gaussian Blur
- Bloom
- Chromatic Aberration (https://www.shadertoy.com/view/Mds3zn)
- Bokeh Depth of Field

Renderer
==========
- Inputs: 3D Scene, 2D Scene, VFX, Screen Space UI
- Outputs: Framebuffer
- Viewports (Subtrees)

Scene
======
Defines objects, their interactions and effects on the scene
- Inputs:
    - Objects+Clip Mask: Meshes, Particles, & Materials, Screen-space or World-space Position, (Grass, Orbs)
    - Lights
    - Cameras
    - Portals
    - Reflection Probes

Material Shader System
========================
- PBR (textures or values)
- Plain colors

Custom Passes
- Some passes will be customisable with custom shaders

Custom Shaders

 --> shader library pack --> composed shader code (inputs linked to material system) (vertex + mesh)


vertex shader
fragment shader





geometry pass?
compute pass? 

Material{
    hashmap(<string, float>,
    <string, vec2>,
    <string, vec3>,
    <string, vec4>,
    <string, mat2>,
    <string, mat3>,
    <string, mat4>,
    <string, texture>,
    <string, buffer>,
    <string, buffer_view>
    )
}

Pass{
    add_object(mesh, material)
}

- PBR shader for example

--> PBR pass --> PBR shader pack

