#version 330 core

out vec4 frag_color;

uniform sampler2D input_color_tex;
uniform ivec2 iresolution;

void main()
{
    vec2 uv = gl_FragCoord.xy / iresolution;//iresolution.y;
    vec4 texture_color = texture(input_color_tex, uv);
    frag_color = vec4(vec3(1.0 - texture_color), 1.0);
}
