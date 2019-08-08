#version 440 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture1;

void main()
{
//    FragColor = vec4(0.0f, 1.0f, 0.0f, 0.0f);
	FragColor = texture(texture1, TexCoords);
}