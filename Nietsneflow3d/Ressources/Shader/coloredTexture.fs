#version 330 core
in vec2 TexCoord;
in vec4 Color;
uniform sampler2D ourTexture;
out vec4 FragColor;

void main()
{
    FragColor = mix(texture(ourTexture, TexCoord), Color, Color.w);
}
