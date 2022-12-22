#pragma once
#include <filesystem>
#include <fstream>
#include <string_view>

#include "ashura/canvas.h"
#include "ashura/image.h"
#include "ashura/primitives.h"
#include "ashura/vulkan.h"
#include "freetype/freetype.h"
#include "hb.h"
#include "stx/allocator.h"
#include "stx/limits.h"
#include "stx/span.h"
#include "stx/string.h"
#include "stx/vec.h"

namespace asr {

enum class TextAlign : u8 { Left, Right, Center };

enum class WordWrap : u8 { None, Wrap };

struct TextStyle {
  f32 font_height = 10;
  f32 letter_spacing = 0;
  f32 word_spacing = 0;
  hb_direction_t direction = HB_DIRECTION_LTR;
  TextAlign align = TextAlign::Left;
  WordWrap word_wrap = WordWrap::None;
  u32 num_tab_spaces = 4;
};

struct Font {
  static constexpr hb_tag_t KERNING_FEATURE =
      HB_TAG('k', 'e', 'r', 'n');  // kerning operations
  static constexpr hb_tag_t LIGATURE_FEATURE =
      HB_TAG('l', 'i', 'g', 'a');  // standard ligature substitution
  static constexpr hb_tag_t CONTEXTUAL_LIGATURE_FEATURE =
      HB_TAG('c', 'l', 'i', 'g');  // contextual ligature substitution

  hb_face_t* hbface = nullptr;
  hb_font_t* hbfont = nullptr;
  hb_buffer_t* hbscratch_buffer = nullptr;
  FT_Library ftlib = nullptr;
  FT_Face ftface = nullptr;

  Font(hb_face_t* ahbface, hb_font_t* ahbfont, hb_buffer_t* ahbscratch_buffer,
       FT_Library aftlib, FT_Face aftface)
      : hbface{ahbface},
        hbfont{ahbfont},
        hbscratch_buffer{ahbscratch_buffer},
        ftlib{aftlib},
        ftface{aftface} {}

  STX_MAKE_PINNED(Font)

