#version 420

precision highp float;

in vec3 color;
in vec2 texCoord;

layout(binding = 0) uniform sampler2D colortexture;

layout(location = 0) out vec4 fragColor;

void main() {
    fragColor = texture(colortexture, texCoord);;//vec3(1.0f, 0.0f, 0.0f);
}