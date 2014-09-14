#version 150 core

in vec3 Color;
in vec2 Texcoord;
out vec4 outColor;

uniform sampler2D sceneSampler;

void main() {
	outColor = texture(sceneSampler);
}