#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec2 TexCoords;
    vec3 TangentLightPos[9];
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} fs_in;

uniform sampler2D diffuseMap;
uniform sampler2D specularMap;
uniform sampler2D normalMap;

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
uniform bool UseAttenuation;



void main(){
    vec4 NormMap = texture(normalMap, fs_in.TexCoords).rgba;
    vec4 SpecMap = texture(specularMap, fs_in.TexCoords).rgba;
    vec4 DiffMap = texture(diffuseMap, fs_in.TexCoords).rgba;
    //从贴图中获得法向量
    vec3 Normal = vec3(NormMap.ra * 2.0 - 1.0, 0.0);
    Normal.z = sqrt(1.0 - Normal.x * Normal.x - Normal.y * Normal.y);
    //vec3 Normal = vec3(0.0, 0.0, 1.0);
    //光滑度
    float Shininess = 16.0 * SpecMap.a;
    //镜面光强度
    float SpecularStrength = SpecMap.g * 0.5;
    //计算颜色
    vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    vec3 Color = vec3(0., 0., 0.);
    for(int i = 0; i < LightNum; i++){
        vec3 lightDir = normalize(fs_in.TangentLightPos[i] - fs_in.TangentFragPos);
        //漫反射着色
        float diff = max(dot(Normal, lightDir), 0.0);
        //镜面光着色
        vec3 reflectDir = reflect(-lightDir, Normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), Shininess);
        // 合并结果
        vec3 ambient  = PointLights[i].ambient  * DiffMap.rgb;
        vec3 diffuse  = PointLights[i].diffuse  * diff * DiffMap.rgb;
        vec3 specular = PointLights[i].specular * spec * vec3(SpecularStrength);
        //计算衰减
        if (UseAttenuation){
            float distance    = length(fs_in.TangentLightPos[i] - fs_in.TangentFragPos);
            float attenuation = 1.0 / (PointLights[i].constant + PointLights[i].linear * distance + 
                                PointLights[i].quadratic * (distance * distance));
            ambient  *= attenuation;
            diffuse  *= attenuation;
            specular *= attenuation;
        }
        Color += max((1.0 - NormMap.b) * (ambient + diffuse + specular), 0.0) + (NormMap.b) * DiffMap.rgb;
        //Color += (ambient + diffuse + specular);
    }
    //最后, 恢复alpha通道
    FragColor = vec4(Color, DiffMap.a);
}