#define _CRT_SECURE_NO_WARNINGS
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include "Shader.h"
#include "Texture.hpp"
#include "CelestialBody.hpp"
#include "ObjLoader.hpp"
#include "SkyBox.hpp"


using namespace std;
#define ScreenWidth 1000
#define ScreenHeight 800

//视角对象
class ViewPoint {
public:
    ViewPoint(GLfloat Ox, GLfloat Oy, GLfloat Oz, GLfloat Fov, GLfloat AspectRatio, GLfloat Near = 1.0f, GLfloat Far = 1000000.0f) :
        CameraPos(glm::vec3(Ox, Oy, Oz)),
        CameraFront(glm::vec3(0.0f, 0.0f, -1.0f)),
        CameraUp(glm::vec3(0.0f, 1.0f, 0.0f)),
        CameraLeft(glm::vec3(-1.0f, 0.0f, 0.0f)),
        Pitch(0.0f),
        Yaw(-175.5f),
        Fov(Fov),
        AspectRatio(AspectRatio),
        Near(Near),
        Far(Far) {
        Turn(0.0f, 0.0f);
    }
    ~ViewPoint(void) { }
    void GoFront(GLfloat Distance) {
        CameraPos += Distance * CameraFront;
    }
    void GoBack(GLfloat Distance) {
        CameraPos -= Distance * CameraFront;
    }
    void GoLeft(GLfloat Distance) {
        CameraPos += Distance * CameraLeft;
    }
    void GoRight(GLfloat Distance) {
        CameraPos -= Distance * CameraLeft;
    }
    void GoUp(GLfloat Distance) {
        CameraPos += Distance * CameraUp;
    }
    void GoDown(GLfloat Distance) {
        CameraPos -= Distance * CameraUp;
    }
    void Move(glm::vec3 Distance) {
        CameraPos += Distance;
    }
    void Turn(GLfloat PitchDelta, GLfloat YawDelta) {
        GLfloat Pi = glm::pi<GLfloat>();
        Pitch += PitchDelta;
        Yaw += YawDelta;
        cout << Yaw << endl;
        if (Pitch >= Pi / 2) Pitch = Pi / 2;
        if (Pitch <= -Pi / 2) Pitch = -Pi / 2;
        //更新三个方向向量
        CameraFront.x = cos(Pitch) * cos(Yaw);
        CameraFront.y = sin(Pitch);
        CameraFront.z = cos(Pitch) * sin(Yaw);
        CameraUp.x = -sin(Pitch) * cos(Yaw);
        CameraUp.y = cos(Pitch);
        CameraUp.z = -sin(Pitch) * sin(Yaw);
        CameraLeft = glm::cross(CameraUp, CameraFront);
    }
    glm::vec3 Front(void) {
        return CameraFront;
    }
    glm::vec3 Up(void) {
        return CameraUp;
    }
    glm::vec3 Left(void) {
        return CameraLeft;
    }
    glm::vec3 Position(void) {
        return CameraPos;
    }
    glm::mat4 Projection(void) {
        return glm::perspective(glm::radians(Fov), AspectRatio, Near, Far);
    }
    glm::mat4 View(void) {
        return glm::lookAt(CameraPos, CameraPos + CameraFront, CameraUp);
    }
private:
    glm::vec3 CameraPos;
    glm::vec3 CameraFront;
    glm::vec3 CameraUp;
    glm::vec3 CameraLeft;
    GLfloat Pitch, Yaw, Fov, AspectRatio, Near, Far;
};

//窗口大小改变回调函数
void FramebufferSizeCallback(GLFWwindow* Window, int Width, int Height);
//键盘消息处理函数
void KeyBoardEventCallback(GLFWwindow* Window, double DeltaTime);
//鼠标消息处理函数
void MouseEventCallback(GLFWwindow* Window, double Xpos, double Ypos);
//初始化函数
GLFWwindow* Init(int Width, int Height, string Name);
//消息循环
void MainLoop(GLFWwindow* Window);
//设置视角
ViewPoint Camera(-900.0f, 0.0f, 4100.0f, 45.0f, (GLfloat)ScreenWidth / ScreenHeight);
int main(){
    //初始化
    GLFWwindow* Window = Init(ScreenWidth, ScreenHeight, "Stellar");
    //进入消息循环
    MainLoop(Window);
    //关闭GLFW
    glfwTerminate();
    return 0;
}

