#version 420

precision highp float;

in vec3 color;

layout(location = 0) out vec4 fragColor;

void main() {
    fragColor.rgb = color;//vec3(1.0f, 0.0f, 0.0f);
}