# Constants:
```
PI                                                                               TYPE: float
TAU                                                                              TYPE: float
PHI                                                                              TYPE: float
e                                                                                TYPE: float
TRUE                                                                             TYPE: bool
FALSE                                                                            TYPE: bool
GL_NEAREST                                                                       TYPE: int
GL_LINEAR                                                                        TYPE: int
GL_REPEAT                                                                        TYPE: int
GL_MIRRORED_REPEAT                                                               TYPE: int
GL_CLAMP_TO_EDGE                                                                 TYPE: int
GL_CLAMP_TO_BORDER                                                               TYPE: int
GL_RGBA                                                                          TYPE: int
GL_RGB                                                                           TYPE: int
GL_RG                                                                            TYPE: int
GL_RED                                                                           TYPE: int
GL_RGBA8                                                                         TYPE: int
GL_RGB8                                                                          TYPE: int
GL_RG8                                                                           TYPE: int
GL_R8                                                                            TYPE: int
GL_R32F                                                                          TYPE: int
GL_RGBA32F                                                                       TYPE: int
GL_R3_G3_B2                                                                      TYPE: int
GL_SRGB8                                                                         TYPE: int
GL_SRGB8_ALPHA8                                                                  TYPE: int
```

# Functions:
## Returning nil:
```
```

## Returning bool:
```
bool left_mouse_button_is_down()                                                 Returns true if the left mouse button is currently down
bool right_mouse_button_is_down()                                                Returns true if the right mouse button is currently down
bool left_mouse_button_was_clicked()                                             Returns true if the left mouse button was pressed in the last frame.
bool right_mouse_button_was_clicked()                                            Returns true if the right mouse button was pressed in the last frame.
bool key_is_down(str key)                                                        Returns true if `key` is down. `key` can be any letter in the English alphabet.
bool key_was_pressed(str key)                                                    Returns true if `key` was pressed . `key` can be any letter in the English alphabet.
bool shaq_reloaded_this_frame()                                                  Returns true if Shaq performed an internal reload operation this frame.
bool shaq_reloaded_last_frame()                                                  Returns true if Shaq performed an internal reload operation last frame.
bool checkbox(str label, bool default)                                           Creates an checkbox widget with the label `label` and default value `default`
bool copy_bool(str shader, str var)                                              Copies the value last assigned to the uniform variable `var` in the shader `shader`
```

## Returning int:
```
int int(float x)                                                                 Typecast float to int.
int mini(int a, int b)                                                           Returns the minimum of `a` and `b`.
int maxi(int a, int b)                                                           Returns the maximum of `a` and `b`.
int randi(int min, int max)                                                      Returns a random number in [`min`, `max`].
int iota()                                                                       Returns the number of times it's been called. See the `iota` in golang.
int frame_count()                                                                Returns the frame count.
int viewport_resolution_x()                                                      Returns the X dimension of the current window resolution
int viewport_resolution_y()                                                      Returns the Y dimension of the current window resolution
int signed(uint x)                                                               Typecast uint to int.
int drag_int(str label, float v, int min, int max, int default)                  Creates an integer slider widget with the label `label`, speed `v`, minimum and maximum allow values `min` and `max`, and default value `default`
int input_int(str label, int default)                                            Creates an input widget for integers with the label `label` and default value `default`
int copy_int(str shader, str var)                                                Copies the value last assigned to the uniform variable `var` in the shader `shader`
```

## Returning uint:
```
uint unsigned(int x)                                                             Typecast int to uint.
uint xor(uint a, uint b)                                                         bitwise XOR of `a` and `b`.
uint and(uint a, uint b)                                                         bitwise AND of `a` and `b`.
uint or(uint a, uint b)                                                          bitwise OR of `a` and `b`.
uint not(uint x)                                                                 bitwise NOT of `x`.
uint lshift(uint x, uint n)                                                      left shift of `x` by `n`.
uint rshift(uint x, uint n)                                                      right shift of `x` by `n`.
uint rol(uint x, uint n)                                                         left rotate of `x` by `n`.
uint ror(uint x, uint n)                                                         right rotate of `x` by `n`.
uint copy_uint(str shader, str var)                                              Copies the value last assigned to the uniform variable `var` in the shader `shader`
```

