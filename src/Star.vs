#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in vec3 tangent;
//副切线B=T×N, 不需要用户计算

//在顶点着色器中把世界坐标系的光源、相机转换到切线空间坐标系中，效率更高
out VS_OUT {
    vec2 TexCoords;
    vec2 NormCoords;
    vec2 SpecCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform vec3 viewPos;
uniform mat4 model;

uniform float TextShift;


void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f);
    vs_out.TexCoords = texCoords;
    vs_out.NormCoords = vec2(texCoords.x - TextShift, texCoords.y);
    vs_out.SpecCoords = vec2(texCoords.x + TextShift, texCoords.y);
    
    mat3 normalMatrix = transpose(inverse(mat3(model)));//正交矩阵的置换矩阵与它的逆矩阵相等
    vec3 T = normalize(normalMatrix * tangent);
    vec3 N = normalize(normalMatrix * normal);
    vec3 B = cross(T, N);
    
    mat3 TBN = transpose(mat3(T, B, N));//正交矩阵的置换矩阵与它的逆矩阵相等
    vec4 LightPos = model * vec4(0.0f, 0.0f, 0.0f, 1.0f);
    vs_out.TangentLightPos = TBN * LightPos.xyz / LightPos.w;
    vs_out.TangentViewPos  = TBN * viewPos;
    vs_out.TangentFragPos  = TBN * vec3(model * vec4(position, 1.0));
}