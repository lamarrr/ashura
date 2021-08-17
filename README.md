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


# Progress Track

[] Widget System
[] Plugin System (Subsystems)
[] Compositing (Tiled Rendering)
[] Clipboard
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
