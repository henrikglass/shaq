#version 330 core

out vec4 frag_color;

uniform sampler2D plot_data;
uniform ivec2 iresolution;
uniform float scale;
uniform vec4 bg_color;
uniform float scale_pos;

float sdf_line_segment(vec2 a, vec2 b, vec2 p)
{
    vec2 pa = p - a;
    vec2 ba = b - a;
    float h = clamp(dot(pa, ba)/dot(ba, ba), 0.0, 1.0);
    return length(pa - ba*h);
}

float sdf_scale_marker(float y, float s)
{
    float r = y - s*round(y/s);
    return r;
}

void main()
{
    vec2 p = (2*gl_FragCoord.xy - iresolution) / iresolution.x;
    vec2 uv = gl_FragCoord.xy / iresolution;
    vec2 pixel = (vec2(1.0) / iresolution);
    float y = (2*uv.y - 1);

    /* set background color */
    frag_color = bg_color;

    /* scale-indicators */
    float x = scale_pos;
    float w = 0.01;
    float s = 10.0;
    float ts = scale;
    while (ts > 0.009) {
        ts *= 0.1;
        s *= 0.1;
    }
    for (int i = 0; i < 3; i++) {
        float d = sdf_scale_marker(y, s*scale);
        if (uv.x > (x - w) && uv.x < (x + w)) {
            frag_color = mix(frag_color, vec4(0.35, 0.35, 0.35, 1.0), 1 - smoothstep(0.0, 2*pixel.y, abs(d)));
        }
        w *= 2;
        s *= 10;
    }

    /* zero-line */
    frag_color = mix(frag_color, vec4(0.5, 0.5, 0.5, 1.0), 1 - smoothstep(0.0, 2*pixel.y, abs(y)));

    vec4 texel = texelFetch(plot_data, ivec2(gl_FragCoord.xy), 0);
    frag_color.rgb = mix(frag_color.rgb, texel.rgb, texel.a);
    frag_color.a = 1.0;
}