  ~Font() {
    hb_face_destroy(hbface);
    hb_font_destroy(hbfont);
    hb_buffer_destroy(hbscratch_buffer);
    ASR_CHECK(FT_Done_Face(ftface) == 0);
    ASR_CHECK(FT_Done_FreeType(ftlib) == 0);
  }
};

enum class FontLoadError { InvalidPath };

stx::Result<stx::Rc<Font*>, FontLoadError> load_font(std::string_view path) {
  if (!std::filesystem::exists(path)) {
    return stx::Err(FontLoadError::InvalidPath);
  }

  std::ifstream stream{path, std::ios::ate | std::ios_base::binary};

  usize size = stream.tellg();
  stream.seekg(0);

  stx::Memory memory = stx::mem::allocate(stx::os_allocator, size).unwrap();

  stream.read(static_cast<char*>(memory.handle), size);

  stream.close();

  hb_blob_t* hbblob = hb_blob_create(static_cast<char*>(memory.handle), size,
                                     HB_MEMORY_MODE_READONLY, nullptr, nullptr);
  ASR_CHECK(hbblob != nullptr);

  hb_face_t* hbface = hb_face_create(hbblob, 0);
  ASR_CHECK(hbface != nullptr);

  hb_font_t* hbfont = hb_font_create(hbface);
  ASR_CHECK(hbfont != nullptr);

  FT_Library ftlib = nullptr;
  ASR_CHECK(FT_Init_FreeType(&ftlib) == 0);

  FT_Face ftface = nullptr;
  ASR_CHECK(FT_New_Memory_Face(ftlib, static_cast<FT_Byte*>(memory.handle),
                               size, 0, &ftface) == 0);

  hb_buffer_t* hbscratch_buffer = hb_buffer_create();
  ASR_CHECK(hbscratch_buffer != nullptr);

  return stx::Ok(stx::rc::make_inplace<Font>(stx::os_allocator, hbface, hbfont,
                                             hbscratch_buffer, ftlib, ftface)
                     .unwrap());
};

// stores codepoint glyphs for a font at a specific font height
struct FontCacheEntry {
  u32 codepoint;
  offset offset;
  extent extent;
  f32 s0 = 0, s1 = 0, t0 = 0, t1 = 0;
};

struct FontCache {
  stx::Vec<FontCacheEntry> entries{stx::os_allocator};
  extent extent;
  u32 font_height = 40;
  Image atlas;
};

FontCache rasterize_font(Font& font, vk::RecordingContext& ctx,
                         u32 font_height) {
  ASR_CHECK(FT_Set_Char_Size(font.ftface, 0, font_height * 64, 72, 72) == 0);

  vk::CommandQueue& cqueue = *ctx.queue.value().handle;
  VkDevice dev = cqueue.device.handle->device;
  VkPhysicalDeviceMemoryProperties const& memory_properties =
      cqueue.device.handle->phy_device.handle->memory_properties;

  stx::Vec<FontCacheEntry> cache_entries{stx::os_allocator};

  // we use column layout because writing the rendered glyphs to it will
  // be faster and loading from memory during rendering will be faster as
  // opposed to laying them all out on one row which will be extremely slow as
  // unrelated pixel rows will be loaded into memory

  extent cache_extent;

  {
    FT_UInt agindex;

    FT_ULong codepoint = FT_Get_First_Char(font.ftface, &agindex);

    while (agindex != 0) {
      ASR_CHECK(FT_Load_Glyph(font.ftface, codepoint, 0) == 0);
      u32 width = font.ftface->glyph->bitmap.width;
      u32 height = font.ftface->glyph->bitmap.rows;

      offset offset{0, cache_extent.height};

      cache_extent.width = std::max(cache_extent.width, width);
      cache_extent.height += height;

      font.ftface->glyph->bitmap_left;
      font.ftface->glyph->bitmap_top;

      cache_entries
          .push(FontCacheEntry{.codepoint = codepoint,
                               .offset = offset,
                               .extent = {width, height},
                               .s0 = 0,
                               .s1 = 0,
                               .t0 = 0,
                               .t1 = 0})
          .unwrap();

      codepoint = FT_Get_Next_Char(font.ftface, codepoint, &agindex);
    }
  }

  for (FontCacheEntry& entry : cache_entries) {
    entry.s0 = AS_F32(entry.offset.x) / cache_extent.width;
    entry.s1 = AS_F32(entry.offset.x + entry.extent.width) / cache_extent.width;
    entry.t0 = AS_F32(entry.offset.y) / cache_extent.height;
    entry.t1 =
        AS_F32(entry.offset.y + entry.extent.height) / cache_extent.height;
  }

  vk::Buffer cache_staging_buffer =
      vk::create_host_buffer(dev, memory_properties, cache_extent.area() * 4,
                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

  for (FontCacheEntry const& entry : cache_entries) {
    ASR_CHECK(FT_Load_Glyph(font.ftface, entry.codepoint, 0) == 0);
    ASR_CHECK(FT_Render_Glyph(font.ftface, FT_RENDER_MODE_NORMAL) == 0);

    u8* out = static_cast<u8*>(cache_staging_buffer.memory_map);

    for (usize j = entry.offset.y; j < entry.offset.y + entry.extent.height;
         j++) {
      // copy the glyph to the atlas
      for (usize i = entry.offset.x; i < entry.offset.x + entry.extent.width;
           i++) {
        out[0] = 0xFF;
        out[1] = 0xFF;
        out[2] = 0xFF;
        out[3] = font.ftface->glyph
                     ->bitmap[(j - entry.offset.y) * entry.extent.width +
                              (i - entry.offset.x)];
        out += 4;
      }
      // fill the unused portion of the slot with transparent pixels
      for (usize i = entry.offset.x + entry.extent.width;
           i < cache_extent.width; i++) {
        out[0] = 0xFF;
        out[1] = 0xFF;
        out[2] = 0xFF;
        out[3] = 0x00;
        out += 4;
      }
    }
  }

  VkImageCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .imageType = VK_IMAGE_TYPE_2D,
      .format = VK_FORMAT_R8G8B8A8_SRGB,
      .extent = VkExtent3D{.width = cache_extent.width,
                           .height = cache_extent.height,
                           .depth = 1},
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .tiling = VK_IMAGE_TILING_OPTIMAL,
      .usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = 0,
      .pQueueFamilyIndices = nullptr,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED};

  VkImage image;

  ASR_VK_CHECK(vkCreateImage(dev, &create_info, nullptr, &image));

  VkMemoryRequirements memory_requirements;

  vkGetImageMemoryRequirements(dev, image, &memory_requirements);

  u32 memory_type_index =
      find_suitable_memory_type(memory_properties, memory_requirements,
                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
          .unwrap();

  VkMemoryAllocateInfo alloc_info{
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .pNext = nullptr,
      .allocationSize = memory_requirements.size,
      .memoryTypeIndex = memory_type_index};

  VkDeviceMemory memory;

  ASR_VK_CHECK(vkAllocateMemory(dev, &alloc_info, nullptr, &memory));

  ASR_VK_CHECK(vkBindImageMemory(dev, image, memory, 0));

  VkImageViewCreateInfo view_create_info{
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .image = image,
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = format,
      .components = VkComponentMapping{.r = VK_COMPONENT_SWIZZLE_IDENTITY,
                                       .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                                       .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                                       .a = VK_COMPONENT_SWIZZLE_IDENTITY},
      .subresourceRange =
          VkImageSubresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                  .baseMipLevel = 0,
                                  .levelCount = 1,
                                  .baseArrayLayer = 0,
                                  .layerCount = 1}};

  VkImageView view;

  ASR_VK_CHECK(vkCreateImageView(dev, &view_create_info, nullptr, &view));

  VkCommandBufferBeginInfo cmd_buffer_begin_info{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .pNext = nullptr,
      .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
      .pInheritanceInfo = nullptr};

  ASR_VK_CHECK(
      vkBeginCommandBuffer(ctx.upload_cmd_buffer, &cmd_buffer_begin_info));

  VkImageMemoryBarrier pre_upload_barrier{
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      .pNext = nullptr,
      .srcAccessMask = VK_ACCESS_NONE_KHR,
      .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
      .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image = image,
      .subresourceRange =
          VkImageSubresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                  .baseMipLevel = 0,
                                  .levelCount = 1,
                                  .baseArrayLayer = 0,
                                  .layerCount = 1}};

