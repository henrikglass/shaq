#version 330 core

out vec4 frag_color;

uniform sampler2D tex;
uniform ivec2 iresolution;
uniform vec4 background;

void main()
{
    vec2 uv = gl_FragCoord.xy / iresolution;
    uv *= 2;
    uv -= vec2(0.5);
    vec4 texel = texture(tex, uv);
    frag_color = mix(background, texel, texel.a);
    frag_color.a = 1.0;
}
