#version 330 core

out vec4 frag_color;

uniform sampler2D previous_frame;
uniform ivec2 iresolution;
uniform float plot_value_prev;
uniform float plot_value;
uniform float thickness;
uniform int speed;
uniform float scale;
uniform float offs;
uniform bool reloaded;

float sdf_line_segment(vec2 a, vec2 b, vec2 p)
{
    vec2 pa = p - a;
    vec2 ba = b - a;
    float h = clamp(dot(pa, ba)/dot(ba, ba), 0.0, 1.0);
    return length(pa - ba*h);
}

void main()
{
    //if (reloaded) {
    //    frag_color = vec4(0.117, 0.117, 0.117, 1.0);
    //    return;
    //}

    vec2 uv = gl_FragCoord.xy/iresolution;
    vec2 p = (2.0*gl_FragCoord.xy-iresolution.xy) / iresolution;
    vec2 pixel = 1.0 / iresolution;
    frag_color = vec4(0.117, 0.117, 0.117, 1.0);
    frag_color = texelFetch(previous_frame, ivec2(gl_FragCoord.xy) + ivec2(speed, 0), 0);
    frag_color = mix(frag_color, vec4(0.35, 0.35, 0.35, 1.0), 1 - smoothstep(0.0, 2*pixel.y, abs(p.y)));

    float f0 = scale*plot_value_prev;
    float f1 = scale*plot_value;
    vec2 v0 = vec2(1.0 - speed*2*(1.0/iresolution.y), f0);
    vec2 v1 = vec2(1.0, f1);
    float d = sdf_line_segment(v0, v1, p);
    frag_color = mix(frag_color, vec4(0.80, 0.20, 0.20, 1), 1-smoothstep(pixel.y*thickness*1.0, 2.0*pixel.y*thickness, d));
}


