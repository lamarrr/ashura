


# STX TODO
1. deduction guide for pointer pair??? will cause issues like matching 0 and nullptr
2. deduction guide for some_ref to accept const? some_cref
3. implicit conversions?
4. symbol visibility macros
5. separating macros
6. make versioning of exported symbols
7. more changes


- investigate performance characteristics of LIKELY and UNLIKELY on STX?
- create storage types for trivial types (movable, copyable, etc)
- consider enabling implicit conversions in API



STX_UNREACHABLE
STX_LIKELY
STX_UNLIKELY for unwrap and the likes
are visibility macros used correctly?
  // copy() copy_err()
  // copy() copy_none()
//
//
//
//
//
// // future-TODO(lamarrr): accessibility text scale factor, can be changed at runtime? or constant. probably use a subsystem for this
//
//
// 
// TODO(lamarrr): we should be able to return std::string that is not released in the panic formatter? ReportQuery seems unnecessary, std::string*  stx::make_report(  )
//
//
//
//
// TODO(lamarrr): on effects: users would be able to inherit from the base
// widget and override the draw methods of each widget to provide their own draw
// methods that apply one effect or the other. effects that span acrross
// widgets? i.e. parent opacity and intends to fade along with its children.
// TODO(lamarrr): consider opacity tree? or require manually setting the
// opacitgies?
// we need render props that all widgets must have: i.e. opacity
//
// we need visibility prop
//
// is_layout_changed
// is_render_changed
//
// BaseProps? BaseProps::clean_state()..
//


# vlk::ui TODO


// TODO(lamarrr): how do we implement stacking without overriding a very large
// area?

- When using skia with vulkan SK_VULKAN macro should reside in the source file

  // about breaking text into multiple ones, we can: raterize on the CPU and
  // divide as necessary using a text widget
  // or split into multiple widgets outselves using the text metrics
  // waiting for vsync so we don't do too much work
  /// IMPORTANT: make a visualization of the text boundaries

- consider changing to list for easier insertion and deletion of children nodes
- children replacing, adding and/or removal and how it will affect the trees
- effects are performed on a per-widget basis
```cpp
    // complete layout rebuild
    // remove children at node on the tree.
    // if the resulting layout is same as the previous node's then we don't need
    // an update of the dimensions of the layout tree. else, we need to perform
    // a re-layout.
    // the backend should store the view node the widget belongs to so we can
    // remove it.

```

- effects and effect trees to stay in the snapshots or should we only have a transform widgets that implicitly adds effects to its child?, I think this would be a nicer approach
- rename ui to ui
- no effect tree. just parents of type effect injecting effects into their children (order of post effects and pre effects matters) using (update_effect that calls the on_effect_updated proxy method)
- widget floating (views or individual, absolutely positioned or relative, z-index, etc)

```cpp

    // TODO(lamarrr): should_float floating widgets are always drawn last
    // should these have another name?
    std::vector<WidgetSnapshot> in_view_floaters;
    std::vector<WidgetSnapshot> out_of_view_floaters;
    //
    // OR: preferred
    bool is_floating;
    ChildLayout overall_offset;
    //

```


- state management for effects
- insertion order of the effects
- sequence of operation between layout, render, tick, and input event handling
- evaluate cheapness of the effects (they are of no use if in fact they aren't cheap to use???)
- simple effects: skew, transform, rotate, warp, clip, opacity, transform, visibility, e.t.c. (constexpr if possible) (some effects are a combination of post and pre-effects) effects on an effect widget will have no effect
- on_post_effect_changed, pre_effect_changed
- we also need to keep track of the change of the effects as it affects the overall canvas caching/tiling
- should each view have it's own cache or should we draw everything into a single buffer and check if any part of it is dirty and clear that dirty area instead.
- for eeasy children removal, should we store a pointer to the view and node tree? so we can easily modify the node at that point and rebuild the tree as needed. we'd just have to perform a binary search on the snapshots using the z_index to remove the children update all caceh; render tree, layout tree
- *should we make snapshot contain a view instead? since views can overlap with other views and/or widgets (in consideration to stacking). PENDING-ANSWER: a stack widget can just group them and create a separate view for one of them if needed?
- *views seem to have the problem mentioned above


- move trees to impl header



# future-TODO
- overdraw handling (thread-safe, lifetime-safe design pattern i.e. worker, scheduler, draw-splitting between frames, etc.)
- jank reduction (rendering is presently done synchronously at once). consider performance and code implications of janking here
- separating work/tasks into separate threads and how it is done in other projects


# special care and consideration for testing
1. pointer invalidation between widgets, widget layout tree and render tree. state syncing, etc.



# Materials
- Accessibility: https://www.youtube.com/watch?v=9yK8DjlNRLM
- TImw travel debugging: https://www.youtube.com/watch?v=pV7AYofV95A
- Life of a Video Frame: https://www.youtube.com/watch?v=t_DFAHDSIiQ
- Virtual Machines of Chromium: https://www.youtube.com/watch?v=BD_lcnkNAk4
- Sensors in Chromium: https://www.youtube.com/watch?v=skrbujwn-J8
- IPC 101: https://www.youtube.com/watch?v=ZdB5P88-w8s
- EVolution of Chromium SYstem UI: https://www.youtube.com/watch?v=e1dT348THdI
- SLimming paint: https://www.youtube.com/watch?v=5Xv2A7aqJ9Y
- SKiA update: https://www.youtube.com/watch?v=SU58JHK0-3o

