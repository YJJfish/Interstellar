#pragma once
#include <random>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Texture.hpp"
#include "ObjLoader.hpp"

namespace CelestialBodyConstant{
    static float Pi = glm::pi<float>();
    static glm::mat4 E = glm::identity<glm::mat4>();
    const int Xsegment = 64;
    const int Ysegment = 64;
    const float CloudMaxLife = 1500.0;
    const int CloudNum = 1024;
    const float FireMaxLife = 1000.0;
    const int FireNum = 2048;
    const float FlareMaxLife = 4000.0f;
    const float PulsarFlareMaxLife = 200.0;
    const int FlareNum = 14;
    const float PulsarMaxLife = 200.0;
    const int PulsarNum = 800;
    const float LightningMaxLife = 200.0;
    const int LightningNum = 6;
    //着色器
    Shader* PlanetShader = NULL;
    Shader* StarShader = NULL;
    Shader* BackgroundShader = NULL;
    //正态分布随机数生成器
    std::normal_distribution<double> Distribution(0.0, 1.0);
    std::default_random_engine Generator;
}

using namespace CelestialBodyConstant;

//天体对象
class CelestialBody {
public:
    //构造函数
    CelestialBody(glm::vec3 Position, GLfloat Radius, glm::vec3 Axis, const CelestialBody* RevoCenter) :
        StartPosition(Position), RevoPosition(Position), Radius(Radius),
        Axis(glm::normalize(Axis)), Rotate2Axis((this->Axis == glm::vec3(0., 1., 0.)) ? E : glm::rotate(E, glm::acos(glm::dot(glm::vec3(0., 1., 0.), this->Axis)), glm::cross(glm::vec3(0., 1., 0.), this->Axis))),
        RevoCenter(RevoCenter) {}
    //析构函数
    ~CelestialBody(void){}
    //添加天体, 被添加到数组的天体可以在DisplayAll函数中被画出，而不需要单独调用该天体的Display方法
    static void AddCelestialBody(CelestialBody* Obj) {
        CelestialBody::Body[NumBody++] = Obj;
    }
    //初始化
    static void Init(void) {
        //设置背景/粒子着色器
        BackgroundShader = new Shader("Background.vs", "Background.fs");
        BackgroundShader->use();
        BackgroundShader->setInt("diffuseMap", 0);
        //设置行星着色器的光源
        PlanetShader = new Shader("Planet.vs", "Planet.fs");
        PlanetShader->use();
        PlanetShader->setBool("UseAttenuation", true);
        PlanetShader->setInt("LightNum", 0);
        PlanetShader->setInt("diffuseMap", 0);
        PlanetShader->setInt("normalMap", 1);
        PlanetShader->setInt("specularMap", 2);
        //设置恒星着色器
        StarShader = new Shader("Star.vs", "Star.fs");
        StarShader->use();
        StarShader->setInt("diffuseMap", 0);
        StarShader->setInt("normalMap", 1);
        StarShader->setInt("specularMap", 2);
        //初始化单位球上的点
        InitSphere();
        //初始化粒子的四个点
        InitParticle();
    }
    static void InitSphere(void) {
        //初始化单位球上的点
        float SphereVertices[(Xsegment + 1) * (Ysegment + 1) * 11];
        int SphereIndices[Xsegment * Ysegment * 6];
        for (int y = 0; y <= Ysegment; y++) {
            GLfloat Pitch = -Pi / 2 + Pi * y / Ysegment;
            for (int x = 0; x <= Xsegment; x++) {
                GLfloat Yaw = 2 * Pi * x / Xsegment;
                SphereVertices[(y * (Xsegment + 1) + x) * 11 + 0] = cos(Pitch) * cos(Yaw);
                SphereVertices[(y * (Xsegment + 1) + x) * 11 + 1] = sin(Pitch);
                SphereVertices[(y * (Xsegment + 1) + x) * 11 + 2] = cos(Pitch) * -sin(Yaw);
                SphereVertices[(y * (Xsegment + 1) + x) * 11 + 3] = cos(Pitch) * cos(Yaw);
                SphereVertices[(y * (Xsegment + 1) + x) * 11 + 4] = sin(Pitch);
                SphereVertices[(y * (Xsegment + 1) + x) * 11 + 5] = cos(Pitch) * -sin(Yaw);
                SphereVertices[(y * (Xsegment + 1) + x) * 11 + 6] = (GLfloat)x / Xsegment;
                SphereVertices[(y * (Xsegment + 1) + x) * 11 + 7] = (GLfloat)-y / Ysegment;
                SphereVertices[(y * (Xsegment + 1) + x) * 11 + 8] = -sin(Yaw);
                SphereVertices[(y * (Xsegment + 1) + x) * 11 + 9] = 0.0;
                SphereVertices[(y * (Xsegment + 1) + x) * 11 + 10] = -cos(Yaw);
            }
        }
        //初始化单位球上的点索引
        for (int y = 0; y < Ysegment; y++) {
            for (int x = 0; x < Xsegment; x++) {
                SphereIndices[(y * Xsegment + x) * 6 + 0] = y * (Xsegment + 1) + x;
                SphereIndices[(y * Xsegment + x) * 6 + 1] = (y + 1) * (Xsegment + 1) + x;
                SphereIndices[(y * Xsegment + x) * 6 + 2] = y * (Xsegment + 1) + x + 1;
                SphereIndices[(y * Xsegment + x) * 6 + 3] = (y + 1) * (Xsegment + 1) + x;
                SphereIndices[(y * Xsegment + x) * 6 + 4] = y * (Xsegment + 1) + x + 1;
                SphereIndices[(y * Xsegment + x) * 6 + 5] = (y + 1) * (Xsegment + 1) + x + 1;
            }
        }
        //分配缓冲池
        //加载缓存
        glGenVertexArrays(1, &SphereVAO);
        glGenBuffers(1, &SphereVBO);
        glGenBuffers(1, &SphereEBO);
        glBindVertexArray(SphereVAO);
        glBindBuffer(GL_ARRAY_BUFFER, SphereVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * (Xsegment + 1) * (Ysegment + 1) * 11, SphereVertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, SphereEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * Xsegment * Ysegment * 6, SphereIndices, GL_STATIC_DRAW);
        //设置点的属性
        //坐标
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        //法向量
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        //材质坐标
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
        //切线向量
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(8 * sizeof(float)));
        glEnableVertexAttribArray(3);
    }
    //画单位球
    static void DrawSphere(void) {
        glBindVertexArray(SphereVAO);
        glDrawElements(GL_TRIANGLES, Xsegment * Ysegment * 6, GL_UNSIGNED_INT, 0);
    }
    static void InitParticle(void) {
        //初始化矩形的点
        float SquareVertices[4 * 5];
        SquareVertices[0] = -1.; SquareVertices[1] = -1.; SquareVertices[2] = 0.; SquareVertices[3] = 0.; SquareVertices[4] = 0.;
        SquareVertices[5] = 1.; SquareVertices[6] = -1.; SquareVertices[7] = 0.; SquareVertices[8] = 1.; SquareVertices[9] = 0.;
        SquareVertices[10] = -1.; SquareVertices[11] = 1.; SquareVertices[12] = 0.; SquareVertices[13] = 0.; SquareVertices[14] = 1.;
        SquareVertices[15] = 1.; SquareVertices[16] = 1.; SquareVertices[17] = 0.; SquareVertices[18] = 1.; SquareVertices[19] = 1.;
        glGenVertexArrays(1, &ParticleVAO);
        glGenBuffers(1, &ParticleVBO);
        glBindVertexArray(ParticleVAO);
        glBindBuffer(GL_ARRAY_BUFFER, ParticleVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * 5, SquareVertices, GL_STATIC_DRAW);
        //设置点的属性
        //坐标
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        //材质坐标
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }
    //画粒子
    static void DrawParticle(void) {
        glBindVertexArray(ParticleVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
    //画天体
    virtual void Display(const glm::mat4& projection, const glm::mat4& view, const glm::vec3 viewPos) = 0;
    static void DisplayAll(const glm::mat4& projection, const glm::mat4& view, const glm::vec3 viewPos) {
        //画所有天体
        for (int i = 0; i < NumBody; i++)
            Body[i]->Display(projection, view, viewPos);
    }
    //进行旋转
    virtual void Rotate(double TimeInterval) = 0;
    static void RotateAll(double TimeInterval) {
        for (int i = 0; i < NumBody; i++)
            Body[i]->Rotate(TimeInterval);
    }
    //设置中心点
    void SetStartPosition(glm::vec3 Position) { this->StartPosition = Position;}
    const glm::vec3& GetPosition(void) const { return this->RevoPosition; }
    const glm::vec3& GetStartPosition(void) const { return this->StartPosition; }
    //设置大小
    void SetRadius(GLfloat Radius) { this->Radius = Radius; }
    const GLfloat& GetRadius(void) const { return this->Radius; }
    //设置旋转轴
    void SetAxis(glm::vec3 Axis) {
        this->Axis = glm::normalize(Axis);
        Rotate2Axis = (this->Axis == glm::vec3(0., 1., 0.)) ? E : glm::rotate(E, glm::acos(glm::dot(glm::vec3(0., 1., 0.), this->Axis)), glm::cross(glm::vec3(0., 1., 0.), this->Axis));
    }
    const glm::vec3& GetAxis(void) const { return this->Axis; }
protected:
    GLfloat Radius;
    glm::vec3 StartPosition;
    glm::vec3 RevoPosition;
    glm::vec3 Axis;
    glm::mat4 Rotate2Axis;
    const CelestialBody* RevoCenter;
    static int NumStars;
private:
    //单位球上的缓存编号
    //点的格式: 世界坐标(3), 法向量(3), 材质坐标(2), 切向量(3)
    static GLuint SphereVAO, SphereVBO, SphereEBO;
    //粒子的缓存编号
    //点的格式: 世界坐标(3), 材质坐标(2)
    static GLuint ParticleVAO, ParticleVBO;
    //保存所有天体的数组
    static int NumBody;
    static CelestialBody* Body[1024];
};
//初始化静态变量
int CelestialBody::NumBody = 0;
CelestialBody* CelestialBody::Body[] = {};
GLuint CelestialBody::SphereVAO = 0;
GLuint CelestialBody::SphereVBO = 0;
GLuint CelestialBody::SphereEBO = 0;
GLuint CelestialBody::ParticleVAO = 0;
GLuint CelestialBody::ParticleVBO = 0;
int CelestialBody::NumStars = 0;







//行星
class Planet :public CelestialBody {
    friend class RingWorld;
public:
    static void Init(void) {
        //初始化星环的点
        InitRing();
        //加载大气层贴图
        TextureAtmosphere[0] = new Texture(GL_TEXTURE_2D, "Texture/PlanetBackgroung/blue.png");
        TextureAtmosphere[1] = new Texture(GL_TEXTURE_2D, "Texture/PlanetBackgroung/brown.png");
        TextureAtmosphere[2] = new Texture(GL_TEXTURE_2D, "Texture/PlanetBackgroung/gray.png");
        TextureAtmosphere[3] = new Texture(GL_TEXTURE_2D, "Texture/PlanetBackgroung/green.png");
        TextureAtmosphere[4] = new Texture(GL_TEXTURE_2D, "Texture/PlanetBackgroung/purple.png");
        TextureAtmosphere[5] = new Texture(GL_TEXTURE_2D, "Texture/PlanetBackgroung/red.png");
        TextureAtmosphere[6] = new Texture(GL_TEXTURE_2D, "Texture/PlanetBackgroung/yellow.png");
        //行星贴图使用Lazy加载方式，当行星被生成时才加载贴图
    }
    static void InitRing(void) {
        float RingVertices[(Xsegment + 1) * 2 * 5];
        for (int x = 0; x <= Xsegment; x++) {
            //内径点
            //世界坐标
            RingVertices[x * 2 * 5 + 0] = 1.5 * cos(2 * Pi * x / Xsegment);
            RingVertices[x * 2 * 5 + 1] = 0.;
            RingVertices[x * 2 * 5 + 2] = 1.5 * sin(2 * Pi * x / Xsegment);
            //材质坐标
            RingVertices[x * 2 * 5 + 3] = (float)x / Xsegment;
            RingVertices[x * 2 * 5 + 4] = 0.0;
            //外径点
            //世界坐标
            RingVertices[x * 2 * 5 + 5] = 2.5 * cos(2 * Pi * x / Xsegment);
            RingVertices[x * 2 * 5 + 6] = 0.;
            RingVertices[x * 2 * 5 + 7] = 2.5 * sin(2 * Pi * x / Xsegment);
            //材质坐标
            RingVertices[x * 2 * 5 + 8] = (float)x / Xsegment;
            RingVertices[x * 2 * 5 + 9] = -1.0;
        }
        glGenVertexArrays(1, &RingVAO);
        glGenBuffers(1, &RingVBO);
        glBindVertexArray(RingVAO);
        glBindBuffer(GL_ARRAY_BUFFER, RingVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * (Xsegment + 1) * 2 * 5, RingVertices, GL_STATIC_DRAW);
        //设置点的属性
        //坐标
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        //材质坐标
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }
    //行星类别
    enum TYPE {
        AI,         //机械星球, 1种贴图
        ALPINE,     //山脉星球, 2种贴图
        ARCTIC,     //极地星球, 3种贴图
        ARID,       //干旱星球, 3种贴图
        BARREN,     //荒芜星球, 4种贴图
        CITY,       //都市星球, 9种贴图
        COLDBARREN, //寒冷荒芜星球, 4种贴图
        CONTINENTAL,//大陆星球, 5种贴图, 最后1种是地球贴图
        DESERT,     //沙漠星球, 3种贴图
        FROZEN,     //冰冻星球, 3种贴图
        GASGIANT,   //气态巨行星, 6种贴图
        INFESTED,   //感染星球, 1种贴图
        MOLTEN,     //熔岩星球, 3种贴图
        OCEAN,      //海洋星球, 4种贴图
        RELIC,      //遗迹星球, 1种贴图
        SAVANNAH,   //草原星球, 2种贴图
        TOXIC,      //剧毒星球, 4种贴图
        TROPICAL,   //热带星球, 3种贴图
        TUNDRA,     //苔原星球, 3种贴图
        CLOUDS,     //云雾, 4种贴图
        RING,       //星环, 1种贴图
    };
    //构造函数
    Planet(glm::vec3 Position, GLfloat Radius, glm::vec3 Axis, TYPE PlanetType, const CelestialBody* RevoCenter, int TextureType = 0, int CloudTextureType = 0, float PlanetRotateSpeed = 1.0, float CloudsRotateSpeed = 1.5, float RevoSpeed = 0.5, bool HasRing = false)
        :CelestialBody(Position, Radius, Axis, RevoCenter), PlanetType(PlanetType), TextureType(TextureType),
        HasRing(HasRing), CloudTextureType(CloudTextureType),
        PlanetTheta(0.0), CloudsTheta(0.0), PlanetRotateSpeed(PlanetRotateSpeed),
        CloudsRotateSpeed(CloudsRotateSpeed),
        RevoSpeed(RevoSpeed), RevoTheta(0.0) {
        //如果该类贴图还没加载, 则进行初始化
        //地表贴图
        if (TextureDiffuse[GetTypeID(PlanetType, TextureType)] == NULL) {
            string FileName = GetTextureName(PlanetType, TextureType);
            TextureDiffuse[GetTypeID(PlanetType, TextureType)] = new Texture(GL_TEXTURE_2D, "Texture/Planets/" + FileName + "diffuse.dds");
            TextureNormal[GetTypeID(PlanetType, TextureType)] = new Texture(GL_TEXTURE_2D, "Texture/Planets/" + FileName + "normal.dds");
            TextureSpecular[GetTypeID(PlanetType, TextureType)] = new Texture(GL_TEXTURE_2D, "Texture/Planets/" + FileName + "specular.dds");
        }
        //云雾贴图
        if (TextureDiffuse[GetTypeID(CLOUDS, CloudTextureType)] == NULL) {
            string FileName = GetTextureName(CLOUDS, CloudTextureType);
            TextureDiffuse[GetTypeID(CLOUDS, CloudTextureType)] = new Texture(GL_TEXTURE_2D, "Texture/Planets/" + FileName + "diffuse.dds");
            TextureNormal[GetTypeID(CLOUDS, CloudTextureType)] = new Texture(GL_TEXTURE_2D, "Texture/Planets/" + FileName + "normal.dds");
            TextureSpecular[GetTypeID(CLOUDS, CloudTextureType)] = new Texture(GL_TEXTURE_2D, "Texture/Planets/" + FileName + "specular.dds");
        }
        //星环贴图
        if (HasRing) {
            if (TextureDiffuse[GetTypeID(RING, 0)] == NULL) {
                string FileName = GetTextureName(RING, 0);
                TextureDiffuse[GetTypeID(RING, 0)] = new Texture(GL_TEXTURE_2D, "Texture/Planets/" + FileName + "diffuse.dds");
                TextureNormal[GetTypeID(RING, 0)] = new Texture(GL_TEXTURE_2D, "Texture/Planets/" + FileName + "normal.dds");
                TextureSpecular[GetTypeID(RING, 0)] = new Texture(GL_TEXTURE_2D, "Texture/Planets/" + FileName + "specular.dds");
            }
        }
    }
    //进行旋转
    void Rotate(double TimeInterval) {
        PlanetTheta += TimeInterval * PlanetRotateSpeed / 50.0;
        CloudsTheta += TimeInterval * CloudsRotateSpeed / 50.0;
        PlanetTheta -= (int)(PlanetTheta / 2 / Pi) * 2 * Pi;
        CloudsTheta -= (int)(CloudsTheta / 2 / Pi) * 2 * Pi;
        RevoTheta += TimeInterval * RevoSpeed / 50.0;
        RevoTheta -= (int)(RevoTheta / 2 / Pi) * 2 * Pi;
        if (RevoCenter == NULL) {//绕原点旋转
            RevoPosition = glm::rotate(E, RevoTheta, glm::vec3(0., 1., 0.)) * glm::vec4(StartPosition, 1.0F);
        }
        else {
            RevoPosition = glm::rotate(E, RevoTheta, glm::vec3(0., 1., 0.)) * glm::vec4(StartPosition - RevoCenter->GetStartPosition(), 1.0F) + glm::vec4(RevoCenter->GetPosition(), 1.0F);
        }
    }
    //画图
    void Display(const glm::mat4& projection, const glm::mat4& view, const glm::vec3 viewPos) {
        //初始化
        PlanetShader->use();
        PlanetShader->setMat4("projection", projection);
        PlanetShader->setMat4("view", view);
        PlanetShader->setVec3("viewPos", viewPos);
        glm::mat4 Rotate2View;
        glm::vec3 NormViewPos = glm::normalize(viewPos - RevoPosition);
        glm::vec3 NormViewXZ = viewPos - RevoPosition; NormViewXZ.y = 0; NormViewXZ = glm::normalize(NormViewXZ);
        glm::mat4 Rotate2ViewY = glm::rotate(E, -glm::asin(NormViewPos.y), glm::vec3(1., 0., 0.));
        glm::mat4 Rotate2ViewXZ = (NormViewXZ.x > 0) ? glm::rotate(E, glm::acos(NormViewXZ.z), glm::vec3(0., 1., 0.)) : glm::rotate(E, -glm::acos(NormViewXZ.z), glm::vec3(0., 1., 0.));
        Rotate2View = Rotate2ViewXZ * Rotate2ViewY;
        float Distance = glm::length(viewPos - RevoPosition);
        //行星表面
        glDepthMask(GL_TRUE);
        glm::mat4 PlanetModel = Rotate2Axis * glm::rotate(glm::scale(E, glm::vec3(Radius)), PlanetTheta, glm::vec3(0., 1., 0.));
        PlanetModel[3].x = RevoPosition.x; PlanetModel[3].y = RevoPosition.y; PlanetModel[3].z = RevoPosition.z;
        TextureDiffuse[GetTypeID(PlanetType, TextureType)]->Bind(GL_TEXTURE0);
        TextureNormal[GetTypeID(PlanetType, TextureType)]->Bind(GL_TEXTURE1);
        TextureSpecular[GetTypeID(PlanetType, TextureType)]->Bind(GL_TEXTURE2);
        PlanetShader->setMat4("model", PlanetModel);
        CelestialBody::DrawSphere();
        //大气层
        glDepthMask(GL_FALSE);
        BackgroundShader->use();
        BackgroundShader->setMat4("projection", projection);
        BackgroundShader->setMat4("view", view);
        BackgroundShader->setFloat("Alpha", 1.0);
        glm::mat4 AtmosphereModel = Rotate2View * glm::scale(E, glm::vec3(Distance * Radius / sqrt(Distance * Distance - Radius * Radius) * 1.1));
        AtmosphereModel[3].x = RevoPosition.x; AtmosphereModel[3].y = RevoPosition.y; AtmosphereModel[3].z = RevoPosition.z;
        BackgroundShader->setMat4("model", AtmosphereModel);
        BackgroundShader->setVec3("Filter", 1., 1., 1.);
        TextureAtmosphere[GetAtmosphereID(PlanetType, TextureType)]->Bind(GL_TEXTURE0);
        DrawParticle();
        //只有生态圈的行星才有云雾
        if (PlanetType == ALPINE || PlanetType == ARCTIC || PlanetType == ARID || PlanetType == CONTINENTAL || 
            PlanetType == DESERT || PlanetType == OCEAN || PlanetType == RELIC || PlanetType == SAVANNAH || 
            PlanetType == TROPICAL || PlanetType == TUNDRA) {
            glDepthMask(GL_FALSE);
            glm::mat4 CloudsModel = Rotate2Axis * glm::rotate(glm::scale(E, glm::vec3(Radius * 1.01)), CloudsTheta, glm::vec3(0., 1., 0.));
            CloudsModel[3].x = RevoPosition.x; CloudsModel[3].y = RevoPosition.y; CloudsModel[3].z = RevoPosition.z;
            TextureDiffuse[GetTypeID(CLOUDS, CloudTextureType)]->Bind(GL_TEXTURE0);
            TextureNormal[GetTypeID(CLOUDS, CloudTextureType)]->Bind(GL_TEXTURE1);
            TextureSpecular[GetTypeID(CLOUDS, CloudTextureType)]->Bind(GL_TEXTURE2);
            PlanetShader->use();
            PlanetShader->setMat4("model", CloudsModel);
            CelestialBody::DrawSphere();
        }
        //星环
        if (HasRing) {
            glDepthMask(GL_FALSE);
            TextureDiffuse[GetTypeID(RING, 0)]->Bind(GL_TEXTURE0);
            TextureNormal[GetTypeID(RING, 0)]->Bind(GL_TEXTURE1);
            TextureSpecular[GetTypeID(RING, 0)]->Bind(GL_TEXTURE2);
            BackgroundShader->use();
            BackgroundShader->setMat4("model", PlanetModel);
            glBindVertexArray(RingVAO);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, (Xsegment + 1) * 2);
        }
        glDepthMask(GL_TRUE);
    }
protected:
    float RevoSpeed;
    float RevoTheta;
    float PlanetTheta;//已转过的角度
    float CloudsTheta;//已转过的角度
    float PlanetRotateSpeed;//旋转速度
    float CloudsRotateSpeed;//旋转速度
    bool HasRing;
    TYPE PlanetType;//行星类型
    int TextureType;//贴图类型
    int CloudTextureType;//云雾贴图类型
    //根据行星类别和贴图类别得到贴图前缀名
    static string GetTextureName(TYPE PlanetType, int TextureType) {
        switch (GetTypeID(PlanetType, TextureType)) {
        case 00:return "ai_01_";
        case 10:return "alpine_01_";
        case 11:return "alpine_02_";
        case 20:return "arctic_01_";
        case 21:return "arctic_02_";
        case 22:return "arctic_03_";
        case 30:return "arid_01_";
        case 31:return "arid_02_";
        case 32:return "arid_03_";
        case 40:return "barren_01_";
        case 41:return "barren_02_";
        case 42:return "barren_03_";
        case 43:return "barren_04_";
        case 50:return "city_01_";
        case 51:return "city_02_";
        case 52:return "city_03_";
        case 53:return "city_04_";
        case 54:return "city_05_";
        case 55:return "city_06_";
        case 56:return "city_07_";
        case 57:return "city_08_";
        case 58:return "city_09_";
        case 60:return "cold_barren_01_";
        case 61:return "cold_barren_02_";
        case 62:return "cold_barren_03_";
        case 63:return "cold_barren_04_";
        case 70:return "continental_01_";
        case 71:return "continental_02_";
        case 72:return "continental_03_";
        case 73:return "continental_04_";
        case 74:return "continental_05_";
        case 80:return "desert_01_";
        case 81:return "desert_02_";
        case 82:return "desert_03_";
        case 90:return "frozen_01_";
        case 91:return "frozen_02_";
        case 92:return "frozen_03_";
        case 100:return "gas_giant_01_";
        case 101:return "gas_giant_02_";
        case 102:return "gas_giant_03_";
        case 103:return "gas_giant_04_";
        case 104:return "gas_giant_05_";
        case 105:return "gas_giant_06_";
        case 110:return "infested_01_";
        case 120:return "molten_01_";
        case 121:return "molten_02_";
        case 122:return "molten_03_";
        case 130:return "ocean_01_";
        case 131:return "ocean_02_";
        case 132:return "ocean_03_";
        case 133:return "ocean_04_";
        case 140:return "relic_01_";
        case 150:return "savannah_01_";
        case 151:return "savannah_02_";
        case 160:return "toxic_01_";
        case 161:return "toxic_02_";
        case 162:return "toxic_03_";
        case 163:return "toxic_04_";
        case 170:return "tropical_01_";
        case 171:return "tropical_02_";
        case 172:return "tropical_03_";
        case 180:return "tundra_01_";
        case 181:return "tundra_02_";
        case 182:return "tundra_03_";
        case 190:return "clouds_01_";
        case 191:return "clouds_02_";
        case 192:return "clouds_03_";
        case 193:return "clouds_04_";
        case 200:return "ring_";
        default:throw exception("Invalid planet type");
        }
    }
    //根据行星类别和贴图类别得到编号
    static int GetTypeID(TYPE PlanetType, int TextureType) {
        return PlanetType * 10 + TextureType;
    }
    //根据行星类别返回大气层颜色
    static int GetAtmosphereID(TYPE PlanetType, int TextureType) {
        //0: blue
        //1: brown
        //2: gray
        //3: green
        //4: purple
        //5: red
        //6: yellow
        switch (PlanetType) {
        case CONTINENTAL:case OCEAN:return 0;
        case BARREN:return 1;
        case ALPINE:case AI:case ARCTIC:case CITY:case COLDBARREN:case FROZEN:case TUNDRA:return 2;
        case TROPICAL:return 3;
        case INFESTED:return 4;
        case MOLTEN:return 5;
        case RELIC:case ARID:case DESERT:case SAVANNAH:return 6;
        case GASGIANT:
            switch (TextureType) {
            case 0:case 4:case 5:return 1;
            case 1:case 2:case 3:return 2;
            default:throw exception("Invalid texture type");
            }
        case TOXIC:
            switch (TextureType) {
            case 0:case 1:return 3;
            case 2:return 1;
            case 3:return 6;
            default:throw exception("Invalid texture type");
            }
        default:throw exception("Invalid planet type");
        }
    }
    //星环缓存
    static GLuint RingVAO, RingVBO;
    //纹理
    static Texture* TextureDiffuse[201];
    static Texture* TextureNormal[201];
    static Texture* TextureSpecular[201];
    static Texture* TextureAtmosphere[7];
};
Texture* Planet::TextureDiffuse[] = {};
Texture* Planet::TextureNormal[] = {};
Texture* Planet::TextureSpecular[] = {};
Texture* Planet::TextureAtmosphere[] = {};
GLuint Planet::RingVAO = 0;
GLuint Planet::RingVBO = 0;