  vkCmdPipelineBarrier(ctx.upload_cmd_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                       VK_PIPELINE_STAGE_TRANSFER_BIT,
                       VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1,
                       &pre_upload_barrier);

  VkBufferImageCopy copy{
      .bufferOffset = 0,
      .bufferRowLength = 0,
      .bufferImageHeight = 0,
      .imageSubresource =
          VkImageSubresourceLayers{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                   .mipLevel = 0,
                                   .baseArrayLayer = 0,
                                   .layerCount = 1},
      .imageOffset = VkOffset3D{.x = 0, .y = 0, .z = 0},
      .imageExtent = VkExtent3D{.width = cache_extent.width,
                                .height = cache_extent.height,
                                .depth = 1}};

  vkCmdCopyBufferToImage(ctx.upload_cmd_buffer, cache_staging_buffer.buffer,
                         image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);

  VkImageMemoryBarrier post_upload_barrier{
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      .pNext = nullptr,
      .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
      .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
      .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image = image,
      .subresourceRange =
          VkImageSubresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                  .baseMipLevel = 0,
                                  .levelCount = 1,
                                  .baseArrayLayer = 0,
                                  .layerCount = 1}};

  vkCmdPipelineBarrier(ctx.upload_cmd_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                       VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                       VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1,
                       &post_upload_barrier);

  ASR_VK_CHECK(vkEndCommandBuffer(ctx.upload_cmd_buffer));

  VkSubmitInfo submit_info{.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                           .pNext = nullptr,
                           .waitSemaphoreCount = 0,
                           .pWaitSemaphores = nullptr,
                           .pWaitDstStageMask = nullptr,
                           .commandBufferCount = 1,
                           .pCommandBuffers = &ctx.upload_cmd_buffer,
                           .signalSemaphoreCount = 0,
                           .pSignalSemaphores = nullptr};

  ASR_VK_CHECK(vkResetFences(dev, 1, &ctx.upload_fence));

  ASR_VK_CHECK(vkQueueSubmit(cqueue.info.queue, 1, &submit_info, upload_fence));

  ASR_VK_CHECK(
      vkWaitForFences(dev, 1, &ctx.upload_fence, VK_TRUE, COMMAND_TIMEOUT));

  ASR_VK_CHECK(vkResetCommandBuffer(ctx.upload_cmd_buffer, 0));

  staging_buffer.destroy(dev);

  return FontCache{
      .entries = std::move(cache_entries),
      .extent = cache_extent,
      .font_height = font_height,
      .atlas = vk::create_image_sampler(
          stx::rc::make_inplace<vk::ImageResource>(
              stx::os_allocator, image, view, memory, ctx.queue.value().share())
              .unwrap())};
}

