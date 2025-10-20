#version 450 core

out vec4 frag_color;

uniform ivec2 iresolution;
uniform float zoom;
uniform int max_iterations;
uniform vec2 position;

uniform bool animate;
uniform float animate_zoom;

vec2 f(vec2 v) {
    float x = pow(v.x, 2) - pow(v.y, 2);
    float y = 2 * v.x * v.y;
    return vec2(x, y);
}

void main()
{
    vec2 uv, c, z; 
    int i;

    uv = gl_FragCoord.xy / iresolution.y;
    uv = uv * 2.0 - vec2(1.0);

    if (animate) {
        c = position + uv * (1.0 / animate_zoom);
    } else {
        c = position + uv * (1.0 / zoom);
    }

    z = vec2(0.0);
    for (i = 0; i < max_iterations; i++) {
        z = f(z) + c;
        if (length(z) > 2.0) {
            break;
        }
    }
    float l = float(i)/float(max_iterations);
    frag_color = vec4(l, l, l, 1.0);
}