//恒星
class Star :public CelestialBody {
public:
    //初始化材质
    static void Init(void) {
        for (int i = 0; i < 4; i++)
            try {
                string FileName = GetTextureName((TYPE)i);
                TextureDiffuse[i] = new Texture(GL_TEXTURE_2D, "Texture/Stars/" + FileName + "diffuse.bmp");
                TextureNormal[i] = new Texture(GL_TEXTURE_2D, "Texture/Stars/" + FileName + "normal.bmp");
                TextureSpecular[i] = new Texture(GL_TEXTURE_2D, "Texture/Stars/" + FileName + "specular.bmp");
            }
            catch (...) {}
        //脉冲星的球面和蓝色恒星一样
        TextureDiffuse[PULSAR] = TextureDiffuse[BLUE];
        TextureNormal[PULSAR] = TextureNormal[BLUE];
        TextureSpecular[PULSAR] = TextureSpecular[BLUE];
        //粒子贴图
        TextureCloud[0] = new Texture(GL_TEXTURE_2D, "Texture/Stars/cloud_01.dds");
        TextureCloud[1] = new Texture(GL_TEXTURE_2D, "Texture/Stars/cloud_02.dds");
        TextureCloud[2] = new Texture(GL_TEXTURE_2D, "Texture/Stars/cloud_03.dds");
        TextureFire[0] = new Texture(GL_TEXTURE_2D, "Texture/Stars/fire_01.dds");
        TextureFire[1] = new Texture(GL_TEXTURE_2D, "Texture/Stars/fire_02.dds");
        TextureFire[2] = new Texture(GL_TEXTURE_2D, "Texture/Stars/fire_03.dds");
        TextureGlow = new Texture(GL_TEXTURE_2D, "Texture/Stars/glow.dds");
        TextureFlare = new Texture(GL_TEXTURE_2D, "Texture/Stars/flare.dds");
        //脉冲星
        TextureLightning = new Texture(GL_TEXTURE_2D, "Texture/Stars/lightning.png");
        PulsarDust = new Texture(GL_TEXTURE_2D, "Texture/Stars/pulsar_dust.dds");
        PulsarDustModel = new ObjLoader("Texture/Stars/pulsar_dust.obj");
    }
    //恒星类别
    enum TYPE {
        ORANGE,
        BLUE,
        PURPLE,
        WHITE,
        PULSAR
    };
    //构造函数
    Star(glm::vec3 Position, GLfloat Radius, glm::vec3 Axis, TYPE StarType, const CelestialBody* RevoCenter, float StarRotateSpeed = 1.0, float RevoSpeed = 0.5)
        :CelestialBody(Position, Radius, Axis, RevoCenter), StarType(StarType),
        StarTheta(0.0), StarRotateSpeed(StarRotateSpeed),
        RevoSpeed(RevoSpeed), RevoTheta(0.0) {
        string Num = "0";
        Num[0] = NumStars + '0';
        PlanetShader->use();
        PlanetShader->setInt("LightNum", NumStars + 1);
        PlanetShader->setVec3("lightPos[" + Num + "]", glm::vec3(0., 0., 0.));
        PlanetShader->setVec3("PointLights[" + Num + "].ambient", 0.3f, 0.3f, 0.3f);
        PlanetShader->setVec3("PointLights[" + Num + "].diffuse", 10.f, 10.f, 10.f);
        PlanetShader->setVec3("PointLights[" + Num + "].specular", 10.f, 10.f, 10.f);
        PlanetShader->setFloat("PointLights[" + Num + "].constant", 1.0);
        PlanetShader->setFloat("PointLights[" + Num + "].linear", 0.00007);
        PlanetShader->setFloat("PointLights[" + Num + "].quadratic", 0.0000004);
        NumStars++;
        //随机生成粒子
        for (int i = 0; i < CloudNum; i++) {
            CloudLife[i] = (CloudMaxLife + 1) * i / CloudNum;
            CloudPos[i] = glm::normalize(glm::vec3(Distribution(Generator), Distribution(Generator), Distribution(Generator)));
        }
        for (int i = 0; i < FireNum; i++) {
            FireLife[i] = (FireMaxLife + 1) * i / FireNum;
            FirePos[i] = glm::normalize(glm::vec3(Distribution(Generator), Distribution(Generator), Distribution(Generator)));
        }
        for (int i = 0; i < FlareNum; i++) {
            FlareLife[i] = ((StarType == PULSAR) ? PulsarFlareMaxLife : FlareMaxLife) * i / FlareNum;
            FlarePos[i] = 2 * Pi * rand() / (RAND_MAX + 1);
        }
        if (StarType == PULSAR) {
            LastPulsar = PulsarNum - 1;
            for (int i = 0; i < PulsarNum; i++) {
                PulsarLife[i] = (PulsarMaxLife) * (i + 1) / PulsarNum;
            }
            for (int i = 0; i < LightningNum; i++) {
                LightningLife[i] = (LightningMaxLife) * (i + 1) / LightningNum;
                LightningPos[i] = glm::normalize(glm::vec3(Distribution(Generator), Distribution(Generator), Distribution(Generator)));
            }
        }
    }
    //进行旋转
    void Rotate(double TimeInterval) {
        //表面旋转
        StarTheta += TimeInterval * StarRotateSpeed / 50.0;
        StarTheta -= (int)(StarTheta / 24 / Pi) * 24 * Pi;
        RevoTheta += TimeInterval * RevoSpeed / 50.0;
        RevoTheta -= (int)(RevoTheta / 2 / Pi) * 2 * Pi;
        if (RevoCenter == NULL) {//绕原点旋转
            RevoPosition = glm::rotate(E, RevoTheta, glm::vec3(0., 1., 0.)) * glm::vec4(StartPosition, 1.0F);
        }
        else {
            RevoPosition = glm::rotate(E, RevoTheta, glm::vec3(0., 1., 0.)) * glm::vec4(StartPosition - RevoCenter->GetStartPosition(), 1.0F) + glm::vec4(RevoCenter->GetPosition(), 1.0F);
        }
        //更新粒子
        float LifeDecrease = TimeInterval * 200;
        for (int i = 0; i < CloudNum; i++)
            if (CloudLife[i] > LifeDecrease) CloudLife[i] -= LifeDecrease;
            else {
                CloudLife[i] += 1500 - LifeDecrease;
                CloudPos[i] = glm::normalize(glm::vec3(Distribution(Generator), Distribution(Generator), Distribution(Generator)));
            }
        for (int i = 0; i < FireNum; i++)
            if (FireLife[i] > LifeDecrease) FireLife[i] -= LifeDecrease;
            else {
                FireLife[i] += 1000 - LifeDecrease;
                FirePos[i] = glm::normalize(glm::vec3(Distribution(Generator), Distribution(Generator), Distribution(Generator)));
            }
        for (int i = 0; i < FlareNum; i++)
            if (FlareLife[i] > LifeDecrease) FlareLife[i] -= LifeDecrease;
            else {
                FlareLife[i] += ((StarType == PULSAR) ? PulsarFlareMaxLife : FlareMaxLife) - LifeDecrease;
                FlarePos[i] = 2 * Pi * rand() / (RAND_MAX + 1);
            }
        if (StarType == PULSAR) {
            int NewLastPulsar = LastPulsar;
            for (int i = (LastPulsar + 1) % PulsarNum; i != LastPulsar; i = (i + 1) % PulsarNum)
                if (PulsarLife[i] > LifeDecrease) {
                    PulsarLife[i] -= LifeDecrease;
                }
                else {
                    NewLastPulsar = i;
                    PulsarLife[i] += PulsarMaxLife - LifeDecrease;
                }
            for (int i = 0; i < LightningNum; i++)
                if (LightningLife[i] > -200) LightningLife[i] -= LifeDecrease;
                else {
                    LightningLife[i] += 2 * LightningMaxLife - LifeDecrease;
                    LightningPos[i] = glm::normalize(glm::vec3(Distribution(Generator), Distribution(Generator), Distribution(Generator)));
                }
            LastPulsar = NewLastPulsar;
        }
    }
    //画图
    void Display(const glm::mat4& projection, const glm::mat4& view, const glm::vec3 viewPos) {
        //初始化
        BackgroundShader->use();
        BackgroundShader->setMat4("projection", projection);
        BackgroundShader->setMat4("view", view);
        StarShader->use();
        StarShader->setMat4("projection", projection);
        StarShader->setMat4("view", view);
        StarShader->setVec3("viewPos", viewPos);
        glm::mat4 Rotate2View;
        glm::vec3 NormViewPos = glm::normalize(viewPos - RevoPosition);
        glm::vec3 NormViewXZ = viewPos - RevoPosition; NormViewXZ.y = 0; NormViewXZ = glm::normalize(NormViewXZ);
        glm::mat4 Rotate2ViewY = glm::rotate(E, -glm::asin(NormViewPos.y), glm::vec3(1., 0., 0.));
        glm::mat4 Rotate2ViewXZ = (NormViewXZ.x > 0) ? glm::rotate(E, glm::acos(NormViewXZ.z), glm::vec3(0., 1., 0.)) : glm::rotate(E, -glm::acos(NormViewXZ.z), glm::vec3(0., 1., 0.));
        Rotate2View = Rotate2ViewXZ * Rotate2ViewY;
        float Distance = glm::length(viewPos - RevoPosition);
        //发光层
        glDepthMask(GL_FALSE);
        BackgroundShader->use();
        BackgroundShader->setFloat("Alpha", 1.0);
        switch (StarType) {
        case PURPLE:BackgroundShader->setVec3("Filter", 0.7, 0.0, 2.0); break;
        case ORANGE:BackgroundShader->setVec3("Filter", 2.0, 0.7, 0.0); break;
        case BLUE:case PULSAR:BackgroundShader->setVec3("Filter", 0.6, 1.0, 3.0); break;
        case WHITE:BackgroundShader->setVec3("Filter", 0.8, 1., 1.); break;
        }
        glm::mat4 AtmosphereModel = Rotate2View * glm::scale(E, glm::vec3(Distance * Radius / sqrt(Distance * Distance - Radius * Radius) * 1.9));
        AtmosphereModel[3].x = RevoPosition.x; AtmosphereModel[3].y = RevoPosition.y; AtmosphereModel[3].z = RevoPosition.z;
        BackgroundShader->setMat4("model", AtmosphereModel);
        TextureGlow->Bind(GL_TEXTURE0);
        DrawParticle();
        //恒星表面
        glDepthMask(GL_TRUE);
        StarShader->use();
        glm::mat4 StarModel = Rotate2Axis * glm::rotate(glm::scale(E, glm::vec3(Radius)), StarTheta, glm::vec3(0., 1., 0.));
        StarModel[3].x = RevoPosition.x; StarModel[3].y = RevoPosition.y; StarModel[3].z = RevoPosition.z;
        TextureDiffuse[StarType]->Bind(GL_TEXTURE0);
        TextureNormal[StarType]->Bind(GL_TEXTURE1);
        TextureSpecular[StarType]->Bind(GL_TEXTURE2);
        StarShader->setMat4("model", StarModel);
        StarShader->setFloat("TextShift", StarTheta / 3.1415926536);
        DrawSphere();
        //云雾粒子
        glDepthMask(GL_FALSE);
        BackgroundShader->use();
        switch (StarType) {
        case PURPLE:BackgroundShader->setVec3("Filter", 1.5, 0.0, 4.0); break;
        case ORANGE:BackgroundShader->setVec3("Filter", 4.0, 0.5, 0.0); break;
        case BLUE:case PULSAR:BackgroundShader->setVec3("Filter", 1.2, 1.8, 3.5); break;
        case WHITE:BackgroundShader->setVec3("Filter", 2., 3., 2.); break;
        }
        for (int i = 0; i < CloudNum; i++) {
            BackgroundShader->setFloat("Alpha", (1500.0 - CloudLife[i]) * CloudLife[i] / 750 / 750 / 3);
            glm::mat4 Model = glm::scale(E, glm::vec3(Radius * 0.20 + Radius * 0.1 * (CloudMaxLife - CloudLife[i]) / CloudMaxLife));
            Model[3].x = 0.; Model[3].y = 0.; Model[3].z = Radius + Radius * 0.08 * i / CloudNum;
            glm::vec3 TempPos = glm::rotate(E, StarTheta, Axis) * glm::vec4(CloudPos[i], 1.0F);
            Model = glm::rotate(E, glm::acos(glm::dot(glm::vec3(0., 0., 1.), TempPos)), glm::cross(glm::vec3(0., 0., 1.), TempPos)) * Model;
            Model[3].x += RevoPosition.x; Model[3].y += RevoPosition.y; Model[3].z += RevoPosition.z;
            BackgroundShader->setMat4("model", Model);
            TextureCloud[i % 3]->Bind(GL_TEXTURE0);
            DrawParticle();
        }
        //脉冲星
        if (StarType == PULSAR) {
            glDepthMask(GL_FALSE);
            BackgroundShader->use();
            //闪电
            BackgroundShader->setVec3("Filter", 1.0, 1.5, 1.5);
            for (int i = 0; i < LightningNum; i++) {
                if (LightningLife[i] < 0) continue;
                BackgroundShader->setFloat("Alpha", LightningLife[i] / LightningMaxLife * 5.0);
                glm::mat4 Model = Rotate2View * glm::rotate(E, 2 * Pi * i / LightningNum, glm::vec3(0., 0., 1.)) * glm::scale(E, glm::vec3(Radius * 0.3));
                Model[3].x += 1.5 * Radius * LightningPos[i].x + RevoPosition.x;
                Model[3].y += 1.5 * Radius * LightningPos[i].y + RevoPosition.y;
                Model[3].z += 1.5 * Radius * LightningPos[i].z + RevoPosition.z;
                BackgroundShader->setMat4("model", Model);
                TextureLightning->Bind(GL_TEXTURE0);
                DrawParticle();
            }
            //磁极粒子
            BackgroundShader->setVec3("Filter", 1.2, 2.0, 4.0);
            for (int i = LastPulsar; i != (LastPulsar + 1) % PulsarNum; i = (i ? i - 1 : PulsarNum - 1)) {
                BackgroundShader->setFloat("Alpha", PulsarLife[i] / PulsarMaxLife / 8);
                glm::mat4 Model = Rotate2View * glm::scale(glm::rotate(E, 2 * Pi * i / PulsarNum, glm::vec3(0., 0., 1.)), glm::vec3(max(Radius * 0.15, - Radius * 0.15 * log(0.002 * (2 + PulsarMaxLife - PulsarLife[i])) - Radius * 0.2)));
                float OutBurst = Radius * 0.99 + (PulsarMaxLife - PulsarLife[i]) / PulsarMaxLife * Radius * 4;
                Model[3].x = OutBurst * Axis.x; Model[3].y = OutBurst * Axis.y; Model[3].z = OutBurst * Axis.z;
                Model[3].x += RevoPosition.x; Model[3].y += RevoPosition.y; Model[3].z += RevoPosition.z;
                BackgroundShader->setMat4("model", Model);
                TextureFire[i % 3]->Bind(GL_TEXTURE0);
                DrawParticle();
                Model[3].x = 2 * RevoPosition.x - Model[3].x; Model[3].y = 2 * RevoPosition.y - Model[3].y; Model[3].z = 2 * RevoPosition.z - Model[3].z;
                BackgroundShader->setMat4("model", Model);
                DrawParticle();
            }
            //磁极
            glm::mat4 Model = Rotate2Axis * glm::scale(glm::rotate(E, StarTheta / 2, glm::vec3(0., 1., 0.)), glm::vec3(Radius / 2.2));
            Model[3].x += RevoPosition.x; Model[3].y += RevoPosition.y; Model[3].z += RevoPosition.z;
            BackgroundShader->setMat4("model", Model);
            BackgroundShader->setFloat("Alpha", 0.18);
            PulsarDust->Bind(GL_TEXTURE0);
            PulsarDustModel->Draw();
            
        }
        //火焰粒子
        glDepthMask(GL_FALSE);
        BackgroundShader->use();
        switch (StarType) {
        case PURPLE:BackgroundShader->setVec3("Filter", 0.7, 0.0, 4.0); break;
        case ORANGE:BackgroundShader->setVec3("Filter", 4.0, 0.5, 0.0); break;
        case BLUE:case PULSAR:BackgroundShader->setVec3("Filter", 0.5, 1.8, 2.5); break;
        case WHITE:BackgroundShader->setVec3("Filter", 4., 6., 4.); break;
        }
        for (int i = 0; i < FireNum; i++) {
            BackgroundShader->setFloat("Alpha", (1000.0 - FireLife[i]) * FireLife[i] / 500 / 500 / ((StarType == PULSAR || StarType == BLUE) ? 8 : 4));
            glm::mat4 Model = Rotate2View * glm::rotate(E, 2 * Pi * i / FireNum, Axis) * glm::scale(E, glm::vec3(Radius * 0.07 + Radius * 0.00007 * (1000.0 - FireLife[i])));
            glm::vec3 TempPos = glm::rotate(E, StarTheta / 2, Axis) * glm::vec4(FirePos[i], 1.0F);
            Model[3].x += (Radius + 0.00015 * Radius * (1000 - FireLife[i])) * TempPos.x + RevoPosition.x;
            Model[3].y += (Radius + 0.00015 * Radius * (1000 - FireLife[i])) * TempPos.y + RevoPosition.y;
            Model[3].z += (Radius + 0.00015 * Radius * (1000 - FireLife[i])) * TempPos.z + RevoPosition.z;
            BackgroundShader->setMat4("model", Model);
            TextureFire[i % 3]->Bind(GL_TEXTURE0);
            DrawParticle();
        }
        //日耳
        glDepthMask(GL_FALSE);
        BackgroundShader->use();
        switch (StarType) {
        case PURPLE:BackgroundShader->setVec3("Filter", 0.75, 0.5, 1.0); break;
        case ORANGE:BackgroundShader->setVec3("Filter", 2.0, 0.7, 0.0); break;
        case BLUE:case PULSAR:BackgroundShader->setVec3("Filter", 0.5, 1.2, 2.0); break;
        case WHITE:BackgroundShader->setVec3("Filter", 1., 1.2, 1.); break;
        }
        for (int i = 0; i < FlareNum; i++) {
            BackgroundShader->setFloat("Alpha", (((StarType == PULSAR) ? PulsarFlareMaxLife : FlareMaxLife) - FlareLife[i]) * FlareLife[i] / ((StarType == PULSAR) ? PulsarFlareMaxLife : FlareMaxLife) / ((StarType == PULSAR) ? PulsarFlareMaxLife : FlareMaxLife) * 2);
            glm::mat4 Model = glm::scale(glm::rotate(E, 2 * Pi * FlareLife[i] / ((StarType == PULSAR) ? PulsarFlareMaxLife : FlareMaxLife), glm::vec3(0., 0., 1.)), glm::vec3(Distance * Radius / sqrt(Distance * Distance - Radius * Radius) * 0.8));
            Model[3].x = Radius * 0.8 * cos(FlarePos[i]); Model[3].y = Radius * 0.8 * sin(FlarePos[i]);
            Model = Rotate2View * Model;
            Model[3].x += RevoPosition.x; Model[3].y += RevoPosition.y; Model[3].z += RevoPosition.z;
            BackgroundShader->setMat4("model", Model);
            TextureFlare->Bind(GL_TEXTURE0);
            DrawParticle();
        }
        glDepthMask(GL_TRUE);
    }
protected:
    float RevoSpeed;
    float RevoTheta;
    float StarTheta;//已转过的角度
    float StarRotateSpeed;//旋转速度
    TYPE StarType;//行星类型
    //云
    glm::vec3 CloudPos[CloudNum];
    float CloudLife[CloudNum];
    //火焰
    glm::vec3 FirePos[FireNum];
    float FireLife[FireNum];
    //日耳
    float FlarePos[FlareNum];
    float FlareLife[FlareNum];
    //磁极
    float PulsarLife[PulsarNum];
    int LastPulsar;
    //闪电
    glm::vec3 LightningPos[LightningNum];
    float LightningLife[LightningNum];
    //根据行星类别和贴图类别得到贴图前缀名
    static string GetTextureName(TYPE StarType) {
        switch (StarType) {
        case ORANGE:return "orange_";
        case BLUE:return "blue_";
        case PURPLE:return "purple_";
        case WHITE:return "white_";
        default:throw exception("Invalid star type");
        }
    }
    static Texture* TextureDiffuse[5];
    static Texture* TextureNormal[5];
    static Texture* TextureSpecular[5];
    static Texture* TextureCloud[3];
    static Texture* TextureFire[3];
    static Texture* TextureGlow;
    static Texture* TextureFlare;
    static Texture* TextureLightning;
    static Texture* PulsarDust;
    static ObjLoader* PulsarDustModel;
    static Texture* PulsarEnd;
};
Texture* Star::TextureDiffuse[] = {};
Texture* Star::TextureNormal[] = {};
Texture* Star::TextureSpecular[] = {};
Texture* Star::TextureCloud[3] = {};
Texture* Star::TextureFire[3] = {};
Texture* Star::TextureGlow = NULL;
Texture* Star::TextureFlare = NULL;
Texture* Star::TextureLightning = NULL;
Texture* Star::PulsarDust = NULL;
Texture* Star::PulsarEnd = NULL;
ObjLoader* Star::PulsarDustModel = NULL;



