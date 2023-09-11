#include "ashura/lgfx.h"

// ash::lgfx::rid ash::lgfx::Graph::create_buffer(ash::lgfx::BufferDesc const &desc)
// {
//   ASH_CHECK(buffers.size() < ash::lgfx::RID_MAX - 1);
//   ash::lgfx::rid buffer_id = (ash::lgfx::rid) buffers.size();
//   buffers.push_inplace(desc).unwrap();
//   return buffer_id;
// }

// ash::lgfx::rid ash::lgfx::Graph::create_image(ash::lgfx::ImageDesc const &desc)
// {
//   ASH_CHECK((desc.usages & ImageUsages::ColorAttachment & ImageUsages::DepthStencilAttachment) != (ImageUsages::ColorAttachment | ImageUsages::DepthStencilAttachment));
//   ASH_CHECK(images.size() < ash::lgfx::RID_MAX - 1);
//   ASH_CHECK(desc.extent.is_visible());
//   ash::lgfx::rid image_id = (ash::lgfx::rid) images.size();
//   images.push_inplace(desc).unwrap();
//   return image_id;
// }

// ash::lgfx::rid ash::lgfx::Graph::create_render_pass(ash::lgfx::RenderPassDesc const &desc)
// {
//   // ASH_CHECK(ash::lgfx::is_color_format(desc.color_format));
//   // ASH_CHECK(desc.depth_stencil_format == ash::lgfx::ImageFormat::Undefined || desc.depth_stencil_format == ash::lgfx::ImageFormat::D16_Unorm);
//   ASH_CHECK(render_passes.size() < ash::lgfx::RID_MAX - 1);
//   ash::lgfx::rid render_pass_id = (ash::lgfx::rid) render_passes.size();
//   render_passes.push_inplace(desc).unwrap();
//   return render_pass_id;
// }

// ash::lgfx::rid ash::lgfx::Graph::create_framebuffer(ash::lgfx::FramebufferDesc const &desc)
// {
//   // ASH_CHECK(framebuffers.size() < ash::lgfx::RID_MAX - 1);
//   ASH_CHECK(desc.color == ash::lgfx::RID_MAX || (desc.color < images.size()));
//   // ASH_CHECK(desc.depth_stencil == ash::lgfx::RID_MAX || (desc.depth_stencil < images.size()));
//   // ASH_CHECK(desc.render_pass < render_passes.size());
//   ASH_CHECK(desc.color == ash::lgfx::RID_MAX || render_passes[desc.render_pass].color_format == images[desc.color].format);
//   ASH_CHECK(desc.depth_stencil == ash::lgfx::RID_MAX || render_passes[desc.render_pass].depth_stencil_format == images[desc.depth_stencil].format);

//   // ash::lgfx::rid framebuffer_id = (ash::lgfx::rid) framebuffers.size();
//   // framebuffers.push_inplace(desc).unwrap();
//   return 0;
// }
