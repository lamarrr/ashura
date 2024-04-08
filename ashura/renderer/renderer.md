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
  - GBuffer Pass (Sorted by Pipeline and Paramters)
    - Opaque
    - Transparent
  - Clipped/Stencil Composite
- Core Material Shaders
  - PBR
  - RRect
  - Quad: Quad Vertex Shader + Fragment Shader (+Transforms + Instancing)
  - Highlight with Stencil
  - Fresnel-Effect Highlight

# Render Graph Requirements

- Caching of Renderpass
- Caching of Framebuffer?
- Re-use of scratch image attachments across passes
- Caching and management of pipeline along with shaders and renderpasses

# Required Samples

- Custom YAML-defined Imported Materials for Weapons
- Player or Object Highlighting
