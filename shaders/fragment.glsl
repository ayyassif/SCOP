#version 330 core

out vec4 FragColor;

in vec2 texCoord;

uniform sampler2D Texture;
uniform bool useTexture;

void main()
{
    if (useTexture)
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