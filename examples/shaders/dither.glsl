#version 450 core

out vec4 frag_color;

uniform sampler2D tex;
uniform ivec2 iresolution;
uniform vec4 background;

uniform int color_depth;
uniform float spread;
uniform float bias;

void main()
{
    vec2 uv = gl_FragCoord.xy / iresolution;
    int x = int(gl_FragCoord.x) % 4;
    int y = int(gl_FragCoord.y) % 4;

    const float bayer4x4[4][4] = {
        {0,   8,  2, 10},
        {12,  4, 14,  6},
        {3,  11,  1,  9},
        {15,  7, 13,  5},
    };

    vec4 c = texture(tex, uv);
    int n_col = int(exp2(color_depth));

    float M = (1.0/16.0) * bayer4x4[y][x] - 0.5;
    float r = clamp(0.0, 1.0, c.r + M*spread + bias);
    float g = clamp(0.0, 1.0, c.g + M*spread + bias);
    float b = clamp(0.0, 1.0, c.b + M*spread + bias);
    r = floor(r * (n_col - 1) +  0.5) / (n_col - 1);
    g = floor(g * (n_col - 1) +  0.5) / (n_col - 1);
    b = floor(b * (n_col - 1) +  0.5) / (n_col - 1);

    frag_color = vec4(r, g, b, 1);
}
