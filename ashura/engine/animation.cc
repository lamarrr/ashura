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
void Animation<T>::run(Duration delta)
{
  if (state.delay > Duration::zero())
  {
    state.delay -= delta;
    return;
  }

  state.current_time += delta;

  if (state.current_time >= config.duration)
  {
    if (config.loop)
    {
      if (config.duration.count() <= 0)
        state.current_time = Duration::zero();
      // Modulo
      auto division =
          std::lldiv(state.current_time.count(), config.duration.count());
      state.current_time = Duration{division.rem};
    }
    else
    {
      state.current_time = config.duration;
      state.is_playing   = false;
    }
  }

  // Convert time to float for easing calculations
  float t = std::chrono::duration<float>(state.current_time) /
            std::chrono::duration<float>(config.duration);

  float easedT = 0.0f;
  switch (config.easing)
  {
    case CurveType::EaseIn:
      easedT = ease_in<float>(t);
      break;
    case CurveType::EaseOut:
      easedT = ease_out<float>(t);
      break;
    case CurveType::EaseInOut:
      easedT = ease_in_out<float>(t);
      break;
    default:
      easedT = linear<float>(t);
      break;
  }

  current_value = lerp(start, end, easedT);

  if (is_reset)
  {
    state.current_time = Duration::zero();
    state.delay        = config.delay;
    current_value      = start;
    is_reset           = false;
  }
}

template <Animatable T>
void Animation<T>::update(Duration _time, Duration delta)
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
Timeline<T> &Timeline<T>::add(const std::vector<KeyFrame<T>> &keyframes,
                              std::optional<Span<const char>> label)
{
  auto segment = std::make_shared<KeyFrameSegment<T>>(keyframes);
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
  Duration max_duration = Duration::zero();
  Duration start_time   = total_duration;

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
  Duration base_delay = Duration::zero();
  Duration start_time = total_duration;

  for (size_t i = 0; i < count; ++i)
  {
    base_delay = stagger_delay(i, count, stagger_opts);
  }

  Duration delay = stagger_opts.start + base_delay;

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

  auto     current_time_point = Clock::now();
  Duration delta              = get_delta(current_time_point, last_update_time);
  last_update_time            = current_time_point;

  Duration adjusted_delta = delta * time_direction;
  current_time += adjusted_delta;

  if (time_direction > 0 && current_time >= total_duration)
  {
    if (options.loop)
    {
      current_time =
          Duration{std::fmod(current_time.count(), total_duration.count())};
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
  else if (time_direction < 0 && current_time <= Duration::zero())
  {
    if (options.loop)
    {
      current_time =
          total_duration +
          Duration{std::fmod(current_time.count(), total_duration.count())};
      for (auto &segment : segments)
      {
        segment->reset();
      }
    }
    else
    {
      current_time = Duration::zero();
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
  current_time     = Duration::zero();
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
  current_time   = Duration::zero();
  time_direction = 1.0f;
  for (auto &segment : segments)
  {
    segment->reset();
  }
}

template <typename T>
void Timeline<T>::seek(Duration time)
{
  Duration target_time = std::clamp(time, Duration::zero(), total_duration);

  if (target_time < current_time)
  {
    // Reset all segments if seeking backwards
    for (auto &segment : segments)
    {
      segment->reset();
    }
    current_time = Duration::zero();
    update_segments(target_time);
  }
  else
  {
    Duration delta = target_time - current_time;
    update_segments(delta);
  }

  current_time = target_time;
}

template <typename T>
Timeline<T>::SegmentPtr Timeline<T>::get_segment(Span<const char> label)
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
void Timeline<T>::update_segments(Duration delta)
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
      Duration segment_end = segment->startTime + segment->duration;
      if (current_time <= segment_end && current_time >= segment->start_time)
      {
        segment->update(current_time, delta);
      }
    }
  }
}

template <Animatable T>
void BasicSegment<T>::update(Duration current_time, Duration delta)
{
  animation.update(current_time, delta);
  this->interpolated_value = animation.current_value;
}

template <Animatable T>
void KeyFrameSegment<T>::calculate_duration()
{
  this->duration = 0.0f;
  for (const auto &kf : keyframes)
  {
    this->duration += kf.duration;
  }
}

template <Animatable T>
void KeyFrameSegment<T>::update(Duration _time, Duration delta)
{
  if (keyframes.size() < 2)
    return;

  current_time              = (_time - this->start_time) + delta;
  Duration accumulated_time = Duration::zero();

  for (size_t i = 0; i < keyframes.size() - 1; ++i)
  {
    accumulated_time += keyframes[i].duration;

    if (current_time <= accumulated_time)
    {
      const KeyFrame<T> &current_kf = keyframes[i];
      const auto        &next_kf    = keyframes[i + 1];

      Duration local_start_time = accumulated_time - current_kf.duration;
      Duration local_time =
          (current_time - local_start_time) / current_kf.duration;
      f32 eased_time = current_kf.get_ease_fn(local_time);

      this->interpolated_value =
          lerp(current_kf.value, next_kf.value, eased_time);
      break;
    }
  }
}

}        // namespace ash