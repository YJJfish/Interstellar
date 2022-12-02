#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D diffuseMap;
uniform float Alpha;
uniform vec3 Filter;

void main(){
	FragColor = texture(diffuseMap, TexCoord);
	FragColor.rgb *= Filter;
	FragColor.a *= Alpha;
}