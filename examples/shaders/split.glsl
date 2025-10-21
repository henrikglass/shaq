#version 330 core

out vec4 frag_color;

uniform sampler2D input_color_tex_A;
uniform sampler2D input_color_tex_B;

uniform ivec2 iresolution;
uniform float time;

void main()
{
    vec2 uv = gl_FragCoord.xy / iresolution;//iresolution.y;
    if (uv.x+uv.y > (sin(time) + 1.0)) {
        frag_color = texture(input_color_tex_B, uv);
    } else {
        frag_color = texture(input_color_tex_A, uv);
    }
}
