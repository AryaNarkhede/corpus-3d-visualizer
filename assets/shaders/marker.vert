#version 330 core

layout(location = 0) in vec3 aPos;

uniform mat4  uViewProjection;
uniform float uPointSize;

void main()
{
    gl_Position  = uViewProjection * vec4(aPos, 1.0);
    gl_PointSize = uPointSize;
}
