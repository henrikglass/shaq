#version 330 core

out vec4 frag_color;

uniform sampler2D tex;
uniform ivec2 iresolution;

uniform float threshold;

float luminance(vec3 c)
{
    return 0.2126 * c.r + 0.7152 * c.g + 0.0722 * c.b;
}

void main()
{
    vec2 uv = gl_FragCoord.xy / iresolution;
    vec3 color = texture(tex, uv).rgb;

    if (luminance(color) > threshold) {
        frag_color = vec4(1.0);
    } else {
        frag_color = vec4(0.0, 0.0, 0.0, 1.0);
    }
}
