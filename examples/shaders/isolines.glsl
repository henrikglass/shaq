#version 330 core

#define NUM_OCTAVES 3

out vec4 frag_color;

uniform sampler2D bnoise;
uniform vec2 resolution;
uniform float time;
uniform int octaves;

float rand(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898,78.233))) * 43758.5453123);
}

float noise(in vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);
    float a = rand(i);
    float b = rand(i + vec2(1.0, 0.0));
    float c = rand(i + vec2(0.0, 1.0));
    float d = rand(i + vec2(1.0, 1.0));
    vec2 u = f*f*(3.0-2.0*f);
    return mix(a, b, u.x) +
           (c - a)* u.y * (1.0 - u.x) +
           (d - b) * u.x * u.y;
}

float fbm(vec2 x) {
	float v = 0.0;
	float a = 0.5;
	vec2 shift = vec2(100);
	// Rotate to reduce axial bias
    mat2 rot = mat2(cos(0.5), sin(0.5), -sin(0.5), cos(0.50));
	for (int i = 0; i < octaves; ++i) {
		v += a * noise(x);
		x = rot * x * 2.0 + shift;
		a *= 0.5;
	}
	return v;
}

void main()
{
    vec2 uv = gl_FragCoord.xy / resolution;
    vec2 uv2 = 0.01*gl_FragCoord.xy;

    //float d = abs(fract((uv.x + 0.7*noise(uv*0.46 + vec2(time)))*50) - 0.5);
    vec2 p = uv + 10*fbm(uv + 0.001*texture(bnoise, uv2).xy + vec2(time));
    float f = fract(10*p.x) - 0.5;
    float d = abs(f);
    float g = fwidth(f);
    //d = smoothstep(0.00, fwidth(d), abs(d));
    //d = smoothstep(0.06, 1.05, d/fwidth(d));
    //d = smoothstep(-g, g, d);
    float gg = step(0.3, g);
    d = 1-mix(smoothstep(0, 1.2, d/g), 1, gg);
    //d = 1-mix(smoothstep(0, 1.0*length(texture(bnoise, uv2).yx), d/g), 1, gg);
    vec3 bg = vec3(0.99, 0.98, 0.96) + 0.05*vec3((texture(bnoise, uv2).rgb*2.0 - 1));
    vec3 fg = vec3(0.4, 0.4, 0.4);
    frag_color.rgb = mix(bg, fg, d);
    frag_color.a = 1.0;
}
