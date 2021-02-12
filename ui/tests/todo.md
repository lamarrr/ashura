
# vlk::ui TODO

- children replacing, adding and/or removal and how it will affect the trees

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
- a view widget's extent may and may not depend on its children extent, we don't take care of this
- view extent??? on-change (only triggers a re-layout from its layout node down to its children)
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


- view offset scrolling
- oov views handling (LRU_resolve_views), process should be -> resolve views -> resolve snapshots
- state management for effects
- finish calculating view offset
- work on view offsets
- DO WE??? we have to touch and calculate for all as we don't know which changed, and we'd have to trasverse the tree anyway to know which changed.
- finish implementing layout tree for views and take offset so we can know the position on the parent and on the view
- insertion order of the effects
- children reference invalidation.
- sequence of operation between layout, render, tick, and input event handling
- evaluate cheapness of the effects (they are of no use if in fact they aren't cheap to use???)
- simple effects: skew, transform, rotate, warp, clip, opacity, transform, visibility, e.t.c. (constexpr if possible) (some effects are a combination of post and pre-effects) effects on an effect widget will have no effect
- on_post_effect_changed, pre_effect_changed
- we also need to keep track of the change of the effects as it affects the overall canvas caching/tiling
- can we optimize layout re-calculation  for widgets/views that don't depend on their children's layouts?
- should each view have it's own cache or should we draw everything into a single buffer and check if any part of it is dirty and clear that dirty area instead.
- ATM, a view widget will allot uint32_max to its children, views are not necessarily unconstrained, they can be constrained along x and can be constrained along y, or both.
- check whether the view layout calculations will affect the clamps we have in place.
- for eeasy children removal, should we store a pointer to the view and node tree? so we can easily modify the node at that point and rebuild the tree as needed. we'd just have to perform a binary search on the snapshots using the z_index to remove the children update all caceh; render tree, layout tree
- *should we make snapshot contain a view instead? since views can overlap with other views and/or widgets (in consideration to stacking). PENDING-ANSWER: a stack widget can just group them and create a separate view for one of them if needed?
- *views seem to have the problem mentioned above

# vlk::ui NOTES
- there must be no stage after the snapshot stage, we need to ensure snapshots have all data needed for rendering


# Widget tree rebuilding
- due to events being processed at ticks we have to check if any child has been removed and rebuild the whole layout tree and render tree, how do we keep track of snapshots without discarding their cache content? this also implicates that we need to be careful of how we process ticks. we need to tick -> check children dirtiness (rebuild render and layout tree) (since we refer to their data on the trees) -> check layout dirtiness (rebuild render and layout trees) -> check render dirtiness (rebuild render tree).
- how do we discard snapshots for only widgets that actually leave this widget tree?



# future-TODO
- add widget debug info
- overdraw handling (thread-safe, lifetime-safe design pattern i.e. worker, scheduler, draw-splitting between frames, etc.)
- jank reduction (rendering is presently done synchronously at once). consider performance and code implications of janking here
- separating work/tasks into separate threads and how it is done in other projects

- Optimizing layout calculations:
- we need to recalculate layout in batches as multiple widgets can change at once and can have varying effects on the layout
- we should walk through the widget tree and recalculate as necessary, instead of accumulating and finding which changed.

# special care and consideration for testing
1. pointer invalidation between widgets, widget layout tree and render tree. state syncing, etc.


# STX TODO
1. deduction guide for pointer pair??? will cause issues like matching 0 and nullptr
2. deduction guide for some_ref to accept const? some_cref
3. implicit conversions?
