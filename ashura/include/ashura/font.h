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

enum class TextDirection : u8 { LeftToRight, RightToLeft };

struct TextStyle {
  f32 font_height = 10;
  f32 letter_spacing = 0;
  f32 word_spacing = 0;
  TextDirection direction = TextDirection::LeftToRight;
  TextAlign align = TextAlign::Left;
  WordWrap word_wrap = WordWrap::None;
  u32 num_tab_spaces = 4;
};

namespace unicode_ranges {
constexpr range ENGLISH{0x020, 0x007F};  // a.k.a. LATIN ASCII
constexpr range EMOTICONS{0x1F600, 0x1F64F};
constexpr range CURRENCY_SYMBOLS{0x20A0, 0x20CF};
constexpr range ARROWS{0x2190, 0x21FF};
constexpr range MATHEMATICAL_OPERATORS{0x2200, 0x22FF};
constexpr range ARABIC{0x0600, 0x06FF};
constexpr range HIRAGANA{0x3040, 0x309F};
constexpr range KATAKANA{0x30A0, 0x30FF};
}  // namespace unicode_ranges

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

  Font(hb_face_t* ahbface, hb_font_t* ahbfont, FT_Library aftlib,
       FT_Face aftface, hb_buffer_t* ahbscratch_buffer)
      : hbface{ahbface},
        hbfont{ahbfont},
        ftlib{aftlib},
        ftface{aftface},
        hbscratch_buffer{ahbscratch_buffer} {}

  STX_MAKE_PINNED(Font)

  ~Font() {
    hb_face_destroy(hbface);
    hb_font_destroy(hbfont);
    hb_buffer_destroy(hbscratch_buffer);
    ASR_CHECK(FT_Done_Face(ftface) == 0);
    ASR_CHECK(FT_Done_FreeType(ftlib) == 0);
  }
};

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

enum class FontLoadError { InvalidPath };

// stores codepoint glyphs for a font at a specific font height
struct FontCache {
  struct Entry {
    u32 codepoint;
    offset offset;
    extent extent;
  };

  stx::Vec<Entry> entries{stx::os_allocator};
  extent total_extent;
  offset consume_cursor;
  u32 row_height = 0;
  u32 font_height = 20;
  stx::Option<stx::Rc<vk::ImageResource*>> atlas;

  FontCache(vk::RecordingContext& ctx, extent atlas_extent,
            u32 atlas_font_height)
      : total_extent{atlas_extent}, font_height{atlas_font_height} {
    usize atlas_size = atlas_extent.area() * 4;
    stx::Memory atlas_memory =
        stx::mem::allocate(stx::os_allocator, atlas_size).unwrap();
    stx::Span atlas_bytes{static_cast<u8*>(atlas_memory.handle), atlas_size};
    std::memset(atlas_bytes.data(), 0, atlas_bytes.size_bytes());

    atlas = stx::Some(ctx.upload_image(atlas_extent, atlas_bytes));
  }

  stx::Option<offset> add(vk::RecordingContext& ctx, u32 codepoint,
                          stx::Span<u8 const> bitmap, extent glyph_extent) {
    ASR_CHECK(glyph_extent.is_visible());
    ASR_CHECK(bitmap.size_bytes() >= total_extent.area());

    offset glyph_offset;

    // note that the cursor is not advanced if there's no available space

    // if we have space on the current row
    if (consume_cursor.x + glyph_extent.width <= total_extent.width) {
      if (consume_cursor.y + glyph_extent.height <= total_extent.height) {
        consume_cursor.x += glyph_extent.width;
        glyph_offset = consume_cursor;
        row_height = std::max(row_height, glyph_extent.height);
      } else {
        // no space
        return stx::None;
      }
    }
    // try to wrap around to a new row and find space
    else {
      // if we have space with wrap around on the new row
      if (glyph_extent.width <= total_extent.width &&
          consume_cursor.y + row_height + glyph_extent.height <=
              total_extent.height) {
        glyph_offset = offset{0, consume_cursor.y + row_height};
        consume_cursor.y += row_height;
        row_height = glyph_extent.height;
        consume_cursor.x = glyph_extent.width;
      } else {
        // no space
        return stx::None;
      }
    }

    vk::CommandQueue& cqueue = *ctx.queue.value().handle;
    VkDevice dev = cqueue.device.handle->device;
    VkPhysicalDeviceMemoryProperties const& memory_properties =
        cqueue.device.handle->phy_device.handle->memory_properties;

    vk::Buffer staging_buffer =
        vk::create_host_buffer(dev, memory_properties, total_extent.area() * 4,
                               VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    {
      u8* out = static_cast<u8*>(staging_buffer.memory_map);

      for (u8 alpha : bitmap) {
        out[0] = 0xFF;
        out[1] = 0xFF;
        out[2] = 0xFF;
        out[3] = alpha;
        out += 4;
      }
    }

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

    vkCmdPipelineBarrier(
        ctx.upload_cmd_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr,
        0, nullptr, 1, &pre_upload_barrier);

    VkBufferImageCopy copy{
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource =
            VkImageSubresourceLayers{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                     .mipLevel = 0,
                                     .baseArrayLayer = 0,
                                     .layerCount = 1},
        .imageOffset =
            VkOffset3D{.x = glyph_offset.x, .y = glyph_offset.y, .z = 0},
        .imageExtent = VkExtent3D{.width = glyph_extent.width,
                                  .height = glyph_extent.height,
                                  .depth = 1}};

    vkCmdCopyBufferToImage(ctx.upload_cmd_buffer, staging_buffer.buffer,
                           atlas.value().handle->image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);

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
        vkQueueSubmit(cqueue.info.queue, 1, &submit_info, upload_fence));

    ASR_VK_CHECK(
        vkWaitForFences(dev, 1, &ctx.upload_fence, VK_TRUE, COMMAND_TIMEOUT));

    ASR_VK_CHECK(vkResetCommandBuffer(ctx.upload_cmd_buffer, 0));

    staging_buffer.destroy(dev);

    entries
        .push(Entry{.codepoint = codepoint,
                    .offset = glyph_offset,
                    .extent = glyph_extent})
        .unwrap();

    return stx::Some(offset{glyph_offset});
  }
};

