#version 420

layout (location = 0) in vec4 pos;

uniform mat4 modelViewProjectionMatrix;

uniform vec2 texLookup[6];

out vec3 color;
out vec2 texCoord;


void main(){
    gl_Position = modelViewProjectionMatrix * vec4(pos.xyz, 1.0);

    texCoord = texLookup[uint(pos.w)];
}