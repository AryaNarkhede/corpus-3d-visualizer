#version 330 core

uniform vec4 uColor;
uniform vec4 uSelectedColor;   // color used for the selected marker

flat in float vSelected;

out vec4 fragColor;

void main()
{
    // Draw a circular point sprite by discarding corners of the GL_POINTS quad.
    vec2 coord = gl_PointCoord * 2.0 - vec2(1.0);
    if (dot(coord, coord) > 1.0) discard;

    fragColor = mix(uColor, uSelectedColor, vSelected);
}
