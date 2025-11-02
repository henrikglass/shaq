#version 330 core

#define PI 3.1415926535

out vec4 frag_color;

uniform sampler2D tex;
uniform ivec2 iresolution;

uniform int kernel_size;
uniform float kernel_scale;
uniform bool vertical;

void main()
{
    ivec2 tex_resolution = textureSize(tex, 0);
    vec2 uv = gl_FragCoord.xy / iresolution;
    vec2 s;
    if (vertical) {
        s = kernel_scale * vec2(0.0, 1.0) / float(tex_resolution.y);
    } else {
        s = kernel_scale * vec2(1.0, 0.0) / float(tex_resolution.x);
    }
   
    int h = kernel_size / 2;
    float sigma = (kernel_size - 1) / 6.0;
    vec3 sum = vec3(0.0);
    float wsum = 0;
    for (int i = -h; i < h + 1; i++) {
        vec2 p = uv + i*s;
        float w = (1.0 / (exp((i*i)/(2.0*sigma*sigma))));
        sum += w * texture(tex, p).rgb;
        wsum += w;
    }
    sum /= wsum; // normalize afterwards 
    
    frag_color = vec4(vec3(sum), 1.0);
}
