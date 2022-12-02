#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec2 TexCoords;
    vec2 NormCoords;
    vec2 SpecCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} fs_in;

uniform sampler2D diffuseMap;
uniform sampler2D specularMap;
uniform sampler2D normalMap;


void main(){           
    //����ͼ�л�÷�����
    vec3 Normal = vec3(0.,0.,-texture(normalMap, fs_in.NormCoords).b);
    //��������ɫ
    vec3 DiffuseColor = texture(diffuseMap, fs_in.TexCoords).rgb;
    //�⻬��
    float Shininess = 8.0 * texture(specularMap, fs_in.SpecCoords).g;
    //�����ǿ��
    float SpecularStrength = texture(specularMap, fs_in.SpecCoords).g * 1.2;
    //������ɫ
    vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    //��������ɫ
    float diff = max(dot(Normal, lightDir), 0.0);
    //�������ɫ
    vec3 reflectDir = reflect(-lightDir, Normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 0.5);
    // �ϲ����
    vec3 diffuse  = 10.0 * diff * DiffuseColor;
    vec3 specular = spec * vec3(SpecularStrength);
    vec3 Color = diffuse + specular;
    //���, �ָ�alphaͨ��
    FragColor = vec4(Color, texture(diffuseMap, fs_in.TexCoords).a);
}