## Returning float:
```
float float(int x)                                                               Typecast int to float.
float time()                                                                     Returns the program runtime in seconds.
float deltatime()                                                                Returns the frame delta time in seconds.
float rand(float min, float max)                                                 Returns a random number in [`min`, `max`].
float sqrt(float x)                                                              Returns the square root of `x`.
float pow(float x, float y)                                                      Returns the result of `x` raised to the power `y`
float exp(float x)                                                               Returns the result of `e` raised to the power `x`
float log(float x)                                                               Returns the natural logarithm of `x`
float exp2(float x)                                                              Returns the result of 2 raised to the power `x`
float log2(float x)                                                              Returns the base-2 logarithm of `x`
float sin(float x)                                                               Returns the sine of `x`
float cos(float x)                                                               Returns the cosine of `x`
float tan(float x)                                                               Returns the tangent of `x`
float asin(float x)                                                              Returns the principal value of the arc sine of `x`
float acos(float x)                                                              Returns the arc cosine of `x`
float atan(float x)                                                              Returns the principal value of the arc tangent of `x`
float atan2(float y, float x)                                                    Returns the principal value of the arc tangent of `y` / `x`, using the sine of the two arguments to determine the quadrant of the result
float round(float x)                                                             Returns the integer value closest to `x`, as a float
float floor(float x)                                                             Returns the integer part of `x`, as a float
float ceil(float x)                                                              Returns the smallest integer that is larger than `x`, as a float
float fract(float x)                                                             Returns the fractional part of `x`
float min(float a, float b)                                                      Returns the minimum of `a` and `b`
float max(float a, float b)                                                      Returns the maximum of `a` and `b`
float clamp(float min, float max, float x)                                       Returns x clamped to the range [`min`,`max`]
float lerp(float a, float b, float t)                                            Linearly interpolates between `a` and `b` for values of `t` in [0, 1]. I.e. lerp(a,b,t) = a*(1-t)+b*t
float ilerp(float a, float b, float x)                                           Calculates the inverse of lerp(a,b,t). I.e. solves the equation x = a*(1-t)+b*t for t.
float remap(float in_min, float in_max, float out_min, float out_max, float x)   See Freya Holmér's talks :-)
float lerpsmooth(float a, float b, float dt, float omega)                        See Freya Holmér's talks :-)
float smoothstep(float t)                                                        Steps, smoothly. :3
float radians(float degrees)                                                     Converts degrees into radians
float perlin3D(float x, float y, float z)                                        Perlin noise at (x,y,z)
float aspect_ratio()                                                             Returns the current window aspect ratio (width/height)
float vec2_distance(vec2 a, vec2 b)                                              Returns the absolute distance between `a` and `b`
float vec2_length(vec2 v)                                                        Returns the absolute length of `v`
float vec2_dot(vec2 a, vec2 b)                                                   Returns the dot product of `a` and `b`
float vec3_distance(vec3 a, vec3 b)                                              Returns the absolute distance between `a` and `b`
float vec3_length(vec3 v)                                                        Returns the absolute length of `v`
float vec3_dot(vec3 a, vec3 b)                                                   Returns the dot product of `a` and `b`
float vec4_distance(vec4 a, vec4 b)                                              Returns the absolute distance between `a` and `b`
float vec4_length(vec4 v)                                                        Returns the absolute length of `v`
float vec4_dot(vec4 a, vec4 b)                                                   Returns the dot product of `a` and `b`
float input_float(str label, float default)                                      Creates an input widget for floats with the label `label` and default value `default`
float slider_float(str label, float min, float max, float default)               Creates an float slider widget with the label `label`, minimum and maximum allow values `min` and `max`, and default value `default`
float slider_float_log(str label, float min, float max, float default)           Creates an float slider widget, with logarithmic scaling, with the label `label`, minimum and maximum allow values `min` and `max`, and default value `default`
float copy_float(str shader, str var)                                            Copies the value last assigned to the uniform variable `var` in the shader `shader`
```

