# Valkyrie

Experimental 2D UI + 3D Game Engine


NOTE: our code is not exception-safe or exception-tolerant

# Goals

Lean, Simple, Readable, Flexible,  Minimum Possible Edge Cases


Widgets here are minute state machines


# Contributing


## Module Structure

public vlk headers should be in the include and must be one per module????
otherwise provide separate subdirectories for public exposed? and make them under a subdirecotry of the name

i.e.
vlk/async/include/async/async_logger.h

private:
vlk/async/include

Power of lambdas

trivial types? leave copy and move constructors
non-trivial. disable copy but write move constructors.
copying is usually a byte-level semantic
not so unique

Features and Flexibility beat complexity

NOTE: this is more of a C++/C hybrid. C with C++ abstraction and resource cleanup

# Progress Track

[] Widget System
[] Plugin System (Subsystems)
[] Compositing (Tiled Rendering)
[] Async Clipboard
[] Async Runtime
[] Asset Loading & Management
[] Drag & Drop
[] *Accessibility*
[] Keyboard Input
[] Mouse Input
[] Pointer Input
[] Combo Boxes
[] Dropdown Menus
[] Gesture Input
[] Animations (Tweens)
[] Text (UTF-8, UTF-16, UTF-32)
[] System & File Fonts
[] Scroll Preferences
[] Keyboard Shortcuts
[] High-DPI Support
[] Resource Management & Error Reporting

 

# Error handling and Subsystems
# ...
# failure handling
# during prototyping, feel free to use widgets and panic
# but for mainstream ones, you really shouldn't be panicking
#
#
# The OS, the CPU, the ... sees bytes not objects
#
#
#

Control over performance and predictability


(add bobby angular dev tweet on iMGUI table/tree here)
Retained mode, you give the UI the description of your UI, the state it wants to observe and how it'd react to them
giving the system a much better chance at optimizing the UI

Immediate Mode
similar to deferred mode, except instead of a graph, you process your draw calls into a draw list and the draw list is updated on every frame. layout is calculated on every frame as the UI unware of the state change and doesn't have enough information about them.
for medium-sized apps, immediately rendered UI will often render faster than retained mode, but it won't scale to millions of widgets or draw items, and the scalability is the PoC.
You'd end up writing code to manage state and monitor information as your app grows anyway.


