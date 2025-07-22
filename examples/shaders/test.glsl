#version 450 core

#define PI 3.1415926535

out vec4 frag_color;

uniform float time;
uniform ivec2 iresolution;
uniform vec4 color;

void main()
{
    // (-1, -1) to (1, 1)
    vec2 uv = 2.0*(gl_FragCoord.xy - 0.5*iresolution)/iresolution.y;
    //ivec2 irr = ivec2(800, 600);
    //vec2 uv = 2.0*(gl_FragCoord.xy - 0.5*irr)/irr.y;
    //vec2 uv = gl_FragCoord.xy / iresolution;
    frag_color = vec4(0.118, 0.118, 0.118, 1.0);
   
    float slow_time = 0.3 * time;

    if ((uv.x*uv.x*sin(4*slow_time) + uv.y*uv.y*sin(PI + 5*slow_time)) < 0.1) {
        //frag_color = vec4(1.0, 0.0, 0.4, 1.0);
        frag_color = color;
        //frag_color = vec4(1.0, 1.0, 0.4, 1.0);
    }
}


