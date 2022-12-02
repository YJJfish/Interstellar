#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D diffuseMap;
uniform vec3 viewPos;

//点光源变量类型
struct PointLight {
    //辐射参数
    float constant;
    float linear;
    float quadratic;
    //颜色参数
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
    //漫反射颜色
    vec3 DiffuseColor = texture(diffuseMap, TexCoord).rgb;
    //光滑度
    float Shininess = 16.0;
    //镜面光强度
    float SpecularStrength = 0.1;
    //计算颜色
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 Color = vec3(0., 0., 0.);
    for(int i = 0; i < LightNum; i++){
        vec3 lightDir = normalize(lightPos[i] - FragPos);
        //漫反射着色
        float diff = max(dot(norm, lightDir), 0.0);
        //镜面光着色
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), Shininess);
        //合并结果
        vec3 ambient  = PointLights[i].ambient  * DiffuseColor;
        vec3 diffuse  = PointLights[i].diffuse  * diff * DiffuseColor;
        vec3 specular = PointLights[i].specular * spec * vec3(SpecularStrength);
        //计算衰减
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
    //最后, 恢复alpha通道
    FragColor = vec4(Color, texture(diffuseMap, TexCoord).a);
}