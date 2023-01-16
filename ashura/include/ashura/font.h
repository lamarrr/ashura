#pragma once
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <string_view>
#include <utility>

#include "ashura/image.h"
#include "ashura/primitives.h"
#include "ashura/rect_pack.h"
#include "ashura/vulkan.h"
#include "freetype/freetype.h"
#include "harfbuzz/hb.h"
#include "stx/allocator.h"
#include "stx/limits.h"
#include "stx/span.h"
#include "stx/string.h"
#include "stx/text.h"
#include "stx/vec.h"
#include "unicode/ubidi.h"
#include "unicode/unistr.h"
#include "unicode/uscript.h"

namespace asr {
namespace gfx {

enum class FontLoadError : u8 { InvalidPath };

enum class TextAlign : u8 { Left, Center, Right };

enum class TextDirection : u8 { LeftToRight, RightToLeft };

// TODO(lamarrr): implement
enum class TextOverflow : u8 { None, Ellipsis };

struct TextStyle {
  f32 font_height = 16;
  /// multiplied by font_height
  f32 line_height = 1.2f;
  f32 letter_spacing = 2;
  f32 word_spacing = 4;
  u32 tab_size = 8;
  bool use_kerning = true;
  ///  use standard and contextual ligature substitution
  bool use_ligatures = true;
  bool underline = false;
  bool strikethrough = false;
  color foreground_color = colors::BLACK;
  color background_color = colors::TRANSPARENT;
};

struct Font {
  /// kerning operations
  static constexpr hb_tag_t KERNING_FEATURE = HB_TAG('k', 'e', 'r', 'n');
  /// standard ligature substitution
  static constexpr hb_tag_t LIGATURE_FEATURE = HB_TAG('l', 'i', 'g', 'a');
  /// contextual ligature substitution
  static constexpr hb_tag_t CONTEXTUAL_LIGATURE_FEATURE =
      HB_TAG('c', 'l', 'i', 'g');

  hb_face_t* hbface = nullptr;
  hb_font_t* hbfont = nullptr;
  hb_buffer_t* hbscratch_buffer = nullptr;
  FT_Library ftlib = nullptr;
  FT_Face ftface = nullptr;
  stx::Memory font_data;

  Font(hb_face_t* ahbface, hb_font_t* ahbfont, hb_buffer_t* ahbscratch_buffer,
       FT_Library aftlib, FT_Face aftface, stx::Memory afont_data)
      : hbface{ahbface},
        hbfont{ahbfont},
        hbscratch_buffer{ahbscratch_buffer},
        ftlib{aftlib},
        ftface{aftface},
        font_data{std::move(afont_data)} {}

  STX_MAKE_PINNED(Font)

