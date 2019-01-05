#version 420

layout (location = 0) in vec3 pos;

uniform mat4 modelViewProjectionMatrix;

void main(){
    gl_Position = modelViewProjectionMatrix * vec4(pos.xyz, 1.0);
}