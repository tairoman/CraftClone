#version 420

layout (location = 0) in vec4 pos;

uniform mat4 modelViewProjectionMatrix;

out vec3 color;


void main(){
    gl_Position = modelViewProjectionMatrix * vec4(pos.xyz, 1.0);

    if (pos.w == 1){
        color = vec3(1.0f, 0.0f, 0.0f);
    } else if (pos.w == 2){
        color = vec3(0.0f, 1.0f, 0.0f);
    } else {
        color = vec3(0.0f, 0.0f, 1.0f);
    }
    //color = vec3(1.0f, 0.0f, 0.0f);
}