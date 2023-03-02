#version 330 core
in vec4 Color;
in vec2 TexCoord;

uniform sampler2D ourTexture;
out vec4 FragColor;

void main()
{
	FragColor = texture(ourTexture, TexCoord) * Color;
}
