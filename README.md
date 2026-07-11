# PhosphorusGUI

**PhosphorusGUI** is a C library that aims to make creating GUIs in Raylib easier, more customizable, and more reactive.

> ![NOTE]
> PhosphorusGUI is not thread-safe.

## Features

- Ready-to-go out of the box flexible and customizable GUIs
- Lightweight

## Clone the repository

```
git clone https://github.com/loganjellis/PhosphorusGUI.git
cd PhosphorusGUI
```

## Building

```
cmake -S . -B build
cmake --build build
```

### Using PhosphorusGUI (in-directory, place header files in /include, .a files in /lib, and any .dll files next to any executables)

```
target_include_directories(app PRIVATE "path/to/include")
target_link_libraries(app PRIVATE "path/to/lib")
```

### Using PhosphorusGUI (subdirectory)

```
add_subdirectory(PhosphorusGUI)
target_link_libraries(app PRIVATE PhosphorusGUI::phosphorus_gui)
```

## Example

For an example of basic usage of the library, refer to `example.c`:

## Documentation and Information

Please refer to the PhosphorusGUI documentation <a href="docs/html" target="_blank">here</a>.
