#version 330 core
in vec2 TexCoord;
in vec4 Color;
uniform sampler2D ourTexture;
out vec4 FragColor;

void main()
{
    FragColor = texture(ourTexture, TexCoord) * vec4(Color.xyz, 0.6);
}
