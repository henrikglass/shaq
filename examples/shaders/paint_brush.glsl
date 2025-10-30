#version 330 core

out vec4 frag_color;

uniform sampler2D tex;
uniform ivec2 iresolution;

uniform vec2 pen_position;
uniform vec4 pen_color;
uniform float pen_radius;

uniform bool eraser_down;
uniform vec4 eraser_color;
uniform float eraser_radius;

void main()
{
    /* Draw canvas */
    frag_color = texture(tex, gl_FragCoord.xy / iresolution);

    float d = distance(pen_position, gl_FragCoord.xy);
    if (eraser_down) {
        if (d < eraser_radius) {
            if (d > eraser_radius - 2) {
                frag_color = vec4(0.67, 0.67, 0.67, 1.0);
            } else {
                frag_color = eraser_color;
            }
        }
    } else if (d < pen_radius) {
        frag_color = pen_color;
    }
}
