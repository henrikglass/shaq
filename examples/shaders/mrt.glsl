#version 450 core

layout (location = 0) out vec4 frag_color_A;
layout (location = 1) out vec4 frag_color_B;
layout (location = 2) out vec4 frag_color_C;

uniform ivec2 iresolution;

void main()
{
    vec2 uv = gl_FragCoord.xy / iresolution;//iresolution.y;
    
    // pink
    frag_color_A = vec4(1.0, 0.0, 0.4, 1.0);
    frag_color_B = vec4(1.0, 0.0, 0.4, 1.0);
    frag_color_C = vec4(1.0, 0.0, 0.4, 1.0);

    if ((int(gl_FragCoord.y) / 20) % 2 == 0){
        frag_color_A = vec4(1.0, 1.0, 0.0, 1.0);
        frag_color_B = vec4(1.0, 0.0, 1.0, 1.0);
        frag_color_C = vec4(0.0, 1.0, 1.0, 1.0);
    }
}