//初始化函数
GLFWwindow* Init(int Width, int Height, string Name) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    GLFWwindow* Window = glfwCreateWindow(Width, Height, Name.c_str(), NULL, NULL);
    if (Window == NULL) {
        cout << "Failed to create GLFW window" << endl;
        glfwTerminate();
        return NULL;
    }
    glfwMakeContextCurrent(Window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Failed to initialize GLAD" << endl;
        return NULL;
    }
    //设置窗口渲染区域大小
    glViewport(0, 0, Width, Height);
    //注册窗口大小改变回调函数
    glfwSetFramebufferSizeCallback(Window, FramebufferSizeCallback);
    //注册鼠标消息回调函数
    glfwSetCursorPosCallback(Window, MouseEventCallback);
    glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    //使能
    glEnable(GL_STENCIL_TEST);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //返回窗口
    return Window;
}

//消息循环
void MainLoop(GLFWwindow* Window) {
    double CurrentTime, LastTime, DeltaTime;
    //初始化
    CelestialBody::Init();
    Planet::Init();
    Star::Init();
    DysonSphere::Init();
    RingWorld::Init();
    SkyBox SkyBox("Texture/SkyBox/Purple/");
    //加载星球对象
    //恒星
    Star Sun(glm::vec3(0., 0., -960.), 600, glm::vec3(0., 1., 0.5), Star::TYPE::ORANGE, NULL, 1.0F, 0.0F);
    Star Sirius(glm::vec3(932, 0., 480.), 500, glm::vec3(0., 0.5, -1.), Star::TYPE::PURPLE, NULL, 1.0F, 0.0F);
    Star Evil(glm::vec3(-932., 0., 480.), 300, glm::vec3(0., 1., 0.3), Star::TYPE::PULSAR, NULL, 150.0F, 0.0F);
    DysonSphere Sphere(800, glm::vec3(0., 1., 0.), 2, &Sun, 1.0F);
    RingWorld Ring(7500.0, glm::vec3(0., 1., 0.), 0.1F);
    Satellite Spy(glm::vec3(0., -200., 3250), 20, glm::vec3(0., 0.9, 0.2), NULL, Satellite::SPYORB, 0, 2.0F, 0.0F);
    //Satellite ShipYard(glm::vec3(-400., 1000., 4500), 10, glm::vec3(0., 1.0, 0.0), NULL, Satellite::MEGASHIPYARD, 0, 0.0F, 0.0F);
    //Satellite Coord(glm::vec3(500., -1800., 3500), 15, glm::vec3(0., 1.0, 0.0), NULL, Satellite::COORDINATION, 0, 0.0F, 0.0F);
    //Satellite Gate(glm::vec3(-500., 0., -3500), 5, glm::vec3(0., 1.0, 0.0), NULL, Satellite::GATEWAY, 0, 0.0F, 0.0F);
    Satellite Sci(glm::vec3(1000., 0., 5500.), 8, glm::vec3(0., 1.0, 0.0), NULL, Satellite::THINKTANK, 0, 0.0F, 0.0F);
    //近日行星
    Planet Molten(glm::vec3(0., 0., 2500.), 100, glm::vec3(0., 0.5, 0.2), Planet::MOLTEN, NULL, 1, 0, 2.0F, 3.0F, 0.0F);
    Planet Earth(glm::vec3(0., 0. , 4500.), 350, glm::vec3(0., 0.9, -0.1), Planet::TYPE::CONTINENTAL, NULL, 0, 1, 2.0F, 3.0F, 0.0F);
    //小行星
    /*int NumAsteroid = 90;
    for (int i = 0; i < NumAsteroid; i++) {
        CelestialBody::AddCelestialBody(new Asteroid(glm::vec3(137. * sin(6.2832 * (i + 0.5 * rand() / RAND_MAX) / NumAsteroid) + 2.F * rand() / RAND_MAX, 2.F * rand() / RAND_MAX - 1.F, 140. * cos(6.2832 * (i + 0.5 * rand() / RAND_MAX) / NumAsteroid) + 2.F * rand() / RAND_MAX) , 1.F + 0.7F * (float)rand() / RAND_MAX, glm::vec3(Distribution(Generator), Distribution(Generator), Distribution(Generator)), NULL, 4.0F, 0.5F));
        CelestialBody::AddCelestialBody(new Asteroid(glm::vec3(142. * sin(6.2832 * (i + 0.5 * rand() / RAND_MAX) / NumAsteroid) + 2.F * rand() / RAND_MAX, 2.F * rand() / RAND_MAX - 1.F, 145. * cos(6.2832 * (i + 0.5 * rand() / RAND_MAX) / NumAsteroid) + 2.F * rand() / RAND_MAX), 1.F + 0.7F * (float)rand() / RAND_MAX, glm::vec3(Distribution(Generator), Distribution(Generator), Distribution(Generator)), NULL, 4.0F, 0.4F));
        CelestialBody::AddCelestialBody(new Asteroid(glm::vec3(147. * sin(6.2832 * (i + 0.5 * rand() / RAND_MAX) / NumAsteroid) + 2.F * rand() / RAND_MAX, 2.F * rand() / RAND_MAX - 1.F, 150. * cos(6.2832 * (i + 0.5 * rand() / RAND_MAX) / NumAsteroid) + 2.F * rand() / RAND_MAX), 1.F + 0.7F * (float)rand() / RAND_MAX, glm::vec3(Distribution(Generator), Distribution(Generator), Distribution(Generator)), NULL, 4.0F, 0.3F));
        CelestialBody::AddCelestialBody(new Asteroid(glm::vec3(152. * sin(6.2832 * (i + 0.5 * rand() / RAND_MAX) / NumAsteroid) + 2.F * rand() / RAND_MAX, 2.F * rand() / RAND_MAX - 1.F, 155. * cos(6.2832 * (i + 0.5 * rand() / RAND_MAX) / NumAsteroid) + 2.F * rand() / RAND_MAX), 1.F + 0.7F * (float)rand() / RAND_MAX, glm::vec3(Distribution(Generator), Distribution(Generator), Distribution(Generator)), NULL, 4.0F, 0.2F));
    }*/
    //巨行星
    Planet Saturn(glm::vec3(0., 0., 9500.), 500, glm::vec3(0.1, 0.6, -0.4), Planet::GASGIANT, NULL, 0, 0, 1.0F, 2.0F, 0.0F, true);
    //将所有天体添加到列表中
    CelestialBody::AddCelestialBody(&Ring);
    CelestialBody::AddCelestialBody(&Sphere);
    CelestialBody::AddCelestialBody(&Spy);
    //CelestialBody::AddCelestialBody(&ShipYard);
    //CelestialBody::AddCelestialBody(&Coord);
    //CelestialBody::AddCelestialBody(&Gate);
    CelestialBody::AddCelestialBody(&Sci);
    CelestialBody::AddCelestialBody(&Sun);
    CelestialBody::AddCelestialBody(&Sirius);
    CelestialBody::AddCelestialBody(&Evil);
    CelestialBody::AddCelestialBody(&Molten);
    CelestialBody::AddCelestialBody(&Earth);
    CelestialBody::AddCelestialBody(&Saturn);
    //加载飞船
    //消息循环
    LastTime = glfwGetTime();
    while (!glfwWindowShouldClose(Window)) {
        //计算时间差
        CurrentTime = glfwGetTime();
        DeltaTime = CurrentTime - LastTime;
        LastTime = CurrentTime;
        //处理输入
        KeyBoardEventCallback(Window, DeltaTime);
        //清空背景
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        SkyBox.Display(Camera.Projection(), Camera.View());
        //画行星
        CelestialBody::RotateAll(DeltaTime);
        CelestialBody::DisplayAll(Camera.Projection(), Camera.View(), Camera.Position());
        // 检查并调用事件，交换缓冲
        glfwPollEvents();
        glfwSwapBuffers(Window);
    }
}
//窗口大小改变回调函数
void FramebufferSizeCallback(GLFWwindow* Window, int Width, int Height){
    glViewport(0, 0, Width, Height);
}