//戴森球
class DysonSphere : public CelestialBody {
public:
    static void Init(void) {
        Frame = new ObjLoader("Texture/DysonSphere/frame.obj", true);
        Part1 = new ObjLoader("Texture/DysonSphere/part1.obj", true);
        Part2 = new ObjLoader("Texture/DysonSphere/part2.obj", true);
        Part3 = new ObjLoader("Texture/DysonSphere/part3.obj", true);
        Part4 = new ObjLoader("Texture/DysonSphere/part4.obj", true);
        Diffuse = new Texture(GL_TEXTURE_2D, "Texture/DysonSphere/dysonsphere_diffuse.dds");
        Normal = new Texture(GL_TEXTURE_2D, "Texture/DysonSphere/dysonsphere_normal.dds");
        Specular = new Texture(GL_TEXTURE_2D, "Texture/DysonSphere/dysonsphere_specular.dds");
    }
    //构造函数
    DysonSphere(GLfloat Radius, glm::vec3 Axis, int State, const Star* Center, float RotateSpeed = 1.0)
        : CelestialBody(glm::vec3(0., 0., 0.), Radius, Axis, Center), State(State), RotateSpeed(RotateSpeed), Theta(0.0){}
    void Rotate(double TimeInterval) {
        Theta += TimeInterval * RotateSpeed / 50.0;
        Theta -= (int)(Theta / 2 / Pi) * 2 * Pi;
        if (RevoCenter == NULL) {//绕原点旋转
            RevoPosition = glm::vec3(0., 0., 0.);
        }
        else {
            RevoPosition = RevoCenter->GetPosition();
        }
    }
    void Display(const glm::mat4& projection, const glm::mat4& view, const glm::vec3 viewPos) {
        PlanetShader->use();
        PlanetShader->setMat4("projection", projection);
        PlanetShader->setMat4("view", view);
        PlanetShader->setVec3("viewPos", viewPos);
        //表面
        glDepthMask(GL_TRUE);
        glm::mat4 Model = Rotate2Axis * glm::rotate(glm::scale(E, glm::vec3(Radius / 5.2)), Theta, glm::vec3(0., 1., 0.));
        Model[3].x = RevoPosition.x; Model[3].y = RevoPosition.y; Model[3].z = RevoPosition.z;
        Diffuse->Bind(GL_TEXTURE0);
        Normal->Bind(GL_TEXTURE1);
        Specular->Bind(GL_TEXTURE2);
        PlanetShader->setMat4("model", Model);
        switch (State) {
        case 4:Part4->Draw();
        case 3:Part3->Draw();
        case 2:Part2->Draw();
        case 1:Part1->Draw();
        case 0:Frame->Draw();
        }
    }
private:
    int State;
    float RotateSpeed;
    float Theta;
    static ObjLoader* Frame, * Part1, * Part2, * Part3, * Part4;
    static Texture* Diffuse, * Normal, * Specular;
};
ObjLoader* DysonSphere::Frame = NULL;
ObjLoader* DysonSphere::Part1 = NULL;
ObjLoader* DysonSphere::Part2 = NULL;
ObjLoader* DysonSphere::Part3 = NULL;
ObjLoader* DysonSphere::Part4 = NULL;
Texture* DysonSphere::Diffuse = NULL;
Texture* DysonSphere::Normal = NULL;
Texture* DysonSphere::Specular = NULL;


