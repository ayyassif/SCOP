#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in int aChartIndex;

out vec2 texCoord;
flat out int chartIndex;

uniform mat4 mvp;

// uniform float offset;

void main()
{
    gl_Position = mvp * vec4(aPos, 1.0);
    texCoord = aTexCoord;
    chartIndex = aChartIndex;
}