## Returning vec2:
```
vec2 vec2(float x, float y)                                                      Creates a 2D vector with components `x` and `y`
vec2 vec2_from_polar(float r, float phi)                                         Creates a 2D vector from the polar coordinates `r` and `phi`
vec2 vec2_normalize(vec2 v)                                                      Returns the normalized vector of `v`
vec2 vec2_mul_scalar(vec2 v, float s)                                            Calculates the scalar-vector multiplication `s`*`v`
vec2 vec2_lerp(vec2 a, vec2 b, float t)                                          Linearly interpolates between `a` and `b` for values of `t` in [0, 1]. I.e. lerp(a,b,t) = a*(1-t)+b*t
vec2 vec2_slerp(vec2 a, vec2 b, float t)                                         Interpolates between `a` and `b` for values of `t` in [0, 1] with constant speed along an arc on the unit circle.
vec2 mouse_position()                                                            Returns the current mouse position, in pixel coordinates.
vec2 mouse_position_last()                                                       Returns the mouse position from the last frame, in pixel coordinates.
vec2 mouse_drag_position()                                                       Returns the mouse position from when the left mouse button was last held, in pixel coordinates.
vec2 input_vec2(str label, vec2 default)                                         Creates an input widget for 2D vectors with the label `label` and default value `default`
vec2 copy_vec2(str shader, str var)                                              Copies the value last assigned to the uniform variable `var` in the shader `shader`
```

## Returning vec3:
```
vec3 vec3(float x, float y, float z)                                             Creates a 3D vector with components `x`, `y`, and `z`
vec3 vec3_from_spherical(float r, float phi, float theta)                        Creates a 2D vector from the spherical coordinates `r`, `phi`, and `theta`
vec3 vec3_normalize(vec3 v)                                                      Returns the normalized vector of `v`
vec3 vec3_mul_scalar(vec3 v, float s)                                            Calculates the scalar-vector multiplication `s`*`v`
vec3 vec3_lerp(vec3 a, vec3 b, float t)                                          Linearly interpolates between `a` and `b` for values of `t` in [0, 1]. I.e. lerp(a,b,t) = a*(1-t)+b*t
vec3 vec3_slerp(vec3 a, vec3 b, float t)                                         Interpolates between `a` and `b` for values of `t` in [0, 1] with constant speed along an arc on the unit sphere.
vec3 vec3_cross(vec3 a, vec3 b)                                                  Returns the cross product of `a` and `b`
vec3 vec3_xyz(vec4 v)                                                            Returns the x,y, and z components of `v` as a vec3
vec3 input_vec3(str label, vec3 default)                                         Creates an input widget for 3D vectors with the label `label` and default value `default`
vec3 copy_vec3(str shader, str var)                                              Copies the value last assigned to the uniform variable `var` in the shader `shader`
```

## Returning vec4:
```
vec4 vec4(float x, float y, float z, float w)                                    Creates a 4D vector with components `x`, `y`, `z`, and `w`
vec4 vec4_normalize(vec4 v)                                                      Returns the normalized vector of `v`
vec4 vec4_mul_scalar(vec4 v, float s)                                            Calculates the scalar-vector multiplication `s`*`v`
vec4 vec4_lerp(vec4 a, vec4 b, float t)                                          Linearly interpolates between `a` and `b` for values of `t` in [0, 1]. I.e. lerp(a,b,t) = a*(1-t)+b*t
vec4 rgba(int hexcode)                                                           Returns a vector with R, G, B, and A components normalized to 0.0 - 1.0 given a color hexcode
vec4 mat4_mul_vec4(mat4 m, vec4 v)                                               Calculates the matrix-vector multiplication `m`*`v`
vec4 input_vec4(str label, vec4 default)                                         Creates an input widget for 4D vectors with the label `label` and default value `default`
vec4 color_picker(str label, vec4 default)                                       Creates a color picker widget with the label `label` and default value `default`
vec4 copy_vec4(str shader, str var)                                              Copies the value last assigned to the uniform variable `var` in the shader `shader`
```

## Returning ivec2:
```
ivec2 ivec2(int x, int y)                                                        Creates a 2D integer vector with components `x` and `y`
ivec2 viewport_resolution()                                                      Returns the current viewport/window resolution
ivec2 resolution_of(str shader)                                                  Returns the resolution of `shader`
ivec2 resolution()                                                               Returns the resolution of the shader to which the current attribute/uniform belongs
ivec2 copy_ivec2(str shader, str var)                                            Copies the value last assigned to the uniform variable `var` in the shader `shader`
```

