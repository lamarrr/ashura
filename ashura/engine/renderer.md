# REQUIREMENTS

- Custom/Authored Materials support
- Custom/Authored Material Shaders (Fragment & Vertex) & Pipelines Support
- Automatic creation, caching, and management of resources: Framebuffer, renderpasses
- Automatic re-use of framebuffer attachments
- Automatic graph resource cleanup without stalling GPU every frame
- Custom/Authored Pass Support
- Automatic Descriptor set and push constant creation and management:
  - Creation
  - Binding
  - Updating
  - Destruction
- Custom/Authored Shader Parameters, Pass Parameters
- Batching and Instancing Support
- Core Passes:
  - Gaussian Bloom
  - Blur
  - Chromatic Aberration
  - Ambient Occlusion
  - FXAA and MSAA
  - Bokeh or Guassian Depth Of Field
  - GBuffer Pass (Sorted by Pipeline and Parameters)
    - Opaque
    - Transparent
- Core Material Shaders
  - PBR
  - RRect
  - Quad: Quad Vertex Shader + Fragment Shader (+Transforms + Instancing)
  - Highlight with Stencil
  - Fresnel-Effect Highlight

# Required Samples

- Custom YAML-defined Imported Materials for Weapons
- Player or Object Highlighting / Fresnel Effect
- Gizmo
- Animation Blending
- Lens (Flare) Effect
- Screen Space Reflection
- CRT Shader
- Ambient Occlusion
- Parallax
https://github.com/GPUOpen-LibrariesAndSDKs/RenderPipelineShaders
https://github.com/GPUOpen-LibrariesAndSDKs/FidelityFX-SDK