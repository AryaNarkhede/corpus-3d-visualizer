#version 330 core

layout(location = 0) in vec3 aPos;

uniform mat4 uViewProjection;
uniform int  uSelectedIndex;   // index of the selected annotation, -1 = none

flat out float vSelected;      // 1.0 if this vertex is the selected marker

void main()
{
    gl_Position = uViewProjection * vec4(aPos, 1.0);

    if (uSelectedIndex >= 0 && gl_VertexID == uSelectedIndex) {
        gl_PointSize = 30.0;   // noticeably larger for the selected marker
        vSelected    = 1.0;
    } else {
        gl_PointSize = 20.0;   // larger base size (was 10)
        vSelected    = 0.0;
    }
}
