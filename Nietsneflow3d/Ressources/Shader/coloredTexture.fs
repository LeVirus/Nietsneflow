#version 330 core
in vec4 Color;
uniform sampler2D ourTexture;
out vec4 FragColor;

void main()
{
		FragColor = Color;
}
