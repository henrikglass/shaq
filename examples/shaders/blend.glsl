#version 330 core

out vec4 frag_color;

uniform sampler2D tex_bg;
uniform sampler2D tex_fg;
uniform ivec2 iresolution;
uniform int blend_mode;
uniform float opacity;
uniform bool bypass;

float luminance(vec3 c)
{
    return 0.2126 * c.r + 0.7152 * c.g + 0.0722 * c.b;
}

float average(vec3 c)
{
    return (c.r + c.g + c.b) / 3.0;
}

void main()
{
    vec2 uv = gl_FragCoord.xy / iresolution;
    vec4 bg = texture(tex_bg, uv);
    vec4 fg = texture(tex_fg, uv);

    if (bypass) {
        frag_color = bg;
        return;
    }

    switch (blend_mode) {
        /* Add */
        case 0: {
            frag_color.rgb = bg.rgb + fg.rgb;
        } break;

        /* Subtract */
        case 1: {
            frag_color.rgb = bg.rgb - fg.rgb;
        } break;

        /* Multiply */
        case 2: {
            frag_color.rgb = bg.rgb * fg.rgb;
        } break;

        /* Color Burn */
        case 3: {
            frag_color.rgb = 1.0 - (1.0 - bg.rgb) / fg.rgb;
        } break;

        /* Screen */
        case 4: {
            frag_color.rgb = 1.0 - (1.0 - bg.rgb)*(1.0 - fg.rgb);
        } break;

        /* Color Dodge */
        case 5: {
            frag_color.rgb = bg.rgb / (1.0 - fg.rgb);
        } break;

        /* Overlay */
        case 6: {
            float t = round(luminance(bg.rgb));
            frag_color.rgb = (1.0 - t) * 2 * (bg.rgb * fg.rgb) +
                             t * (1.0 - 2 * (1.0 - bg.rgb)*(1.0 - fg.rgb));
        } break;

        /* Vivid */
        case 7: {
            float t = round(luminance(fg.rgb));
            frag_color.rgb = (1.0 - t) *  (1.0 - 0.5*(1.0 - bg.rgb) / fg.rgb) +
                             t * (0.5 * bg.rgb /(1.0 - fg.rgb));
        } break;

        default: {
            frag_color = vec4(1.0, 0.0, 1.0, 1.0);
        } break;
    }

    float bias = 0.0001;
    frag_color.rgb = mix(bg.rgb, frag_color.rgb, (1.0 - bias) * opacity + bias);
    //frag_color.rgb = (1.0 - opacity) * bg.rgb + opacity * frag_color.rgb;
    //frag_color.rgb = clamp(frag_color.rgb, vec3(0), vec3(1));
    frag_color.a = 1.0;
}