//环世界
class RingWorld : public CelestialBody {
public:
    static void Init(void) {
        Seam = new ObjLoader("Texture/RingWorld/seam.obj", true);
        Tech = new ObjLoader("Texture/RingWorld/tech.obj", true);
        Habit = new ObjLoader("Texture/RingWorld/habit.obj", true);
        Ground = new ObjLoader("Texture/RingWorld/ground.obj", true);
        Cloud = new ObjLoader("Texture/RingWorld/cloud.obj", true);
        TextureSeam[0] = new Texture(GL_TEXTURE_2D, "Texture/RingWorld/seam_diffuse.dds");
        TextureSeam[1] = new Texture(GL_TEXTURE_2D, "Texture/RingWorld/seam_normal.dds");
        TextureSeam[2] = new Texture(GL_TEXTURE_2D, "Texture/RingWorld/seam_specular.dds");
        TextureTech[0] = new Texture(GL_TEXTURE_2D, "Texture/RingWorld/tech_diffuse.dds");
        TextureTech[1] = new Texture(GL_TEXTURE_2D, "Texture/RingWorld/tech_normal.dds");
        TextureTech[2] = new Texture(GL_TEXTURE_2D, "Texture/RingWorld/tech_specular.dds");
        TextureHabit[0] = new Texture(GL_TEXTURE_2D, "Texture/RingWorld/habit_diffuse.dds");
        TextureHabit[1] = new Texture(GL_TEXTURE_2D, "Texture/RingWorld/habit_normal.dds");
        TextureHabit[2] = new Texture(GL_TEXTURE_2D, "Texture/RingWorld/habit_specular.dds");
        if (Planet::TextureDiffuse[Planet::GetTypeID(Planet::CONTINENTAL, 0)] == NULL) {
            Planet::TextureDiffuse[Planet::GetTypeID(Planet::CONTINENTAL, 0)] = new Texture(GL_TEXTURE_2D, "Texture/Planets/continental_01_diffuse.dds");
            Planet::TextureNormal[Planet::GetTypeID(Planet::CONTINENTAL, 0)] = new Texture(GL_TEXTURE_2D, "Texture/Planets/continental_01_normal.dds");
            Planet::TextureSpecular[Planet::GetTypeID(Planet::CONTINENTAL, 0)] = new Texture(GL_TEXTURE_2D, "Texture/Planets/continental_01_specular.dds");
        }
        TextureGround[0] = Planet::TextureDiffuse[Planet::GetTypeID(Planet::CONTINENTAL, 0)];
        TextureGround[1] = Planet::TextureNormal[Planet::GetTypeID(Planet::CONTINENTAL, 0)];
        TextureGround[2] = Planet::TextureSpecular[Planet::GetTypeID(Planet::CONTINENTAL, 0)];
        if (Planet::TextureDiffuse[Planet::GetTypeID(Planet::CLOUDS, 0)] == NULL) {
            Planet::TextureDiffuse[Planet::GetTypeID(Planet::CLOUDS, 0)] = new Texture(GL_TEXTURE_2D, "Texture/Planets/clouds_01_diffuse.dds");
            Planet::TextureNormal[Planet::GetTypeID(Planet::CLOUDS, 0)] = new Texture(GL_TEXTURE_2D, "Texture/Planets/clouds_01_normal.dds");
            Planet::TextureSpecular[Planet::GetTypeID(Planet::CLOUDS, 0)] = new Texture(GL_TEXTURE_2D, "Texture/Planets/clouds_01_specular.dds");
        }
        TextureCloud[0] = Planet::TextureDiffuse[Planet::GetTypeID(Planet::CLOUDS, 0)];
        TextureCloud[1] = Planet::TextureNormal[Planet::GetTypeID(Planet::CLOUDS, 0)];
        TextureCloud[2] = Planet::TextureSpecular[Planet::GetTypeID(Planet::CLOUDS, 0)];
    }

