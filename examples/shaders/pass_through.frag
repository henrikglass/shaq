#version 450 core

out vec4 frag_color;

//uniform sampler2D tex;

void main()
{
    //vec2 uv = gl_FragCoord.xy / iresolution;//iresolution.y;
    //frag_color = vec4(texture(input_color_tex, uv).rgb, 1.0);
    frag_color = vec4(1.0, 0.5, 1.0, 1.0);
}
