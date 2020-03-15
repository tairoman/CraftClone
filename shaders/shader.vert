#version 420

#define NUM_BLOCK_TYPES 4

layout (location = 0) in vec2 texCoord;
layout (location = 1) in vec3 pos;


uniform mat4 modelViewProjectionMatrix;

//uniform vec2 texLookup[NUM_BLOCK_TYPES*6];

//out vec3 color;
out vec2 texCoordOut;

void main(){
    gl_Position = modelViewProjectionMatrix * vec4(pos.xyz, 1.0);

    texCoordOut = texCoord;//texLookup[uint(pos.w)];
}