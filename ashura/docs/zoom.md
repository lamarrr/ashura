# 3D Transforms
Widgets will have coordinates ranging from (0, 0) -> (w, h)
Transforms gotten from the widgets are used as local transforms (i.e. along the local space/origin).
These are used for:
    - Mouse events
    - Pre-draw transformation (i.e. local transforms of each drawn)

Viewport will have 2-component transforms that will be relative to the widget tree:
    - zooming (scaling)
    - panning (translation)


Widgets have a transform that positions them on the viewport. the positions are then scaled and translated to meet the zooming requirements.
widget clip rects are also transformed accordingly.


TODO(lamarrr): this makes dragging for example difficult

# Hit-Testing

if mouse is at position p. apply inverse transform matrix on vector p to give pt.
pt is then hit-tested against all the transformed quads of the widgets. TODO(lamarrr): this would mean offsets gotten would only