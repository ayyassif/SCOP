#version 330 core

out vec4 FragColor;

in vec2 texCoord;
flat in int chartIndex;

uniform sampler2D Texture;
uniform int renderMode;

vec3 chartColor(int chartIndex)
{
    const vec3 colors[8] = vec3[](
        vec3(0.95, 0.25, 0.25),
        vec3(0.25, 0.65, 0.95),
        vec3(0.35, 0.85, 0.45),
        vec3(0.95, 0.75, 0.20),
        vec3(0.75, 0.35, 0.90),
        vec3(0.20, 0.85, 0.80),
        vec3(0.95, 0.45, 0.20),
        vec3(0.70, 0.85, 0.30)
    );

    return colors[chartIndex % 8];
}

void main()
{
    if (renderMode == 2)
    {
        FragColor = vec4(chartColor(max(chartIndex, 0)), 1.0);
        return;
    }

    if (renderMode == 0)
    {
        FragColor = texture(Texture, texCoord);
        return;
    }

    vec3 shades[6] = vec3[](
        vec3(0.12),
        vec3(0.23),
        vec3(0.35),
        vec3(0.50),
        vec3(0.68),
        vec3(0.82)
    );

    int shadeIndex = int(mod(float(gl_PrimitiveID), 6.0));
    FragColor = vec4(shades[shadeIndex], 1.0);
} 