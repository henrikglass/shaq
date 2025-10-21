#version 330 core

out vec4 frag_color;

uniform sampler2D tex;
uniform ivec2 iresolution;

void main()
{
    vec2 uv = gl_FragCoord.xy / iresolution;
    frag_color = texture(tex, uv);
}
