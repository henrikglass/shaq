#version 450 core

out vec4 frag_color;

uniform sampler2D tex;
uniform ivec2 iresolution;

void main()
{
    vec2 uv = gl_FragCoord.xy / iresolution;
    //frag_color = vec4(texture(tex, uv).rgb, 1.0);
    frag_color = texture(tex, uv);
    //frag_color = vec4(1.0, 0.5, 1.0, 1.0);
}