//键盘消息处理函数
void KeyBoardEventCallback(GLFWwindow* Window, double DeltaTime) {
    if (glfwGetKey(Window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(Window, true);
    if (glfwGetKey(Window, GLFW_KEY_W) == GLFW_PRESS)
        Camera.GoFront(400 * DeltaTime);
    if (glfwGetKey(Window, GLFW_KEY_S) == GLFW_PRESS)
        Camera.GoBack(400 * DeltaTime);
    if (glfwGetKey(Window, GLFW_KEY_A) == GLFW_PRESS)
        Camera.GoLeft(400 * DeltaTime);
    if (glfwGetKey(Window, GLFW_KEY_D) == GLFW_PRESS)
        Camera.GoRight(400 * DeltaTime);
    if (glfwGetKey(Window, GLFW_KEY_SPACE) == GLFW_PRESS)
        Camera.GoUp(400 * DeltaTime);
    if (glfwGetKey(Window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        Camera.GoDown(400 * DeltaTime);
}
//鼠标消息处理函数
void MouseEventCallback(GLFWwindow* Window, double Xpos, double Ypos) {
    static double LastX = Xpos, LastY = Ypos;
    Camera.Turn(0.005*(LastY - Ypos), 0.005*(Xpos - LastX));
    LastX = Xpos;
    LastY = Ypos;
}