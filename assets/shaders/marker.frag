#version 330 core

uniform vec4 uColor;

out vec4 fragColor;

void main()
{
    // Draw a circular point sprite by discarding corners of the GL_POINTS quad.
    vec2 coord = gl_PointCoord * 2.0 - vec2(1.0);
    if (dot(coord, coord) > 1.0) discard;

    fragColor = uColor;
}
