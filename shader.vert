#version 420

layout (location = 0) in vec3 pos;


void main(){
    gl_Position = vec4(pos.xyz, 1.0);
}