
// bufffer memory is aligned for optimal use
// in the skia API (8-bit alignment for single channel images i.e. Grey. and
// 32-bit alignment for multi-channel images i.e. RGB and RGBA).
// a new buffer establishes alignment requirement for the image if necessary
inline sk_sp<SkImage> dispatch_image_to_gpu(
    RenderContext const& context, ImageInfo const& info,
    stx::Span<uint8_t const> unaligned_pixels) {
  sk_sp texture = context.create_target_texture(info.extent);

  SkCanvas* canvas = texture->getCanvas();

  SkPaint paint;
  paint.setBlendMode(SkBlendMode::kSrc);

  switch (info.format) {
    case ImageFormat::Gray: {
      sk_sp data = SkData::MakeWithoutCopy(unaligned_pixels.data(),
                                           unaligned_pixels.size_bytes());
      sk_sp image =
          SkImage::MakeRasterData(to_skia(info), data, info.extent.width * 1);
      VLK_ENSURE(image != nullptr);
      canvas->drawImage(image, 0, 0, &paint);
    } break;
    case ImageFormat::RGB: {
      auto aligned_buffer = make_aligned_RGBx_buffer(
          unaligned_pixels.data(), info.extent.width, info.extent.height);
      sk_sp data = SkData::MakeWithoutCopy(
          aligned_buffer.get(), info.extent.width * info.extent.height *
                                    aligned_channel_size(info.format));
      sk_sp image =
          SkImage::MakeRasterData(to_skia(info), data, info.extent.width * 4);
      VLK_ENSURE(image != nullptr);
      canvas->drawImage(image, 0, 0, &paint);
    } break;
    case ImageFormat::RGBA: {
      auto aligned_buffer = make_aligned_RGBA_buffer(
          unaligned_pixels.data(), info.extent.width, info.extent.height);
      sk_sp data = SkData::MakeWithoutCopy(
          aligned_buffer.get(), info.extent.width * info.extent.height *
                                    aligned_channel_size(info.format));
      sk_sp image =
          SkImage::MakeRasterData(to_skia(info), data, info.extent.width * 4);
      VLK_ENSURE(image != nullptr);
      canvas->drawImage(image, 0, 0, &paint);
    } break;

    default:
      VLK_PANIC("Unsupported Image Format", info.format);
  }

  return texture->makeImageSnapshot();
}

// remove pixmap and replace with this function instead
inline sk_sp<SkImage> dispatch_image_to_gpu(
    RenderContext const& context, StbiImageBuffer const& unaligned_buffer) {
  return dispatch_image_to_gpu(context, unaligned_buffer.info(),
                               unaligned_buffer.span());
}
