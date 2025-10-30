#version 330 core

out vec4 frag_color;

uniform sampler2D tex;
uniform ivec2 iresolution;
uniform bool reloaded;

uniform bool pen_down;
uniform vec2 pen_position;
uniform vec2 pen_position_last;
uniform vec4 pen_color;
uniform float pen_radius;

uniform bool eraser_down;
uniform vec4 eraser_color;
uniform float eraser_radius;

/* 
 * signed distance function for a line segment 
 */
float sdf_line_segment(vec2 a, vec2 b, vec2 p)
{
    vec2 pa = p - a;
    vec2 ba = b - a;
    float h = clamp(dot(pa, ba)/dot(ba, ba), 0.0, 1.0);
    return length(pa - ba*h);
}

void main()
{
    /* Erase canvas on the first frame */
    if (reloaded) {
        frag_color = eraser_color;
        return;
    }

    /* Draw canvas */
    frag_color = texture(tex, gl_FragCoord.xy / iresolution);

    /* Draw pen/eraser strokes */
    if (sdf_line_segment(pen_position_last, pen_position, gl_FragCoord.xy) < pen_radius && pen_down) {
        frag_color = pen_color;
    } else if (sdf_line_segment(pen_position_last, pen_position, gl_FragCoord.xy) < eraser_radius && eraser_down) {
        frag_color = eraser_color;
    }
}
