#version 330 core

out vec4 frag_color;

uniform sampler2D tex;
uniform ivec2 iresolution;
uniform float blobbiness;
uniform float sharpness;
uniform float persistance;
uniform bool reloaded;

uniform vec3 blob0;
uniform vec3 blob1;
uniform vec3 blob2;
uniform vec3 blob3;
uniform vec3 blob4;
uniform vec3 blob5;
uniform vec3 blob6;
uniform vec3 blob7;
uniform vec3 blob8;
uniform vec3 blob9;

float smoothmin(float a, float b, float k)
{
    k *= 1.0;
    float r = exp2(-a/k) + exp2(-b/k);
    return -k*log2(r);
}

float sdf_circle(vec2 p, float r)
{
    return length(p) - r;
}

void main()
{
    if (reloaded) {
        frag_color = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    /* compounding blur thingy */
    vec2 uv = (gl_FragCoord.xy) / iresolution;
    vec2 sx = (1.0/sqrt(2)) * vec2(1.0 / iresolution.x, 0);
    vec2 sy = (1.0/sqrt(2)) * vec2(0, 1.0 / iresolution.y);
    frag_color = 4.0 * texture(tex, uv) +
                 texture(tex, uv + sx + sy) +
                 texture(tex, uv + sx - sy) +
                 texture(tex, uv - sx + sy) +
                 texture(tex, uv - sx - sy);
    frag_color *= 1.0/8.0f;


    /* blobs */
    vec2 p = (2.0*gl_FragCoord.xy-iresolution.xy) / iresolution.y;
    float d0 = sdf_circle(p - blob0.xy, blob0.z);
    float d1 = sdf_circle(p - blob1.xy, blob1.z);
    float d2 = sdf_circle(p - blob2.xy, blob2.z);
    float d3 = sdf_circle(p - blob3.xy, blob3.z);
    float d4 = sdf_circle(p - blob4.xy, blob4.z);
    float d5 = sdf_circle(p - blob5.xy, blob5.z);
    float d6 = sdf_circle(p - blob6.xy, blob6.z);
    float d7 = sdf_circle(p - blob7.xy, blob7.z);
    float d8 = sdf_circle(p - blob8.xy, blob8.z);
    float d9 = sdf_circle(p - blob9.xy, blob9.z);
    float dA = smoothmin(smoothmin(d0, d1, blobbiness), smoothmin(d2, d3, blobbiness), blobbiness);
    float dB = smoothmin(smoothmin(d4, d5, blobbiness), smoothmin(d6, d7, blobbiness), blobbiness);
    float d = smoothmin(smoothmin(dA, dB, blobbiness), smoothmin(d8, d9, blobbiness), blobbiness);
    vec3 color = vec3(0.0);
    color = mix(color, vec3(10.0), 1.0 - smoothstep(0.0, 1.0/exp(sharpness), abs(d)));
    frag_color = mix(frag_color, vec4(color, 1.0), 1.0 - persistance);
    frag_color.a = 1.0;
}
