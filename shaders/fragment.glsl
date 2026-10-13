#version 330 core

out vec4 FragColor;

in vec2 texCoord;
flat in int chartIndex;

uniform sampler2D Texture;
uniform int renderMode;

const int MODE_COLOR = 0;
const int MODE_TEXTURE = 1;
const int MODE_GREY = 2;
const int MODE_CHART = 3;

vec3 colorPicker(int index)
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

    return colors[index % 8];
}

vec3 grayPicker()
{
    const vec3 shades[6] = vec3[](
        vec3(0.12),
        vec3(0.23),
        vec3(0.35),
        vec3(0.50),
        vec3(0.68),
        vec3(0.82)
    );

    return shades[gl_PrimitiveID % 6];
}

void main()
{
    switch (renderMode)
    {
        case MODE_TEXTURE:
            FragColor = texture(Texture, texCoord);
            break;
        case MODE_GREY:
            FragColor = vec4(grayPicker(), 1.0);
            break;
        case MODE_CHART:
            FragColor = vec4(colorPicker(max(chartIndex, 0)), 1.0);
            break;
        default: //MODE_COLOR
            FragColor = vec4(colorPicker(gl_PrimitiveID), 1.0);
            break;
    }
} 