    static void DrawTech(void) {
        TextureTech[0]->Bind(GL_TEXTURE0);
        TextureTech[1]->Bind(GL_TEXTURE1);
        TextureTech[2]->Bind(GL_TEXTURE2);
        Tech->Draw();
    }
    static void DrawSeam(void) {
        TextureSeam[0]->Bind(GL_TEXTURE0);
        TextureSeam[1]->Bind(GL_TEXTURE1);
        TextureSeam[2]->Bind(GL_TEXTURE2);
        Seam->Draw();
    }
    static void DrawHabit(void) {
        TextureHabit[0]->Bind(GL_TEXTURE0);
        TextureHabit[1]->Bind(GL_TEXTURE1);
        TextureHabit[2]->Bind(GL_TEXTURE2);
        Habit->Draw();
        TextureGround[0]->Bind(GL_TEXTURE0);
        TextureGround[1]->Bind(GL_TEXTURE1);
        TextureGround[2]->Bind(GL_TEXTURE2);
        Ground->Draw();
        TextureCloud[0]->Bind(GL_TEXTURE0);
        TextureCloud[1]->Bind(GL_TEXTURE1);
        TextureCloud[2]->Bind(GL_TEXTURE2);
        Cloud->Draw();
    }
    //构造函数
    RingWorld(GLfloat Radius, glm::vec3 Axis, float RotateSpeed = 1.0)
        : CelestialBody(glm::vec3(0., 0., 0.), Radius, Axis, NULL), RotateSpeed(RotateSpeed), Theta(0.0) {}
    void Rotate(double TimeInterval) {
        Theta += TimeInterval * RotateSpeed / 50.0;
        Theta -= (int)(Theta / 2 / Pi) * 2 * Pi;
    }
    //画
    void Display(const glm::mat4& projection, const glm::mat4& view, const glm::vec3 viewPos) {
        glDepthMask(GL_TRUE);
        PlanetShader->use();
        PlanetShader->setMat4("projection", projection);
        PlanetShader->setMat4("view", view);
        PlanetShader->setVec3("viewPos", viewPos);
        glm::mat4 Model = glm::scale(E, glm::vec3(Radius / 450));
        Model[3].z -= Radius;
        Model = glm::rotate(E, Theta, glm::vec3(0., 1., 0.)) * Model;
        for (int i = 0; i < 4; i++) {
            //连接环
            PlanetShader->setMat4("model", Rotate2Axis * Model);
            DrawSeam();
            Model = glm::rotate(E, Pi / 6, glm::vec3(0., 1., 0.)) * Model;
            //居住环
            PlanetShader->setMat4("model", Rotate2Axis * Model);
            DrawHabit();
            Model = glm::rotate(E, Pi / 6, glm::vec3(0., 1., 0.)) * Model;
            //科研环1
            PlanetShader->setMat4("model", Rotate2Axis * Model);
            DrawTech();
            Model = glm::rotate(E, Pi / 6, glm::vec3(0., 1., 0.)) * Model;
        }
    }
private:
    float RotateSpeed;
    float Theta;
    static ObjLoader* Seam, * Tech, * Habit, * Ground,* Cloud;
    static Texture* TextureSeam[3];
    static Texture* TextureTech[3];
    static Texture* TextureHabit[3];
    static Texture* TextureGround[3];
    static Texture* TextureCloud[3];
};
ObjLoader* RingWorld::Seam = NULL;
ObjLoader* RingWorld::Tech = NULL;
ObjLoader* RingWorld::Habit = NULL;
ObjLoader* RingWorld::Ground = NULL;
ObjLoader* RingWorld::Cloud = NULL;
Texture* RingWorld::TextureSeam[3] = {};
Texture* RingWorld::TextureTech[3] = {};
Texture* RingWorld::TextureHabit[3] = {};
Texture* RingWorld::TextureGround[3] = {};
Texture* RingWorld::TextureCloud[3] = {};


