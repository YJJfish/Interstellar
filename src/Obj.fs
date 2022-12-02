#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D diffuseMap;
uniform vec3 viewPos;

//���Դ��������
struct PointLight {
    //�������
    float constant;
    float linear;
    float quadratic;
    //��ɫ����
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform PointLight PointLights[9];
uniform int LightNum;
uniform vec3 lightPos[9];
uniform bool UseAttenuation;


void main() {
    vec3 norm = normalize(Normal);
    //��������ɫ
    vec3 DiffuseColor = texture(diffuseMap, TexCoord).rgb;
    //�⻬��
    float Shininess = 16.0;
    //�����ǿ��
    float SpecularStrength = 0.1;
    //������ɫ
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 Color = vec3(0., 0., 0.);
    for(int i = 0; i < LightNum; i++){
        vec3 lightDir = normalize(lightPos[i] - FragPos);
        //��������ɫ
        float diff = max(dot(norm, lightDir), 0.0);
        //�������ɫ
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), Shininess);
        //�ϲ����
        vec3 ambient  = PointLights[i].ambient  * DiffuseColor;
        vec3 diffuse  = PointLights[i].diffuse  * diff * DiffuseColor;
        vec3 specular = PointLights[i].specular * spec * vec3(SpecularStrength);
        //����˥��
        if (UseAttenuation){
            float distance    = length(lightPos[i] - FragPos);
            float attenuation = 1.0 / (PointLights[i].constant + PointLights[i].linear * distance + 
                                PointLights[i].quadratic * (distance * distance));
            ambient  *= attenuation;
            diffuse  *= attenuation;
            specular *= attenuation;
        }
        Color += (ambient + diffuse + specular);
    }
    //���, �ָ�alphaͨ��
    FragColor = vec4(Color, texture(diffuseMap, TexCoord).a);
}