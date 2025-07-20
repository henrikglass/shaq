#version 450 core

out vec4 frag_color;

uniform sampler2D input_color_tex;
uniform ivec2 iresolution;

void main()
{
    //frag_color = vec4(1.000, 0.000, 1.000, 1.0);
    vec2 uv = gl_FragCoord.xy / iresolution;//iresolution.y;
    //frag_color = vec4(uv, 0.0, 1.0);
    vec4 texture_color = texture(input_color_tex, uv);
    frag_color = vec4(vec3(1.0 - texture_color), 1.0);
}
