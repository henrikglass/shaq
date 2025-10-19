
#version 450 core

out vec4 frag_color;

uniform sampler2D input_texture_1;
uniform sampler2D input_texture_2;

uniform ivec2 iresolution;
uniform vec2 splitter_position;
uniform float splitter_thickness;
uniform vec4 splitter_color;

void main()
{
    vec2 uv = gl_FragCoord.xy / iresolution;
    float x = splitter_position.x / iresolution.x;
    if (uv.x > x + 0.5*splitter_thickness/iresolution.x) {
        frag_color = texture(input_texture_2, uv);
    } else if (uv.x < x - 0.5*splitter_thickness/iresolution.x) {
        frag_color = texture(input_texture_1, uv);
    } else {
        frag_color = splitter_color;
    }
}
