
# STX TODO
- deduction guide for pointer pair??? will cause issues like matching 0 and nullptr
- deduction guide for some_ref to accept const? some_cref
- implicit conversions?
- symbol visibility macros
- separating macros
- make versioning of exported symbols
- more changes
- investigate performance characteristics of `LIKELY` and `UNLIKELY` on STX?
- create storage types for trivial types (movable, copyable, etc)
- consider enabling implicit conversions in API
- `STX_UNREACHABLE`
- `STX_LIKELY`
- `STX_UNLIKELY` for `unwrap` and the likes
- are visibility macros used correctly?
- we might need `Result::copy` `Result::copy_err` `Option::copy` `Option::copy_none`
- we should be able to return `std::string` that is not released in the `panic` formatter? `ReportQuery` seems unnecessary, `std::string*` `stx::make_report()`
- stx::span doesn't support zero-extent arrays

# vlk::ui TODO
- In the contributing guide state how skia isn't thread-safe and the graphics part is basically a vulkan wrapper
- consider making the parent inject the effects and add them to an effect tree, with all of the widgets having individual effects as a result we need to be able to render the effects independent of the widget, we'll thus need bindings for them
- Diagram of Asset manager sqe, cqe
- DPR support. it should be a property of the surface, we only need to re-rasterize and use the same surfaces as before to draw the new content
- zooming support 
- can we have a stack of surfaces we can pop and attach and release surfaces to? allocating these tiles can be costly, even when zooming, this stack can also handle DPR and zooming and force the tiles to release their surfaces. BENCHMARK. consider scroll swapping instead of allocating and de-allocating. i.e. tile surface stack. we might need to re-allocate when the viewport zoom occurs? the stack will always have enough tile surfaces for the tiles covered by the viewport. we don't necessarily want to use the layout tree for this.


THIs might not be necessary since Skia will most likely be using a memory pool and a memory pool should be able to handle this allocation and re-allocation efficiently


- Implement dynamic runtime subsystems for accessibility and text scale factor
- Subsystem map named subsystems, using enums as identifiers and then other subsystems

- how do we implement stacking without overriding a very large area?

- effects are performed on a per-widget basis
- effects and effect trees to stay in the snapshots or should we only have a transform widgets that implicitly adds effects to its child?, I think this would be a nicer approach
- no effect tree. just parents of type effect injecting effects into their children (order of post effects and pre effects matters) using (update_effect that calls the on_effect_updated proxy method)
- widget floating (views or individual, absolutely positioned or relative, z-index, etc)
- simple effects: skew, transform, rotate, warp, clip, opacity, transform, visibility, e.t.c. (constexpr if possible) (some effects are a combination of post and pre-effects) effects on an effect widget will have no effect
- consider opacity tree? or require manually setting the opacities?
- we need render props that all widgets must have: i.e. opacity???
- we need visibility prop?
- overdraw handling (thread-safe, lifetime-safe design pattern i.e. worker, scheduler, draw-splitting between frames, etc.)

# Notes
- When using skia with vulkan SK_VULKAN macro should reside in the implementation/source files

# Materials
- Accessibility: https://www.youtube.com/watch?v=9yK8DjlNRLM
- TImw travel debugging: https://www.youtube.com/watch?v=pV7AYofV95A
- Life of a Video Frame: https://www.youtube.com/watch?v=t_DFAHDSIiQ
- Virtual Machines of Chromium: https://www.youtube.com/watch?v=BD_lcnkNAk4
- IPC 101: https://www.youtube.com/watch?v=ZdB5P88-w8s
- SLimming paint: https://www.youtube.com/watch?v=5Xv2A7aqJ9Y
- SKiA update: https://www.youtube.com/watch?v=SU58JHK0-3o







Turn off vsync to get raw performance numbers

Profilers:

- Radeon GPU Profiler
- NVIDIA GPU Profiler
- RenderDoc
- High ALU usage tasks need to be moved to Compute, How?  Check APPLE METAL talks at WWDC



Linux Perf Attach to Process:
- ```perf stat -p 3129```




perf sat -e L1-icache-loads,L1-icache-load-misses.... stalls ./bin - search for other flags