//卫星
class Satellite : public CelestialBody {
public:
    //初始化材质
    static void Init(void) {

    }
    enum TYPE {
        ASTEROID,       //小行星, 7种模型
        COORDINATION,   //指挥中心, 1种模型
        GATEWAY,        //星门, 1种模型
        MEGASHIPYARD,   //船坞, 1种模型
        SPYORB,         //哨卫天线, 1种模型
        STARBASE,       //星堡, 14种模型,
        THINKTANK,      //科学枢纽, 1种模型
    };
    //构造函数
    Satellite(glm::vec3 Position, GLfloat Radius, glm::vec3 Axis, const CelestialBody* RevoCenter, TYPE Type, int TextureType = 0.0F, float RotateSpeed = 2.0, float RevoSpeed = 0.5)
        :CelestialBody(Position, Radius, Axis, RevoCenter), Theta(0.0), Type(Type), TextureType(TextureType), RotateSpeed(RotateSpeed), RevoSpeed(RevoSpeed), RevoTheta(0.0) {
        //加载模型及纹理贴图
        if (SatelliteTexture[GetTextureID(Type, TextureType)][0] == NULL) {
            string TextureName, ObjName, Index;
            switch (Type) {
            case ASTEROID:
                if (TextureType <= 2) TextureName = "Texture/Asteroids/asteroid_01_"; else TextureName = "Texture/Asteroids/asteroid_02_";
                ObjName = "Texture/Asteroids/asteroid_0"; ObjName.push_back(TextureType + 1 + '0'); ObjName += ".obj";
                SatelliteTexture[GetTextureID(Type, TextureType)][0] = new Texture(GL_TEXTURE_2D, TextureName + "diffuse.dds");
                SatelliteTexture[GetTextureID(Type, TextureType)][1] = new Texture(GL_TEXTURE_2D, TextureName + "normal.dds");
                SatelliteTexture[GetTextureID(Type, TextureType)][2] = new Texture(GL_TEXTURE_2D, TextureName + "specular.dds");
                SatelliteObj[GetObjID(Type, TextureType)] = new ObjLoader(ObjName, true);
                break;
            case COORDINATION:
                TextureName = "Texture/Coordination/coordination_";
                ObjName = "Texture/Coordination/coordination.obj";
                SatelliteTexture[GetTextureID(Type, TextureType)][0] = new Texture(GL_TEXTURE_2D, TextureName + "diffuse.dds");
                SatelliteTexture[GetTextureID(Type, TextureType)][1] = new Texture(GL_TEXTURE_2D, TextureName + "normal.dds");
                SatelliteTexture[GetTextureID(Type, TextureType)][2] = new Texture(GL_TEXTURE_2D, TextureName + "specular.dds");
                SatelliteObj[GetObjID(Type, TextureType)] = new ObjLoader(ObjName, true);
                break;
            case GATEWAY:
                TextureName = "Texture/GateWay/gateway_";
                ObjName = "Texture/GateWay/gateway.obj";
                SatelliteTexture[GetTextureID(Type, TextureType)][0] = new Texture(GL_TEXTURE_2D, TextureName + "diffuse.dds");
                SatelliteTexture[GetTextureID(Type, TextureType)][1] = new Texture(GL_TEXTURE_2D, TextureName + "normal.dds");
                SatelliteTexture[GetTextureID(Type, TextureType)][2] = new Texture(GL_TEXTURE_2D, TextureName + "specular.dds");
                SatelliteObj[GetObjID(Type, TextureType)] = new ObjLoader(ObjName, true);
                break;
            case MEGASHIPYARD:
                TextureName = "Texture/MegaShipYard/megashipyard_";
                ObjName = "Texture/MegaShipYard/megashipyard.obj";
                SatelliteTexture[GetTextureID(Type, TextureType)][0] = new Texture(GL_TEXTURE_2D, TextureName + "diffuse.dds");
                SatelliteTexture[GetTextureID(Type, TextureType)][1] = new Texture(GL_TEXTURE_2D, TextureName + "normal.dds");
                SatelliteTexture[GetTextureID(Type, TextureType)][2] = new Texture(GL_TEXTURE_2D, TextureName + "specular.dds");
                SatelliteObj[GetObjID(Type, TextureType)] = new ObjLoader(ObjName, true);
                break;
            case SPYORB:
                TextureName = "Texture/SpyOrb/spyorb_";
                ObjName = "Texture/SpyOrb/spyorb.obj";
                SatelliteTexture[GetTextureID(Type, TextureType)][0] = new Texture(GL_TEXTURE_2D, TextureName + "diffuse.dds");
                SatelliteTexture[GetTextureID(Type, TextureType)][1] = new Texture(GL_TEXTURE_2D, TextureName + "normal.dds");
                SatelliteTexture[GetTextureID(Type, TextureType)][2] = new Texture(GL_TEXTURE_2D, TextureName + "specular.dds");
                SatelliteObj[GetObjID(Type, TextureType)] = new ObjLoader(ObjName, true);
                break;
            case STARBASE:
                Index = "00"; Index[0] = (TextureType + 1) / 10 + '0'; Index[1] = (TextureType + 1) % 10 + '0';
                TextureName = "Texture/StarBase/starbase_" + Index + "_";
                ObjName = "Texture/StarBase/starbase_" + Index + ".obj";
                SatelliteTexture[GetTextureID(Type, TextureType)][0] = new Texture(GL_TEXTURE_2D, TextureName + "diffuse.dds");
                SatelliteTexture[GetTextureID(Type, TextureType)][1] = new Texture(GL_TEXTURE_2D, TextureName + "normal.dds");
                SatelliteTexture[GetTextureID(Type, TextureType)][2] = new Texture(GL_TEXTURE_2D, TextureName + "specular.dds");
                SatelliteObj[GetObjID(Type, TextureType)] = new ObjLoader(ObjName, true);
                break;
            case THINKTANK:
                TextureName = "Texture/ThinkTank/thinktank_";
                ObjName = "Texture/ThinkTank/thinktank.obj";
                SatelliteTexture[GetTextureID(Type, TextureType)][0] = new Texture(GL_TEXTURE_2D, TextureName + "diffuse.dds");
                SatelliteTexture[GetTextureID(Type, TextureType)][1] = new Texture(GL_TEXTURE_2D, TextureName + "normal.dds");
                SatelliteTexture[GetTextureID(Type, TextureType)][2] = new Texture(GL_TEXTURE_2D, TextureName + "specular.dds");
                SatelliteObj[GetObjID(Type, TextureType)] = new ObjLoader(ObjName, true);
                break;
            }
        }
    }
    //进行旋转
    void Rotate(double TimeInterval) {
        Theta += TimeInterval * RotateSpeed / 50.0;
        Theta -= (int)(Theta / 2 / Pi) * 2 * Pi;
        RevoTheta += TimeInterval * RevoSpeed / 50.0;
        RevoTheta -= (int)(RevoTheta / 2 / Pi) * 2 * Pi;
        if (RevoCenter == NULL) {//绕原点旋转
            RevoPosition = glm::rotate(E, RevoTheta, glm::vec3(0., 1., 0.)) * glm::vec4(StartPosition, 1.0F);
        }
        else {
            RevoPosition = glm::rotate(E, RevoTheta, glm::vec3(0., 1., 0.)) * glm::vec4(StartPosition - RevoCenter->GetStartPosition(), 1.0F) + glm::vec4(RevoCenter->GetPosition(), 1.0F);
        }
    }
    //画图
    void Display(const glm::mat4& projection, const glm::mat4& view, const glm::vec3 viewPos) {
        PlanetShader->use();
        PlanetShader->setMat4("projection", projection);
        PlanetShader->setMat4("view", view);
        PlanetShader->setVec3("viewPos", viewPos);
        glm::mat4 Model = Rotate2Axis * glm::rotate(E, Theta, glm::vec3(0., 1., 0.)) * glm::scale(E, glm::vec3(Radius));
        Model[3].x += RevoPosition.x; Model[3].y += RevoPosition.y; Model[3].z += RevoPosition.z;
        SatelliteTexture[GetTextureID(Type, TextureType)][0]->Bind(GL_TEXTURE0);
        SatelliteTexture[GetTextureID(Type, TextureType)][1]->Bind(GL_TEXTURE1);
        SatelliteTexture[GetTextureID(Type, TextureType)][2]->Bind(GL_TEXTURE2);
        PlanetShader->setMat4("model", Model);
        SatelliteObj[GetObjID(Type, TextureType)]->Draw();
    }
protected:
    float RevoSpeed;
    float RevoTheta;
    float Theta;//已转过的角度
    float RotateSpeed;//旋转速度
    TYPE Type;
    int TextureType;
    static Texture* SatelliteTexture[21][3];
    static ObjLoader* SatelliteObj[26];
    static int GetTextureID(TYPE Type, int TextureType) {
        switch (Type) {
        case ASTEROID:if (TextureType <= 2)return 0; else return 1;
        case COORDINATION:return 2;
        case GATEWAY:return 3;
        case MEGASHIPYARD:return 4;
        case SPYORB:return 5;
        case STARBASE:return 6 + TextureType;
        case THINKTANK:return 20;
        }
    }
    static int GetObjID(TYPE Type, int TextureType) {
        switch (Type) {
        case ASTEROID:return TextureType;
        case COORDINATION:return 7;
        case GATEWAY:return 8;
        case MEGASHIPYARD:return 9;
        case SPYORB:return 10;
        case STARBASE:return 11 + TextureType;
        case THINKTANK:return 25;
        }
    }
};
Texture* Satellite::SatelliteTexture[21][3] = {};
ObjLoader* Satellite::SatelliteObj[26] = {};