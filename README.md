# Shaq

![mandelbrot.png](https://github.com/henrikglass/shaq/blob/main/extras/images/mandelbrot.png?raw=true)

Shaq is a tool for developing GLSL shaders and prototyping multi-pass shader pipelines. Shaq is in many 
ways similar to the fantastic [shadertoy.com](https://shadertoy.com) tool, but there are a few main differences:

1. Shaq is an offline tool.
2. Shaq uses plain GLSL shaders.
3. Shaq places emphasis on host-side logic. Embedded into Shaq is tiny scripting language, the Simple
   Expression Language (SEL), which gives the user the ability to specify the values of uniform
   variables, on a frame-by-frame basis, using basic arithmetic expressions.

# Building
To build Shaq entirely from scratch, run:

```bash
$ make cleaner & make
```

## External dependencies
Shaq depends on the following being installed on your system:

* GCC
* GNU Parallel (only necessary for build process)
* GLFW3

# Usage
At the core of Shaq is the \*.ini project file. In this file, you declare to Shaq what shaders to use, what
their respective inputs (uniform variables) are, and which values are assigned to the inputs.

A Shaq project file may contain one or more shaders. Each shader is declared with a name, wrapped in brackets,
followed by an entry called `source`, which specifies a filepath to the shader source file. Finally, each shader
may have zero or more uniform variables that may be assigned from the host program.

Each entry marking a uniform variable shares the same delaration syntax as GLSL and is followed by an `=`
symbol. To the right of the `=` symbol is an expression. The language in which these expressions are written is
called the Simple Expression Language (SEL). Mostly, SEL expressions follow the same rules as arithmetic expressions
in languages like C or GLSL. Each expression evaluates to some value and to some type. The type of an expression
must match the type of the uniform variable on the left-hand-side of the `=` symbol.

SEL has a large number of built-in functions and constants. These may be listed by runnning `./shaq --list-builtins`.
Most of the built-in functions are pure mathematical primitives and functions, like `float sin(float x)`,
`float vec3_dot(vec3 a, vec3 b)`, and `mat4 mat4_make_rotation(float angle, vec3 axis)`. There is also a handful
of functions that provide information from the Shaq runtime, such as `float time()`, `vec2 mouse_position()`,
`ivec2 iresolution()`, and `texture output_of(str shader)`. Lastly, there is a set of functions that create
GUI widgets, into which the user can enter values dynamically at runtime. These functions evaluate to the
values entered into their respective widgets. A few examples are
`slider_float(str label, float min, float max, float default)`, `bool checkbox(str label, bool default)`, and
`vec4 color_picker(str label, vec4 default)`.

## Example
Inside the `examples/` directory there is an example project called `readme_example.ini`. This project
defines three shaders:

* `Mandelbrot` - Draws a grayscale image of the mandelbrot set, given a couple of parameters such as the zoom
  level `zoom`, the zoom position `position`, and the max allowed number of iterations `max_iterations`. The
  zoom level may be overridden by `animate_zoom` if `animate` is set to true.
* `Gradient` - Takes a grayscale image `input_texture` and applies a 2, 3, or 4 step linear gradient given the
  luminance value at each pixel. The gradient colors are given by `gradient_1` through `gradient_4`. A gamma
  correction function may optinonally be applied to the luminance value before the gradient is applied if
  `use_gamma` is set to true. This will effectively skew the distribution of the gradients for values of
  `gamma` other than `1.0`.
* `Split` - Takes two images, `input_texture_1` and `input_texture_2`, and displays either one or the other
  depending on if the X-coordinate of a given pixel is less than or greater than the X-coordinate of
  `splitter_position`. The splitter itself is shown as a vertical bar with a certain thickness
  `splitter_thickness` and color `splitter_color`.

Here is the entire `readme_example.ini` project file:

```ini
; This is an example Shaq project file

; Renders the Mandelbrot set
[Mandelbrot]
source                            = examples/shaders/mandelbrot.glsl
uniform ivec2 iresolution         = iresolution()
uniform float zoom                = slider_float_log("zoom", 1.0, 1000000.0, 1.0)
uniform int max_iterations        = drag_int("max_n_iterations", 1, 1000, 192)
uniform vec2 position             = input_vec2("position", vec2(-0.74364, 0.13182))
uniform bool animate              = checkbox("Animate", FALSE)
uniform float animate_zoom        = min(pow(0.01*slider_float("Animation Speed", 0.01, 10.0, 1.0)*time() + 1.0, 10.0), 1000000.0)

; Applies a 2 to 4 step color gradient on a grayscale image
[Gradient]
source                            = examples/shaders/gradient.glsl
uniform ivec2 iresolution         = iresolution()
uniform sampler2D input_texture   = output_of("Mandelbrot")
uniform vec4 gradient_1           = color_picker("gradient #1", rgba(0x1E1E1EFF))
uniform vec4 gradient_2           = color_picker("gradient #2", rgba(0x353A87FF))
uniform vec4 gradient_3           = color_picker("gradient #3", rgba(0xD900FFFF))
uniform vec4 gradient_4           = color_picker("gradient #4", rgba(0xFFFFFFFF))
uniform int n_gradients           = input_int("# of gradients to use", 4)
uniform bool use_gamma            = checkbox("Apply gamma function", TRUE)
uniform float gamma               = slider_float_log("gamma", 0.1, 10.0, PHI)

; Draws a split view of two images
[Split]
source                            = examples/shaders/split_view.glsl
uniform ivec2 iresolution         = iresolution()
uniform sampler2D input_texture_1 = output_of("Mandelbrot")
uniform sampler2D input_texture_2 = output_of("Gradient")
uniform vec2 splitter_position    = mouse_drag_position()
uniform float splitter_thickness  = 1.0
uniform vec4 splitter_color       = color_picker("splitter color", rgba(0xA02010FF))
```

Note that the output of `Mandelbrot` is assigned to the `input_texture` variable of `Gradient`, and that the
outputs of both `Mandelbrot` and `Gradient` are assigned to `input_texture_1` and `input_texture_2` variables 
of `Split`. In other words, there exists an implied order in which the shaders need to be rendered. Shaq determines 
this order automatically. In the general case, if there exists a cyclic dependency between the two shaders then 
Shaq will produce a warning message and fail to display an image for the affected shaders.

If `readme_example.ini` is opened with Shaq it will look something like this:

![readme_example.png](https://github.com/henrikglass/shaq/blob/main/extras/images/readme_example.png?raw=true)

I encourage playing around with the source code of the shaders and making edits to the \*.ini project file
while Shaq is running. Shaq will automatically reload and recompile everything as necessary upon changes being
made to any of these files.

# Bugs
Shaq is a relatively young project and probably contains a few bugs. Please message me or open an issue if you
experience any bugs or crashes.

# Why is it named after a basketball player?
The name Shaq is an abbreviaton of Shader Quick. Or Shader Qompositor. Or Shaquille O' Neal if you're
being adamant about it. I haven't really decided. Don't ask me about it again. And don't ask me to
spell compositor, ok?

# Planned features and fixes

* Embed a tiny text editor
* Support uniform arrays
* SEL - Add missing basic functions, e.g. matrix multiplication.
* SEL - Add missing documentation in `--list-builtins`.
* SEL - load\_video()?
* SEL - bool key_pressed(str c) and bool key_down(str c) functions
* Degrade OpenGL version on systems with versions < 4.5
* Fix naughty unaligned data storage in `selvm.c` (Run with -fsantize=undefined)

# Credits
This project uses open source components:

[HGL](https://github.com/henrikglass) - Copyright (c) 2023-2025 Henrik A. Glass

[stb\_image.h](https://github.com/nothings/stb/blob/master/stb_image.h) - Copyright (c) 2017 Sean Barrett

[Dear ImGui](https://github.com/ocornut/imgui) - Copyright (c) 2014-2025 Omar Cornut

[ImGuiFileDialog](https://github.com/aiekick/ImGuiFileDialog) - Copyright (c) 2018-2025 Stephane Cuillerdier (aka Aiekick)

