#version 330 core
layout (location = 0) in vec4 aPos;
layout (location = 1) in vec4 aColor;
out vec4 Color;
out vec2 TexCoord;

void main()
{
    gl_Position = vec4(aPos.xy, 0.0, 1.0);
    Color = vec4(aPos.zw, aColor.xy);
	TexCoord = aColor.zw;
}