  ~Font() {
    hb_face_destroy(hbface);
    hb_font_destroy(hbfont);
    hb_buffer_destroy(hbscratch_buffer);
    ASR_CHECK(FT_Done_Face(ftface) == 0);
    ASR_CHECK(FT_Done_FreeType(ftlib) == 0);
  }
};

inline stx::Rc<Font*> load_font_from_memory(stx::Memory memory, usize size) {
  hb_blob_t* hbblob = hb_blob_create(static_cast<char*>(memory.handle),
                                     static_cast<unsigned int>(size),
                                     HB_MEMORY_MODE_READONLY, nullptr, nullptr);
  ASR_CHECK(hbblob != nullptr);

  hb_face_t* hbface = hb_face_create(hbblob, 0);
  ASR_CHECK(hbface != nullptr);

  hb_font_t* hbfont = hb_font_create(hbface);
  ASR_CHECK(hbfont != nullptr);

  hb_buffer_t* hbscratch_buffer = hb_buffer_create();
  ASR_CHECK(hbscratch_buffer != nullptr);

  FT_Library ftlib;
  ASR_CHECK(FT_Init_FreeType(&ftlib) == 0);

  FT_Face ftface;
  ASR_CHECK(FT_New_Memory_Face(ftlib,
                               static_cast<FT_Byte const*>(memory.handle),
                               static_cast<FT_Long>(size), 0, &ftface) == 0);

  return stx::rc::make_inplace<Font>(stx::os_allocator, hbface, hbfont,
                                     hbscratch_buffer, ftlib, ftface,
                                     std::move(memory))
      .unwrap();
}

inline stx::Result<stx::Rc<Font*>, FontLoadError> load_font_from_file(
    stx::String const& path) {
  if (!std::filesystem::exists(path.c_str())) {
    return stx::Err(FontLoadError::InvalidPath);
  }

  std::ifstream stream{path.c_str(), std::ios::ate | std::ios_base::binary};

  usize size = stream.tellg();
  stream.seekg(0);

  stx::Memory memory = stx::mem::allocate(stx::os_allocator, size).unwrap();

  stream.read(static_cast<char*>(memory.handle), size);

  stream.close();

  return stx::Ok(load_font_from_memory(std::move(memory), size));
}

struct Glyph {
  bool is_valid = false;
  /// the glyph index
  u32 index = 0;
  /// unicode codepoint this glyph represents
  u32 codepoint = 0;
  /// offset into the atlas its glyph resides
  offset offset;
  /// extent of the glyph in the atlas
  extent extent;
  /// defines x-offset from cursor position the glyph will be placed
  f32 x = 0;
  /// defines ascent from baseline of the text
  f32 ascent = 0;
  /// advancement of the cursor after drawing this glyph
  vec2 advance;
  /// texture coordinates of this glyph in the atlas
  f32 s0 = 0, t0 = 0, s1 = 0, t1 = 0;
};

/// stores codepoint glyphs for a font at a specific font height
struct FontAtlas {
  stx::Vec<Glyph> glyphs{stx::os_allocator};
  /// overall extent of the atlas
  extent extent;
  /// font height at which the cache/atlas/glyphs will be rendered and cached
  u32 font_height = 26;
  /// atlas containing the packed glyphs
  image atlas = 0;

