#include "ashura/engine/text.h"
#include "ashura/std/vec.h"

extern "C"
{
#include "SBAlgorithm.h"
#include "SBLine.h"
#include "SBParagraph.h"
#include "SBScriptLocator.h"
#include "harfbuzz/hb.h"
}

namespace ash
{

/// layout is output in FONT_LAYOUT_UNIT units.
/// text must have been sanitized with invalid codepoints replaced before
/// calling this.
static void shape(hb_font_t *font, hb_buffer_t *buffer, Span<u32 const> text,
                  Segment segment, hb_script_t script, hb_direction_t direction,
                  hb_language_t language, bool use_kerning, bool use_ligatures,
                  Span<hb_glyph_info_t const>     &infos,
                  Span<hb_glyph_position_t const> &positions)
{
  // tags are opentype feature tags
  hb_feature_t const shaping_features[] = {
      // kerning operations
      {.tag   = HB_TAG('k', 'e', 'r', 'n'),
       .value = use_kerning,
       .start = HB_FEATURE_GLOBAL_START,
       .end   = HB_FEATURE_GLOBAL_END},
      // standard ligature glyph substitution
      {.tag   = HB_TAG('l', 'i', 'g', 'a'),
       .value = use_ligatures,
       .start = HB_FEATURE_GLOBAL_START,
       .end   = HB_FEATURE_GLOBAL_END},
      // contextual ligature glyph substitution
      {.tag   = HB_TAG('c', 'l', 'i', 'g'),
       .value = use_ligatures,
       .start = HB_FEATURE_GLOBAL_START,
       .end   = HB_FEATURE_GLOBAL_END}};

  hb_buffer_reset(buffer);
  // invalid character replacement
  hb_buffer_set_replacement_codepoint(buffer,
                                      HB_BUFFER_REPLACEMENT_CODEPOINT_DEFAULT);
  // OpenType (ISO15924) Script Tag.
  // See: https://unicode.org/reports/tr24/#Relation_To_ISO15924
  hb_buffer_set_script(buffer, script);
  hb_buffer_set_direction(buffer, direction);
  // OpenType BCP-47 language tag specifying locale-sensitive shaping operations
  // as defined in the font
  hb_buffer_set_language(buffer, language);
  hb_buffer_add_codepoints(buffer, text.data(), (i32) text.size(),
                           segment.first, (i32) segment.count);
  hb_shape_full(font, buffer, shaping_features, (u32) size(shaping_features),
                nullptr);

  u32                        num_pos;
  hb_glyph_position_t const *glyph_pos =
      hb_buffer_get_glyph_positions(buffer, &num_pos);
  CHECK(!(glyph_pos == nullptr && num_pos > 0));

  u32                    num_info;
  hb_glyph_info_t const *glyph_info =
      hb_buffer_get_glyph_infos(buffer, &num_info);
  CHECK(!(glyph_info == nullptr && num_info > 0));

  CHECK(num_pos == num_info);

  infos     = Span{glyph_info, num_info};
  positions = Span{glyph_pos, num_pos};
}

/// @brief only needs to be called if it contains multiple paragraphs
static Slice segment_paragraphs(Span<u32 const> text, Vec<Segment> &segments)
{
  u32 const text_size = (u32) text.size();
  Slice     slice;
  slice.offset = segments.size();
  for (u32 it = 0; it < text_size;)
  {
    u32 beg = it;
    u32 end = it;
    while (it < text_size)
    {
      if (text[it] == '\r' && ((it + 1) < text_size) && text[it + 1] == '\n')
      {
        end = it;
        it += 2;
        break;
      }
      if (text[it] == '\n')
      {
        end = it++;
        break;
      }
      if (text[it] == '\r')
      {
        end = it++;
        break;
      }

      end = ++it;
    }
    CHECK(segments.push(Segment{.first = beg, .count = end - beg}));
  }
  slice.span = segments.size() - slice.offset;
  return slice;
}

/// @brief only needs to be called if it contains multiple scripts
/// outputs iso15924 or OpenType tags
static Slice segment_scripts(Span<u32 const> text, Vec<Segment> &segments,
                             Vec<u32> &scripts)
{
  Slice slice;
  slice.offset = segments.size();

  SBCodepointSequence codepoints{.stringEncoding = SBStringEncodingUTF32,
                                 .stringBuffer   = (void *) text.data(),
                                 .stringLength   = text.size()};

  SBScriptLocatorRef locator = SBScriptLocatorCreate();
  CHECK(locator != nullptr);
  SBScriptLocatorLoadCodepoints(locator, &codepoints);

  SBScriptAgent const *agent = SBScriptLocatorGetAgent(locator);
  CHECK(agent != nullptr);

  while (SBScriptLocatorMoveNext(locator) == SBTrue)
  {
    CHECK(segments.push(
        Segment{.first = (u32) agent->offset, .count = (u32) agent->length}));
    hb_script_t script =
        hb_script_from_iso15924_tag(SBScriptGetOpenTypeTag(agent->script));
    CHECK(scripts.push((u32) script));
  }

  SBScriptLocatorRelease(locator);
  slice.span = segments.size() - slice.offset;
  return slice;
}

/// @brief only needs to be called if it is a bidirectional text
static Slice segment_directions(Span<u32 const> text, Slice paragraphs,
                                SBAlgorithmRef algorithm, TextDirection base,
                                Vec<Segment>       &segments,
                                Vec<TextDirection> &base_directions,
                                Vec<TextDirection> &directions)
{
  Slice slice;
  slice.offset = segments.size();

  // otherwise it will segfault due to the segments vec being inserted
  // into while it is being written into
  CHECK(segments.reserve(segments.size() + text.size()));

  // The embedding level is an integer value. LTR text segments have even
  // embedding levels (e.g., 0, 2, 4), and RTL text segments have odd embedding
  // levels (e.g., 1, 3, 5).
  for (Segment p : to_span(segments)[paragraphs])
  {
    SBParagraphRef paragraph =
        SBAlgorithmCreateParagraph(algorithm, p.first, p.count, (SBLevel) base);
    CHECK(paragraph != nullptr);
    SBUInteger length = SBParagraphGetLength(paragraph);
    CHECK(length == p.count);
    SBLevel base_level = SBParagraphGetBaseLevel(paragraph);
    CHECK(base_directions.push(TextDirection{base_level}));
    SBLevel const *levels = SBParagraphGetLevelsPtr(paragraph);
    CHECK(!(p.count > 0 && levels == nullptr));

    for (u32 it = 0; it < p.count;)
    {
      SBLevel curr = levels[it] & 0x1;
      u32     beg  = it++;
      while (it < p.count)
      {
        if ((levels[it] & 0x1) != curr)
        {
          break;
        }
        it++;
      }
      CHECK(directions.push(TextDirection{curr}));
      CHECK(segments.push(Segment{.first = it, .count = it - beg}));
    }
    SBParagraphRelease(paragraph);
  }
  slice.span = segments.size() - slice.offset;
  return slice;
}

/// @brief only needs to be called if line breaking is required.
static Slice segment_break_opportunities(Span<u32 const> text,
                                         Vec<Segment> &segments, f32 max_width)
{
  Slice slice;
  slice.offset = segments.size();

  if (max_width == F32_MAX)
  {
    CHECK(segments.push(Segment{.first = 0, .count = (u32) text.size()}));
    slice.span = 1;
    return slice;
  }

  u32 const text_size = (u32) text.size();
  for (u32 it = 0; it < text_size;)
  {
    u32 begin = it++;
    while (it < text_size && text[it] != ' ' && text[it] != '\t')
    {
      it++;
    }

    while (it < text_size && (text[it] == ' ' || text[it] == '\t'))
    {
      it++;
    }

    CHECK(segments.push(Segment{.first = begin, .count = it - begin}));
  }
  slice.span = segments.size() - slice.offset;
  return slice;
}

static bool is_segments_valid(Span<Segment const> segments, u32 size)
{
  u32 first = 0;
  for (Segment s : segments)
  {
    if (s.first != first || ((s.first + s.count) > size))
    {
      return false;
    }
    first = s.first + s.count;
  }
  return first == size;
}

//
// shape each run at each break opportunity
//
//
// is there a way we can perform the text shaping in batches without knowing the
// size of the text?
//
// we can layout segments independent of actual font size
//
//
//
void layout_text(TextBlock const &block, f32 max_width, TextLayout &layout)
{
  CHECK(block.text.size() < I32_MAX);
  CHECK(is_segments_valid(to_span(block.segments), (u32) block.text.size()));

  layout.max_width = max_width;
  layout.extent    = Vec2{0, 0};

  hb_language_t language =
      block.language.is_empty() ?
          hb_language_get_default() :
          hb_language_from_string(block.language.data(),
                                  (int) block.language.size());

  hb_buffer_t *shaping_buffer = hb_buffer_create();
  CHECK(shaping_buffer != nullptr);
  defer shaping_buffer_del{[&] { hb_buffer_destroy(shaping_buffer); }};

  SBCodepointSequence codepoints{.stringEncoding = SBStringEncodingUTF32,
                                 .stringBuffer   = (void *) block.text.data(),
                                 .stringLength   = (u32) block.text.size()};
  SBAlgorithmRef      algorithm = SBAlgorithmCreate(&codepoints);
  CHECK(algorithm != nullptr);
  defer algorithm_del{[&] { SBAlgorithmRelease(algorithm); }};

  // use custom iterator?
  Vec<Segment>       segments             = {};
  Vec<u32>           run_scripts          = {};
  Vec<TextDirection> run_directions       = {};
  Vec<TextDirection> paragraph_directions = {};
  // could be needed to be preserved? what about editor  selection and
  // insertion?

  // uninit data

  Slice paragraphs = segment_paragraphs(block.text, segments);
  Slice scripts    = segment_scripts(block.text, segments, run_scripts);
  Slice directions =
      segment_directions(block.text, paragraphs, algorithm, block.direction,
                         segments, paragraph_directions, run_directions);
  Slice break_opportunities =
      segment_break_opportunities(block.text, segments, max_width);

  u32 iparagraph         = 0;
  u32 iscript            = 0;
  u32 ibreak_opportunity = 0;
  u32 idirection         = 0;
  u32 istyle             = 0;
  u32 iline              = 0;

  for (u32 i = 0; i < paragraphs.span; i++)
  {
    while (true) /* in run*/
    {
      // advance, else break
    }
    // insert into run list
  }
  // should consider that newlines will not be included and some might be empty

  // for each run: shape, advance

  //
  // line breaks?

  //
  //
  //
  //
  // separate into Runs -> Run{ iscript, segment, direction   };
  // break into lines based on max width
  // what about visual reordering? of segments on lines?

  //

  // for each paragraph
  // use the codepoint index as anchor.
  // find max overlapping of each segment and perform layout on them, then
  // advance to next max overlapping

  // shape each segment independently, break the lines as necessary then
  // re-order based on base direction and then reverse using segment direction.
}

}        // namespace ash