// special characters:

void render_text(std::string_view text, Font& font, FontCache& cache,
                 vk::RecordingContext& ctx, extent atlas_extent,
                 u32 font_height, hb_script_t script = HB_SCRIPT_LATIN,
                 hb_language_t language = hb_language_from_string("en", 2),
                 hb_direction_t direction = HB_DIRECTION_LTR) {
  if (font_height != cache.font_height) ASR_LOG_WARN();

  hb_font_set_scale(font.hbfont, 64 * font_height, 64 * font_height);

  hb_buffer_reset(font.hbscratch_buffer);
  hb_buffer_set_script(font.hbscratch_buffer, script);
  hb_buffer_set_language(font.hbscratch_buffer, language);
  hb_buffer_set_direction(font.hbscratch_buffer, text_direction);
  hb_buffer_add_utf8(font.hbscratch_buffer, text.data(), text.size(), 0,
                     text.size());

  hb_feature_t features[] = {
      {KERNING, true, 0, std::numeric_limits<unsigned int>::max()},
      {LIGATURE, true, 0, std::numeric_limits<unsigned int>::max()},
      {CONTEXTUAL_LIGATURE, true, 0, std::numeric_limits<unsigned int>::max()}};

  hb_shape(font.hbfont, font.hbscratch_buffer, features, std::size(features));

  unsigned int glyph_count;
  hb_glyph_info_t* glyph_info =
      hb_buffer_get_glyph_infos(font.hbscratch_buffer, &glyph_count);
  hb_glyph_position_t* glyph_pos =
      hb_buffer_get_glyph_positions(font.hbscratch_buffer, &glyph_count);

  ASR_CHECK(FT_Set_Char_Size(font.ftface, 0, font_height * 64, 72, 72) == 0);

  for (usize i = 0; i < glyph_count; ++i) {
    u32 codepoint = glyph_info[i].codepoint;
    FT_Load_Glyph(ftface, codepoint, FT_LOAD_RENDER);
    FT_Render_Glyph(ftface->glyph, FT_RENDER_MODE_NORMAL);
    FT_Bitmap bitmap = ftface->glyph->bitmap;
    auto buff = bitmap.buffer;
    auto h = bitmap.rows;
    auto w = bitmap.width;
  }
}

{
  unsigned int glyph_count;
  hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos(buffer, &glyph_count);
  hb_glyph_position_t* glyph_pos =
      hb_buffer_get_glyph_positions(buffer, &glyph_count);

  for (size_t i = 0; i < glyph_count; ++i) {
    u32 codepoint = glyph_info[i].codepoint;

    stx::Span cache =
        cache.entries.span().which([codepoint](FontCache::Entry const& entry) {
          return entry.codepoint == codepoint;
        });
    if (cache.is_empty()) {
      // add to cache

      FT_Load_Glyph(ftface, codepoint, FT_LOAD_DEFAULT);
      FT_Render_Glyph(ftface->glyph, FT_RENDER_MODE_NORMAL);
      FT_Bitmap bitmap = ftface->glyph->bitmap;
      unsigned char* buff = bitmap.buffer;
      size_t h = bitmap.rows;
      size_t w = bitmap.width;

      //

    } else {
    }
  }
}

struct Typeface {
  TypefaceAtlasConfig config;
  stx::Vec<stbtt_packedchar> glyphs{stx::os_allocator};
  Image atlas;

  vec2 layout();

  vec2 layout(stx::StringView text, TextStyle const& style,
              vec2 max_width = {stx::f32_max, stx::f32_max}) const {
    vec2 extent{0, 0};

    for (char character : text) {
      switch (character) {
        case ' ': {
          switch (style.direction) {
            case TextDirection::LeftToRight: {
              switch (style.word_wrap) {}
              vec2 new_extent = extent + vec2{AS_F32(word_spacing), 0} + vec2{};
              // new_extent.x >

            } break;

            default: {
              ASR_PANIC(unsupported text direction);
            } break;
          }
        } break;

        case '\t': {
          switch (style.direction) {
            case TextDirection::LeftToRight: {
            } break;
            default: {
              ASR_PANIC(unsupported text direction);
            } break;
          }
        }

        default: {
          if (!info.has_character(character)) {
            continue;
          }
        }
      }
    }
  }
};

}  // namespace asr