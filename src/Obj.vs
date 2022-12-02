#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoords;
layout (location = 2) in vec3 normal;

out vec2 TexCoord;
out vec3 FragPos;
out vec3 Normal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main(){
    FragPos = vec3(model * vec4(position, 1.0f));
    TexCoord = texCoords;
    Normal = mat3(transpose(inverse(model))) * normal;

    gl_Position = projection * view * model * vec4(position, 1.0f);
}