//#version 440 core
//
//layout (location = 0) in vec3 aPos;
//
//void main(){
//	gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
//}


#version 440 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main(){
	//gl_Position = vec4(aPos.x, aPos.y, 1.0, 1.0);
    TexCoords = aTexCoords;    
	gl_Position = projection * view * model * vec4(aPos, 1.0f);
}