// special characters: space, tab
// ' ' '\t'
void draw_text(std::string_view text, Font& font, FontCache& cache,
               vk::RecordingContext& ctx, TextStyle const& style,
               u32 font_height, f32 max_width = stx::f32_max,
               hb_script_t script = HB_SCRIPT_LATIN,
               hb_language_t language = hb_language_from_string("en", 2)) {
  ASR_CHECK(style.direction == HB_DIRECTION_LTR ||
            style.direction == HB_DIRECTION_RTL);

  hb_font_set_scale(font.hbfont, 64 * cache.font_height,
                    64 * cache.font_height);
  hb_buffer_reset(font.hbscratch_buffer);
  hb_buffer_set_script(font.hbscratch_buffer, script);
  hb_buffer_set_direction(font.hbscratch_buffer, style.direction);
  hb_buffer_set_language(font.hbscratch_buffer, language);
  hb_buffer_add_utf8(font.hbscratch_buffer, text.data(), text.size(), 0,
                     text.size());

  hb_feature_t features[] = {{Font::KERNING_FEATURE, true, 0,
                              std::numeric_limits<unsigned int>::max()},
                             {Font::LIGATURE_FEATURE, true, 0,
                              std::numeric_limits<unsigned int>::max()},
                             {Font::CONTEXTUAL_LIGATURE_FEATURE, true, 0,
                              std::numeric_limits<unsigned int>::max()}};

  hb_shape(font.hbfont, font.hbscratch_buffer, features, std::size(features));

  unsigned int nglyphs;
  hb_glyph_info_t* glyph_info =
      hb_buffer_get_glyph_infos(font.hbscratch_buffer, &nglyphs);
  hb_glyph_position_t* glyph_pos =
      hb_buffer_get_glyph_positions(font.hbscratch_buffer, &nglyphs);

  for (usize i = 0; i < nglyphs; ++i) {
    u32 codepoint = glyph_info[i].codepoint;

    stx::Span glyph =
        cache.entries.span().which([codepoint](FontCacheEntry const& entry) {
          return entry.codepoint == codepoint;
        });

    if (!glyph.is_empty()) {
      FontCacheEntry const& glyph = glyph[0];
      glyph.extent;
      glyph.s0;
      glyph.s1;
      glyph.t0;
      glyph.t1;
      // TODO(lamarrr): bearingX, bearingY,
      // xAdvance
    } else {
      // draw rect
    }
  }
}

}  // namespace asr
