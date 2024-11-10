/// SPDX-License-Identifier: MIT
#include "ashura/engine/animation.h"

namespace ash
{

template struct Animation<f32>;

template <Animatable T>
void Animation<T>::play()
{
  state.is_playing = true;
  last_update_time = Clock::now();
}
template <Animatable T>
void Animation<T>::pause()
{
  state.is_playing = false;
}
template <Animatable T>
void Animation<T>::resume()
{
  if (!state.is_playing)
  {
    last_update_time = Clock::now();
    state.is_playing = true;
  }
}
template <Animatable T>
void Animation<T>::reset()
{
  is_reset = true;
}
template <Animatable T>
bool Animation<T>::is_playing() const
{
  return state.is_playing;
}

template <Animatable T>
void Animation<T>::run(f32 delta)
{
  if (state.delay > 0)
  {
    state.delay -= delta;
    return;
  }

  state.current_time += delta;

  if (state.current_time >= config.duration)
  {
    if (config.loop)
    {
      state.current_time = std::fmod(state.current_time, config.duration);
    }
    else
    {
      state.current_time = config.duration;
      state.is_playing   = false;
    }
  }

  f32 t      = state.current_time / config.duration;
  f32 easedT = 0.0f;

  switch (config.easing)
  {
    case CurveType::EaseIn:
      easedT = ease_in<f32>(t);
      break;
    case CurveType::EaseOut:
      easedT = ease_out<f32>(t);
      break;
    case CurveType::EaseInOut:
      easedT = ease_in_out<f32>(t);
      break;
    default:
      easedT = linear<f32>(t);
      break;
  }

  current_value = animation::lerp(start, end, easedT);

  if (is_reset)
  {
    state.current_time = 0;
    state.delay        = config.delay;
    current_value      = start;
  }
}
template <Animatable T>
void Animation<T>::update(f32 _time, f32 delta)
{
  if (!state.is_playing)
    return;

  last_update_time   = Clock::now();
  state.current_time = state.current_time - _time;

  run(delta);
}

void AnimationManager::reserve(u32 size)
{
  animations.reserve(size);
}

void AnimationManager::clear()
{
  animations.clear();
}

void AnimationManager::tick()
{
  for (auto &anim : animations)
  {
    if (auto ianim = std::static_pointer_cast<IAnimation>(anim))
    {
      ianim->tick();
    }
  }
}

void AnimationManager::play_all()
{
  for (auto &anim : animations)
  {
    if (auto ianim = std::static_pointer_cast<IAnimation>(anim))
    {
      ianim->play();
    }
  }
}

void AnimationManager::pause_all()
{
  for (auto &anim : animations)
  {
    if (auto ianim = std::static_pointer_cast<IAnimation>(anim))
    {
      ianim->pause();
    }
  }
}

void AnimationManager::reset_all()
{
  for (auto &anim : animations)
  {
    if (auto ianim = std::static_pointer_cast<IAnimation>(anim))
    {
      ianim->reset();
    }
  }
}

template <typename T>
void AnimationManager::stagger(const T &start, const T &end,
                               const AnimationConfig &base_config, size_t count,
                               const StaggerOptions &stagger_opts,
                               std::function<void(const T &, size_t)> callback)
{
  for (size_t i = 0; i < count; ++i)
  {
    f32 base_delay = stagger_delay(i, count, stagger_opts);

    auto config  = base_config;
    config.delay = stagger_opts.start + (base_delay * stagger_opts.from);

    // Create animation with grid index-aware callback
    auto anim = create<T>(start, end, config, [callback, i](const T &value) {
      if (callback)
        callback(value, i);
    });

    anim->play();
  }
}

// Timeline

template <typename T>
Timeline<T> &Timeline<T>::reserve(u32 size)
{
  segments.reserve(size);
}

template <typename T>
void Timeline<T>::clear()
{
  segments.clear();
}

template <typename T>
Timeline<T> &Timeline<T>::add(const T &start, const T &end,
                              AnimationConfig config)
{
  auto segment       = std::make_shared<BasicSegment<T>>(start, end, config);
  segment->label     = config.label;
  segment->startTime = total_duration;
  total_duration += segment->duration;
  segments.push_back(segment);
  return *this;
}

template <typename T>
Timeline<T> &Timeline<T>::add(const std::vector<Keyframe<T>> &keyframes,
                              std::optional<std::string>      label)
{
  auto segment = std::make_shared<KeyframeSegment<T>>(keyframes);
  if (label.has_value())
  {
    segment->label = label.value();
  }
  segment->startTime = total_duration;
  total_duration += segment->duration;
  segments.push_back(segment);
  return *this;
}

template <typename T>
Timeline<T> &Timeline<T>::parallel(const std::vector<AnimationConfig> &configs,
                                   const std::vector<std::pair<T, T>> &values)
{
  f32 max_duration = 0.0f;
  f32 start_time   = total_duration;

  for (size_t i = 0; i < configs.size(); ++i)
  {
    auto segment = std::make_shared<BasicSegment<T>>(
        values[i].first, values[i].second, configs[i]);
    segment->label = configs[i].label;
    segment->start = start_time;
    segment->type  = SegmentType::Parallel;
    segments.push_back(segment);
    max_duration = std::max(max_duration, configs[i].duration);
  }

  total_duration += max_duration;
  return *this;
}

template <typename T>
Timeline<T> &Timeline<T>::stagger(const T &start, const T &end,
                                  AnimationConfig config, size_t count,
                                  const StaggerOptions &stagger_opts)
{
  f32 base_delay = 0.0f;
  f32 start_time = total_duration;

  for (size_t i = 0; i < count; ++i)
  {
    base_delay = stagger_delay(i, count, stagger_opts);
  }

  f32 delay = stagger_opts.start + (base_delay * stagger_opts.from);

  auto segment_config  = config;
  segment_config.delay = delay;

  auto segment = std::make_shared<BasicSegment<T>>(start, end, segment_config);
  segment->start_time = start_time;
  segment->type       = SegmentType::Staggered;
  segment->label      = config.label;
  segments.push_back(segment);

  return *this;
}

template <typename T>
void Timeline<T>::update()
{
  if (!is_playing)
    return;

  auto current_time_point = Clock::now();
  f32  delta = get_delta(current_time_point, last_update_time);
  last_update_time = current_time_point;

  f32 adjusted_delta = delta * time_direction;
  current_time += adjusted_delta;

  if (time_direction > 0 && current_time >= total_duration)
  {
    if (options.loop)
    {
      current_time = std::fmod(current_time, total_duration);
      for (auto &segment : segments)
      {
        segment->reset();
      }
    }
    else
    {
      current_time = total_duration;
      is_playing   = false;
    }
  }
  else if (time_direction < 0 && current_time <= 0)
  {
    if (options.loop)
    {
      current_time = total_duration + std::fmod(current_time, total_duration);
      for (auto &segment : segments)
      {
        segment->reset();
      }
    }
    else
    {
      current_time = 0;
      is_playing   = false;
    }
  }

  update_segments(adjusted_delta);
}

template <typename T>
void Timeline<T>::play()
{
  is_playing       = true;
  last_update_time = Clock::now();
}

template <typename T>
void Timeline<T>::play_from_start()
{
  for (auto &segment : segments)
  {
    segment->reset();
  }
  current_time     = 0;
  time_direction   = 1.0f;
  is_playing       = true;
  last_update_time = Clock::now();
}

template <typename T>
void Timeline<T>::play_from_end()
{
  for (auto &segment : segments)
  {
    segment->reset();
  }
  current_time     = total_duration;
  time_direction   = -1.0f;
  is_playing       = true;
  last_update_time = Clock::now();
}

template <typename T>
void Timeline<T>::pause()
{
  is_playing = false;
}

template <typename T>
void Timeline<T>::resume()
{
  if (!is_playing)
  {
    play();
  }
}

template <typename T>
void Timeline<T>::stop()
{
  is_playing     = false;
  current_time   = 0;
  time_direction = 1.0f;
  for (auto &segment : segments)
  {
    segment->reset();
  }
}

template <typename T>
void Timeline<T>::seek(f32 time)
{
  f32 target_time = std::clamp(time, 0.0f, total_duration);
  f32 delta       = target_time - current_time;

  // Reset all segments if seeking backwards
  if (delta < 0)
  {
    for (auto &segment : segments)
    {
      segment->reset();
    }
    current_time = 0;
    update_segments(target_time);
  }
  else
  {
    update_segments(delta);
  }

  current_time = target_time;
}

template <typename T>
void Timeline<T>::seek_percentage(f32 percentage)
{
  percentage = std::clamp(percentage, 0.0f, 1.0f);
  seek(total_duration * percentage);
}

template <typename T>
void Timeline<T>::reverse()
{
  time_direction *= -1.0f;
}

template <typename T>
void Timeline<T>::set_direction(i32 direction)
{
  time_direction = direction < 0 ? -1.0f : 1.0f;
}

template <typename T>
f32 Timeline<T>::get_current_time() const
{
  return current_time;
}
template <typename T>
f32 Timeline<T>::get_duration() const
{
  return total_duration;
}
template <typename T>
f32 Timeline<T>::get_progress() const
{
  return current_time / total_duration;
}
template <typename T>
bool Timeline<T>::is_reversed() const
{
  return time_direction < 0;
}
template <typename T>
bool Timeline<T>::is_finished() const
{
  return !options.loop &&
         ((time_direction > 0 && current_time >= total_duration) ||
          (time_direction < 0 && current_time <= 0));
}

template <typename T>
Timeline<T>::SegmentPtr Timeline<T>::get_segment(std::string label)
{
  auto filter_segment = [&label](SegmentPtr seg) {
    return seg->get_label().compare(label);
  };
  if (SegmentPtr result = std::ranges::find_if(segments, filter_segment);
      result != segments->end())
  {
    return result;
  }
}
template <typename T>
Timeline<T>::SegmentPtr Timeline<T>::get_segment(u32 index)
{
  return segments[index];
}
template <typename T>
void Timeline<T>::update_segments(f32 delta)
{
  for (auto &segment : segments)
  {
    if (time_direction > 0)
    {
      // Forward playback
      if (current_time >= segment->start_time &&
          current_time <= segment->start_time + segment->duration)
      {
        segment->update(current_time, delta);
      }
    }
    else
    {
      // Reverse playback
      f32 segment_end = segment->startTime + segment->duration;
      if (current_time <= segment_end && current_time >= segment->start_time)
      {
        segment->update(current_time, delta);
      }
    }
  }
}

template <Animatable T>
void BasicSegment<T>::update(f32 current_time, f32 delta)
{
  animation.update(current_time, delta);
}
template <Animatable T>
void BasicSegment<T>::reset()
{
  animation.reset();
}
template <Animatable T>
T BasicSegment<T>::value() const
{
  return this->interpolated_value;
}

template <Animatable T>
void KeyframeSegment<T>::calculate_duration()
{
  this->duration = 0.0f;
  for (const auto &kf : keyframes)
  {
    this->duration += kf.duration;
  }
}

template <Animatable T>
void KeyframeSegment<T>::update(f32 _time, f32 delta)
{
  if (keyframes.size() < 2)
    return;

  current_time         = (_time - this->start) + delta;
  f32 accumulated_time = 0.0f;

  for (size_t i = 0; i < keyframes.size() - 1; ++i)
  {
    accumulated_time += keyframes[i].duration;

    if (current_time <= accumulated_time)
    {
      const Keyframe<T> &current_kf = keyframes[i];
      const auto        &next_kf    = keyframes[i + 1];

      f32 local_start_time = accumulated_time - current_kf.duration;
      f32 local_time = (current_time - local_start_time) / current_kf.duration;
      f32 eased_time = current_kf.get_ease_fn(local_time);

      this->interpolated_value =
          lerp(current_kf.value, next_kf.value, eased_time);
      break;
    }
  }
}

template <Animatable T>
void KeyframeSegment<T>::reset()
{
  current_time = 0.0f;
}

}        // namespace ash