## Returning ivec3:
```
ivec3 ivec3(int x, int y, int z)                                                 Creates a 3D integer vector with components `x`, `y`, and `z`
ivec3 copy_ivec3(str shader, str var)                                            Copies the value last assigned to the uniform variable `var` in the shader `shader`
```

## Returning ivec4:
```
ivec4 ivec4(int x, int y, int z, int w)                                          Creates a 4D integer vector with components `x`, `y`, `z`, and `w`
ivec4 copy_ivec4(str shader, str var)                                            Copies the value last assigned to the uniform variable `var` in the shader `shader`
```

## Returning mat2:
```
mat2 mat2(vec2 c0, vec2 c1)                                                      Creates a 2x2 matrix with column vectors `c0` and `c1`
mat2 mat2_id()                                                                   Creates a 2x2 identity matrix
mat2 copy_mat2(str shader, str var)                                              Copies the value last assigned to the uniform variable `var` in the shader `shader`
```

## Returning mat3:
```
mat3 mat3(vec3 c0, vec3 c1, vec3 c2)                                             Creates a 3x3 matrix with column vectors `c0`, `c1`, and `c2`
mat3 mat3_id()                                                                   Creates a 3x3 identity matrix
mat3 copy_mat3(str shader, str var)                                              Copies the value last assigned to the uniform variable `var` in the shader `shader`
```

## Returning mat4:
```
mat4 mat4(vec4 c0, vec4 c1, vec4 c2, vec4 c3)                                    Creates a 4x4 matrix with column vectors `c0`, `c1`, `c2`, and `c3`.
mat4 mat4_id()                                                                   Creates a 4x4 identity matrix.
mat4 mat4_make_scale(vec3 v)                                                     Creates a 4x4 scaling matrix for 3D vectors with scaling coefficients for the x, y, and z-axes given by `v`.
mat4 mat4_make_rotation(float angle, vec3 axis)                                  Creates a 4x4 rotation matrix for 3D vectors where the rotation operation is given by `angle` and `axis`.
mat4 mat4_make_translation(vec3 v)                                               Creates a 4x4 translation matrix for 3D vectors where translation components for the x, y, and x-axes is given by `v`.
mat4 mat4_look_at(vec3 camera, vec3 target, vec3 up)                             Creates a 4x4 "look-at" view matrix, given a camera position `camera`, a target position `target`, and up-vector `up`.
mat4 mat4_scale(mat4 m, vec3 v)                                                  Applies a scale-operation on `m` given scaling coefficients in `v`.
mat4 mat4_rotate(mat4 m, float angle, vec3 axis)                                 Applies a rotation-operation on `m` given `angle` and `axis`.
mat4 mat4_translate(mat4 m, vec3 v)                                              Applies a translation-operation on `m` given translation components in `v`.
mat4 mat4_mul_mat4(mat4 lhs, mat4 rhs)                                           Calculates the matrix-matrix multiplication `lhs`*`rhs`
mat4 mat4_mul_scalar(mat4 m, float s)                                            Calculates the matrix-scalar multiplication `m`*`s`
mat4 copy_mat4(str shader, str var)                                              Copies the value last assigned to the uniform variable `var` in the shader `shader`
```

## Returning str:
```
```

## Returning texture:
```
texture load_image(str filepath)                                                 Returns a reference to a texture loaded from `filepath`
texture load_image_ex(str filepath, i32 filter, i32 wrap)                        Returns a reference to a texture loaded from `filepath` with the given filter and wrap mode
texture output_of(str shader)                                                    Returns a reference to a texture rendered to by the shader `shader` in this frame. Calling this function implicitly defines the render order.
texture output_of_ex(str shader, i32 filter, i32 wrap)                           Returns a reference to a texture rendered to by the shader `shader` in this frame with the given filter and wrap mode. Calling this function implicitly defines the render order.
texture last_output_of(str shader)                                               Returns a reference to a texture rendered to by the shader `shader` in the last frame.
texture last_output_of_ex(str shader, i32 filter, i32 wrap)                      Returns a reference to a texture rendered to by the shader `shader` in the last frame with the given filter and wrap mode.
```

## Returning type-/namechecker error:
```
```

