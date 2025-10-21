#version 330 core

out vec4 frag_color;

uniform sampler2D input_texture;
uniform ivec2 iresolution;
uniform vec4 gradient_1;
uniform vec4 gradient_2;
uniform vec4 gradient_3;
uniform vec4 gradient_4;
uniform int n_gradients;
uniform bool use_gamma;
uniform float gamma;

vec4 mix2(float l, vec4 g0, vec4 g1)
{
    return (1.0 - l) * g0 + l * g1;
}

vec4 mix3(float l, vec4 g0, vec4 g1, vec4 g2)
{
    if (l < 0.5) {
        l *= 2;
        return (1.0 - l) * g0 + l * g1;
    } else {
        l -= 0.5;
        l *= 2;
        return (1.0 - l) * g1 + l * g2;
    }
}

vec4 mix4(float l, vec4 g0, vec4 g1, vec4 g2, vec4 g3)
{
    if (l < 1.0/3.0) {
        l *= 3;
        return (1.0 - l) * g0 + l * g1;
    } else if (l < 2.0/3.0) {
        l -= 1.0/3.0;
        l *= 3;
        return (1.0 - l) * g1 + l * g2;
    } else {
        l -= 2.0/3.0;
        l *= 3;
        return (1.0 - l) * g2 + l * g3;
    }
}

void main()
{
    vec2 uv = gl_FragCoord.xy / iresolution;//iresolution.y;
    float l = texture(input_texture, uv).r;
    if (use_gamma) {
        l = pow(l, 1/gamma);
    }
    if (n_gradients == 2) {
        frag_color = mix2(l, gradient_1, gradient_2);
    } else if (n_gradients == 3) {
        frag_color = mix3(l, gradient_1, gradient_2, gradient_3);
    } else if (n_gradients == 4) {
        frag_color = mix4(l, gradient_1, gradient_2, gradient_3, gradient_4);
    } else {
        frag_color = vec4(1, 0, 1, 1); // magenta = error
    }
}
