# shaq
Shaq is a tool for developing GLSL shaders, similar to [shadertoy.com](shadertoy.com), but meant for offline use.

# Usage
At the core of Shaq is the *.ini project file. In this file, you define shaders and their respective inputs. 
Lets say you have two shaders you want to compose: One shader, `mandelbrot`, renders the mandelbrot set at a 
certain zoom level; the zoom level is calculated from a uniform variable `time`. The other shader, `dither`, takes an 
image and applies an ordered dithering filter to it with a certain color bit depth `bpp`. Such a project file may
look something like this:

```ini
[mandelbrot]
source                    = shaders/mandelbrot.glsl
uniform float time        = time()*slider_float_log("timescale", 0.01, 10.0, 1.0)

[dither]
source                    = shaders/dither.glsl
uniform sampler2D input   = output_of("mandelbrot")
uniform int bpp           = 4
```

If you're familiar with shader programming, it should be fairly clear what's going on, even if it's your first 
time looking at a Shaq project file. Each shader is defined with a name, wrapped in brackets. Each shader has 
a source file. Finally, each shader may have zero or more uniform variables that may be assigned from the host 
program. 

Each entry marking a uniform variable shares the same delaration syntax as GLSL and is followed by an `=` 
symbol. To the right of the `=` symbol is an expression. The language in which these expressions are written is
called SEL (for Simple Expression Language). Mostly, SEL expressions follow the same rules as arithmetic expressions
in languages like C or GLSL. Each expression evaluates to some value, and, naturally, each expression has a type. The 
type of an expression must match the type of the uniform variable on the left-hand-side of the `=` symbol. SEL has a 
large number of built-in functions and constants. These may be listed by runnning `./shaq --list-builtins`. One caveat 
with SEL is that it doesn't allow implicit type conversions. So if you want to, say, assign the result of `time()` 
(which is of type float) to a uniform variable of type int, you must explicitly typecast it using the built-in
typecasting function `int(..)` as such: `int(time())`.

Shaq and SEL makes it easy to compose multiple shaders. Instead of having to manually go through the process of creating
offscreen frame buffers, creating render textures, and managing the render order yourself, you can simply define a 
dependency (as shown above with `uniform sampler2D input = output_of(\"mandelbrot\")`), and Shaq will automatically 
figure out in what order things need to be rendered.

# Why is it named after a basketball player?
The name Shaq is an abbreviaton of Shader Quick. Or Shader Qompositor. Or Shaquille O' Neal if you're 
being adamant about it. I haven't really decided. Don't ask me about it again. And don't ask me to 
spell compositor, ok?