  stx::Span<Glyph const> get(u32 glyph_index) const {
    if (glyph_index >= glyphs.size()) return {};
    stx::Span glyph = glyphs.span().slice(glyph_index, 1);
    if (glyph.is_empty()) return glyph;
    return glyph[0].is_valid ? glyph : glyph.slice(0, 0);
  }
};

// stx::Fn
// TODO(lamarrr): try to support colored fonts
// TODO(lamarrr): this must always return an rgba colored atlas
inline FontAtlas render_atlas(Font const& font, vk::RecordingContext& ctx,
                              u32 font_height) {
  /// *64 to convert font height to 26.6 pixel format
  ASR_CHECK(font_height > 0);
  ASR_CHECK(FT_Set_Char_Size(font.ftface, 0, font_height * 64, 72, 72) == 0);

  vk::CommandQueue& cqueue = *ctx.queue.value().handle;
  VkDevice dev = cqueue.device.handle->device;
  VkPhysicalDeviceMemoryProperties const& memory_properties =
      cqueue.device.handle->phy_device.handle->memory_properties;

  stx::Vec<Glyph> glyphs{stx::os_allocator};

  {
    FT_UInt glyph_index;

    FT_ULong codepoint = FT_Get_First_Char(font.ftface, &glyph_index);

    while (glyph_index != 0) {
      if (FT_Load_Glyph(font.ftface, glyph_index, 0) == 0) {
        u32 width = font.ftface->glyph->bitmap.width;
        u32 height = font.ftface->glyph->bitmap.rows;

        // convert from 26.6 pixel format
        vec2 advance{font.ftface->glyph->advance.x / 64.0f,
                     font.ftface->glyph->advance.y / 64.0f};

        glyphs
            .push(Glyph{.is_valid = true,
                        .index = glyph_index,
                        .codepoint = codepoint,
                        .offset = {},
                        .extent = {width, height},
                        .x = AS_F32(font.ftface->glyph->bitmap_left),
                        .ascent = AS_F32(font.ftface->glyph->bitmap_top),
                        .advance = advance,
                        .s0 = 0,
                        .t0 = 0,
                        .s1 = 0,
                        .t1 = 0})
            .unwrap();
      }
      codepoint = FT_Get_Next_Char(font.ftface, codepoint, &glyph_index);
    }
  }

  Glyph* last = std::unique(
      glyphs.begin(), glyphs.end(),
      [](Glyph const& a, Glyph const& b) { return a.index == b.index; });

  glyphs.resize(last - glyphs.begin()).unwrap();

  VkImageFormatProperties image_format_properties;

  ASR_VK_CHECK(vkGetPhysicalDeviceImageFormatProperties(
      ctx.queue.value().handle->device.handle->phy_device.handle->phy_device,
      VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_SAMPLED_BIT, 0, &image_format_properties));

  stx::Memory rects_mem =
      stx::mem::allocate(stx::os_allocator, sizeof(stbrp_rect) * glyphs.size())
          .unwrap();

  stx::Span rects{static_cast<stbrp_rect*>(rects_mem.handle), glyphs.size()};

  for (usize i = 0; i < glyphs.size(); i++) {
    rects[i].glyph_index = glyphs[i].index;
    rects[i].w = AS_I32(glyphs[i].extent.width + 2);
    rects[i].h = AS_I32(glyphs[i].extent.height + 2);
  }

  stx::Memory nodes_memory =
      stx::mem::allocate(
          stx::os_allocator,
          sizeof(stbrp_node) * image_format_properties.maxExtent.width)
          .unwrap();

  stbrp_context context;
  stbrp_init_target(&context, image_format_properties.maxExtent.width,
                    image_format_properties.maxExtent.height,
                    static_cast<stbrp_node*>(nodes_memory.handle),
                    image_format_properties.maxExtent.width);
  stbrp_setup_allow_out_of_mem(&context, 0);
  stbrp_setup_heuristic(&context, STBRP_HEURISTIC_Skyline_default);
  ASR_CHECK(stbrp_pack_rects(&context, rects.data(), AS_I32(rects.size())));

  extent atlas_extent;

  for (usize i = 0; i < rects.size(); i++) {
    atlas_extent.width =
        std::max<u32>(atlas_extent.width, rects[i].x + rects[i].w);
    atlas_extent.height =
        std::max<u32>(atlas_extent.height, rects[i].y + rects[i].h);
  }

  rects.sort([](stbrp_rect const& a, stbrp_rect const& b) {
    return a.glyph_index < b.glyph_index;
  });

  glyphs.span().sort(
      [](Glyph const& a, Glyph const& b) { return a.index < b.index; });

  for (usize i = 0; i < glyphs.size(); i++) {
    u32 x = AS_U32(rects[i].x) + 1;
    u32 y = AS_U32(rects[i].y) + 1;
    u32 w = glyphs[i].extent.width;
    u32 h = glyphs[i].extent.height;

    glyphs[i].offset = {x, y};
    glyphs[i].s0 = AS_F32(x) / atlas_extent.width;
    glyphs[i].s1 = AS_F32(x + w) / atlas_extent.width;
    glyphs[i].t0 = AS_F32(y) / atlas_extent.height;
    glyphs[i].t1 = AS_F32(y + h) / atlas_extent.height;
  }

  {
    usize iter = 0;
    for (u32 next_index = 0; iter < glyphs.size(); next_index++, iter++) {
      for (; next_index < glyphs[iter].index; next_index++) {
        glyphs
            .push(Glyph{.is_valid = false,
                        .index = next_index,
                        .codepoint = 0,
                        .offset = {},
                        .extent = {},
                        .x = 0,
                        .ascent = 0,
                        .s0 = 0,
                        .t0 = 0,
                        .s1 = 0,
                        .t1 = 0})
            .unwrap();
      }
    }
  }

  glyphs.span().sort(
      [](Glyph const& a, Glyph const& b) { return a.index < b.index; });

  // NOTE: vulkan doesn't allow zero-extent images
  atlas_extent.width = std::max<u32>(1, atlas_extent.width);
  atlas_extent.height = std::max<u32>(1, atlas_extent.height);

  vk::Buffer atlas_staging_buffer =
      vk::create_host_buffer(dev, memory_properties, atlas_extent.area() * 4,
                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

  u8* buffer = static_cast<u8*>(atlas_staging_buffer.memory_map);

  std::memset(buffer, 0, atlas_staging_buffer.size);

  for (Glyph const& glyph : glyphs) {
    ASR_CHECK(FT_Load_Glyph(font.ftface, glyph.index, 0) == 0);
    ASR_CHECK(FT_Render_Glyph(font.ftface->glyph, FT_RENDER_MODE_NORMAL) == 0);

    u8* bitmap = font.ftface->glyph->bitmap.buffer;

    // copy the rendered glyph to the atlas
    for (usize j = glyph.offset.y; j < glyph.offset.y + glyph.extent.height;
         j++) {
      for (usize i = glyph.offset.x * 4;
           i < (glyph.offset.x + glyph.extent.width) * 4; i += 4) {
        buffer[j * atlas_extent.width * 4 + i + 0] = 0xFF;
        buffer[j * atlas_extent.width * 4 + i + 1] = 0xFF;
        buffer[j * atlas_extent.width * 4 + i + 2] = 0xFF;
        buffer[j * atlas_extent.width * 4 + i + 3] = *bitmap;
        bitmap++;
      }
    }
  }

  VkImageCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .imageType = VK_IMAGE_TYPE_2D,
      .format = VK_FORMAT_R8G8B8A8_SRGB,
      .extent = VkExtent3D{.width = atlas_extent.width,
                           .height = atlas_extent.height,
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
      vk::find_suitable_memory_type(memory_properties, memory_requirements,
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
      .format = VK_FORMAT_R8G8B8A8_SRGB,
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
      .imageExtent = VkExtent3D{.width = atlas_extent.width,
                                .height = atlas_extent.height,
                                .depth = 1}};

  vkCmdCopyBufferToImage(ctx.upload_cmd_buffer, atlas_staging_buffer.buffer,
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

  ASR_VK_CHECK(
      vkQueueSubmit(cqueue.info.queue, 1, &submit_info, ctx.upload_fence));

  ASR_VK_CHECK(
      vkWaitForFences(dev, 1, &ctx.upload_fence, VK_TRUE, COMMAND_TIMEOUT));

  ASR_VK_CHECK(vkResetCommandBuffer(ctx.upload_cmd_buffer, 0));

  atlas_staging_buffer.destroy(dev);

  return FontAtlas{
      .glyphs = std::move(glyphs),
      .extent = atlas_extent,
      .font_height = font_height,
      .atlas = vk::create_image_sampler(
          stx::rc::make_inplace<vk::ImageResource>(
              stx::os_allocator, image, view, memory, ctx.queue.value().share())
              .unwrap())};
}

struct CachedFont {
  stx::Rc<Font*> font;
  FontAtlas atlas;
};

inline CachedFont cache_font(stx::Rc<Font*> font,
                             asr::vk::RecordingContext& ctx, u32 font_height) {
  FontAtlas atlas = render_atlas(*font.handle, ctx, font_height);
  return CachedFont{.font = std::move(font), .atlas = std::move(atlas)};
}

/// A text run is a sequence of characters sharing a single property set. Any
/// change to the format, such as font style, foreground color, font family, or
/// any other formatting effect, breaks the text run.
struct TextRun {
  // text size in bytes
  usize text_size = 0;
  usize font = 0;
  TextStyle style;
  TextDirection direction = TextDirection::LeftToRight;
  hb_script_t script = HB_SCRIPT_LATIN;
  hb_language_t language = hb_language_from_string("en", 2);
};

struct Paragraph {
  /// utf-8 text
  stx::Span<char const> text;
  stx::Span<TextRun const> runs;
  TextAlign align = TextAlign::Left;
  TextOverflow overflow = TextOverflow::None;
};

struct TextLineGlyph {
  // glyph index
  u32 glyph = 0;
  // run of the glyph
  usize run = 0;
};

struct TextLineWord {
  usize glyph_count = 0;
  // right spacing
  f32 spacing = 0;
};

struct TextLine {
  stx::Vec<TextLineGlyph> glyphs{stx::os_allocator};
  stx::Vec<TextLineWord> words{stx::os_allocator};
};

/*
inline vec2 layout_text(Font& font, FontAtlas& cache, std::string_view text,
                        vec2 position, Paragraph const& paragraph,
                        f32 max_width = stx::f32_max) {
  constexpr u32 SPACE = ' ';
  constexpr u32 TAB = '\t';
  constexpr u32 NEWLINE = '\n';
  constexpr u32 RETURN = '\r';

  hb_font_set_scale(font.hbfont, 64 * cache.font_height,
                    64 * cache.font_height);

  vec2 extent;
  vec2 cursor;

  hb_feature_t const features[] = {
      {Font::KERNING_FEATURE, style.use_kerning, 0,
       std::numeric_limits<unsigned int>::max()},
      {Font::LIGATURE_FEATURE, style.use_ligatures, 0,
       std::numeric_limits<unsigned int>::max()},
      {Font::CONTEXTUAL_LIGATURE_FEATURE, style.use_ligatures, 0,
       std::numeric_limits<unsigned int>::max()}};

  ASR_CHECK(style.direction == HB_DIRECTION_LTR ||
            style.direction == HB_DIRECTION_RTL);

  f32 font_scale = AS_F32(style.font_height) / cache.font_height;
  f32 font_height = style.font_height;
  f32 line_height = style.line_height * font_height;
  f32 vertical_line_space = line_height - font_height;
  f32 vertical_line_padding = vertical_line_space / 2;
  f32 letter_spacing = style.letter_spacing;
  f32 word_spacing = style.word_spacing;

  for (char const* iter = text.data(); iter < text.data() + text.size();) {
    // TODO(lamarrr): handle multiple spaces or tabs
    char const* word_start = iter;

    u32 last_codepoint = 0;
    for (; iter < text.data() + text.size();) {
      last_codepoint = stx::utf8_next(iter);
      if (last_codepoint == SPACE || last_codepoint == TAB) {
        break;
      }
    }

    usize word_size = iter - word_start;

    hb_buffer_reset(font.hbscratch_buffer);
    hb_buffer_set_script(font.hbscratch_buffer, script);
    hb_buffer_set_direction(font.hbscratch_buffer, style.direction);
    hb_buffer_set_language(font.hbscratch_buffer, language);
    hb_buffer_add_utf8(font.hbscratch_buffer, word_start,
                       static_cast<int>(word_size), 0,
                       static_cast<int>(word_size));
    hb_shape(font.hbfont, font.hbscratch_buffer, features,
             static_cast<unsigned int>(std::size(features)));

    unsigned int nglyphs;
    hb_glyph_info_t* glyph_info =
        hb_buffer_get_glyph_infos(font.hbscratch_buffer, &nglyphs);

    f32 word_width = 0;

    for (usize i = 0; i < nglyphs; i++) {
      u32 glyph_index = glyph_info[i].codepoint;
      stx::Span glyph = cache.get(glyph_index);

      if (!glyph.is_empty()) {
        word_width += glyph[0].advance.x * font_scale + letter_spacing;
      } else {
        word_width += font_height + letter_spacing;
      }
    }

    if (cursor.x + word_width > max_width) {
      cursor.x = 0;
      cursor.y += line_height;
    }

    for (usize i = 0; i < nglyphs; i++) {
      u32 glyph_index = glyph_info[i].codepoint;
      stx::Span glyph = cache.get(glyph_index);
      glyph = glyph.is_empty() ? cache.glyphs.span().slice(0, 1) : glyph;

      Glyph const& g = glyph[0];
      cursor.x += font_scale * g.advance.x;
    }

    if (last_codepoint == SPACE) {
      cursor.x += word_spacing;
    } else if (last_codepoint == TAB) {
      cursor.x += word_spacing * style.tab_size;
    }

    extent.x = std::max(extent.x, cursor.x);
    extent.y = std::max(extent.y, cursor.y);
  }
  // TODO(lamarrr): support RTL
  // ASR_PANIC("RTL not supported yet");
  // }

  return extent;
}
*/

}  // namespace gfx
}  // namespace asr
