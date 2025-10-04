## REQUIREMENTS

- Automatic resource synchronization
- Custom allocator and logger support
- Abstract boiler-plate vulkan code
- Abstract descriptor set management
- Abstract over API implementation detail differences

# Resource Aliasing Rules

- A resource can only be used in one stage and state of a pipeline at a time. i.e. an image may only be used as a sampled image (and not as a storage image at the same time) at a point in time and in only one stage 
