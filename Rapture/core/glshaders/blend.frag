#version 150 core
in vec3 Color;
in vec2 Texcoord;
out vec4 outColor;

uniform sampler2D baseTexture;
uniform sampler2D blendedTexture;

void main() {
	vec4 baseTex = texture(baseTexture);
	vec4 blendedTex = texture(blendedTexture);
	
	// blend it
	vec4 color = baseTex * 1.0 + blendedTex * 2.0 / 3.0;
	
	outColor = color;
}