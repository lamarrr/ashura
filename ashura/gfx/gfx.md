## REQUIREMENTS

- Automatic synchronization
- Custom allocator support
- Abstract boiler-plate vulkan code
- Abstract descriptor set management
- Abstract over API implementation detail differences

# Aliasing Rule

- A resource may only be used in one state in a pass. i.e. an image can not be sampled and used as a color attachment in a renderpass.
