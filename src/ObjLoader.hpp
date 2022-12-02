#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include "Texture.hpp"

using namespace std;

//ģ�ͼ�����
class ObjLoader {
private:
    GLuint VAO, VBO;
    int NumElements;
public:
    //���캯��
    ObjLoader(string FileName, bool GenerateTangent = false) {
        //�������ݱ�����
        vector<GLfloat> VertexCoords;//��Ŷ���(x,y,z)����
        vector<GLfloat> TextureCoords;//�����������(x,y)
        vector<GLfloat> NormalVectors;//��ŷ�����(x,y,z)
        vector<GLint> VertexIndex;
        vector<GLint> TextureIndex;
        vector<GLint> NormalIndex;
        vector<GLfloat> Buffer;
        //���ļ�
        std::ifstream Fin(FileName);
        std::string Line;
        while (getline(Fin, Line)) {
            if (Line.substr(0, 2) == "vt") {//��������(u,v)
                GLfloat x, y;
                std::istringstream s(Line.substr(3));
                s >> x; s >> y;
                TextureCoords.push_back(x);
                TextureCoords.push_back(y);
            }
            else if (Line.substr(0, 2) == "vn") {//������(x,y,z)
                GLfloat x, y, z;
                std::istringstream s(Line.substr(3));
                s >> x; s >> y; s >> z;
                NormalVectors.push_back(x);
                NormalVectors.push_back(y);
                NormalVectors.push_back(z);
            }
            else if (Line.substr(0, 1) == "v") {//������(x,y,z)
                GLfloat x, y, z;
                std::istringstream s(Line.substr(2));
                s >> x; s >> y; s >> z;
                VertexCoords.push_back(x);
                VertexCoords.push_back(y);
                VertexCoords.push_back(z);
            }
            else if (Line.substr(0, 1) == "f") {//��������/������������/����������
                std::istringstream sstm(Line.substr(2));
                //���п�����3���������, Ҳ�п�����4���������
                string S[4] = { "", "", "", "" };
                int V[4] = {}, VT[4] = {}, VN[4] = {};
                int i;
                for (i = 0; i < 4; i++) {
                    sstm >> S[i];
                    if (S[i] == "") break;
                    int Split1 = S[i].find_first_of('/'), Split2 = S[i].find_last_of('/');
                    V[i] = atoi(S[i].substr(0, Split1).c_str()) - 1;
                    VT[i] = atoi(S[i].substr(Split1 + 1, Split2 - Split1 - 1).c_str()) - 1;
                    VN[i] = atoi(S[i].substr(Split2 + 1).c_str()) - 1;
                }
                //��������
                VertexIndex.push_back(V[0]);
                VertexIndex.push_back(V[1]);
                VertexIndex.push_back(V[2]);
                TextureIndex.push_back(VT[0]);
                TextureIndex.push_back(VT[1]);
                TextureIndex.push_back(VT[2]);
                NormalIndex.push_back(VN[0]);
                NormalIndex.push_back(VN[1]);
                NormalIndex.push_back(VN[2]);
                if (i == 4) {
                    VertexIndex.push_back(V[2]);
                    VertexIndex.push_back(V[3]);
                    VertexIndex.push_back(V[0]);
                    TextureIndex.push_back(VT[2]);
                    TextureIndex.push_back(VT[3]);
                    TextureIndex.push_back(VT[0]);
                    NormalIndex.push_back(VN[2]);
                    NormalIndex.push_back(VN[3]);
                    NormalIndex.push_back(VN[0]);
                }
            }
            else if (Line.substr(0, 1) == "#") {
                //cout << "Line commanded." << endl;
            }
            else {
                //cout << "Line Unrecogized." << endl;
            }
        }
        Fin.close();
        NumElements = VertexIndex.size();

        //�ָ�����������
        if (GenerateTangent) {
            glm::vec3 T, B;
            for (int i = 0; i < NumElements; i++) {
                if (i % 3 == 0) {//��ĵ�һ�����㣬���㸱����
                    float Du1 = TextureCoords[2 * TextureIndex[i + 1] + 0] - TextureCoords[2 * TextureIndex[i + 0] + 0];
                    float Dv1 = TextureCoords[2 * TextureIndex[i + 1] + 1] - TextureCoords[2 * TextureIndex[i + 0] + 1];
                    float Du2 = TextureCoords[2 * TextureIndex[i + 2] + 0] - TextureCoords[2 * TextureIndex[i + 1] + 0];
                    float Dv2 = TextureCoords[2 * TextureIndex[i + 2] + 1] - TextureCoords[2 * TextureIndex[i + 1] + 1];
                    float E1x = VertexCoords[3 * VertexIndex[i + 1] + 0] - VertexCoords[3 * VertexIndex[i + 0] + 0];
                    float E1y = VertexCoords[3 * VertexIndex[i + 1] + 1] - VertexCoords[3 * VertexIndex[i + 0] + 1];
                    float E1z = VertexCoords[3 * VertexIndex[i + 1] + 2] - VertexCoords[3 * VertexIndex[i + 0] + 2];
                    float E2x = VertexCoords[3 * VertexIndex[i + 2] + 0] - VertexCoords[3 * VertexIndex[i + 1] + 0];
                    float E2y = VertexCoords[3 * VertexIndex[i + 2] + 1] - VertexCoords[3 * VertexIndex[i + 1] + 1];
                    float E2z = VertexCoords[3 * VertexIndex[i + 2] + 2] - VertexCoords[3 * VertexIndex[i + 1] + 2];
                    glm::mat2x3 TB = glm::mat2x3(E1x, E1y, E1z, E2x, E2y, E2z) * glm::inverse(glm::mat2(Du1, Dv1, Du2, Dv2));
                    B = TB[1];
                    if (glm::cross(glm::vec3(Du1, Dv1, 0.0), glm::vec3(Du2, Dv2, 0.0)).z < 0) B = -B;
                }
                //�������
                Buffer.push_back(VertexCoords[3 * VertexIndex[i] + 0]);
                Buffer.push_back(VertexCoords[3 * VertexIndex[i] + 1]);
                Buffer.push_back(VertexCoords[3 * VertexIndex[i] + 2]);
                //������
                Buffer.push_back(NormalVectors[3 * NormalIndex[i] + 0]);
                Buffer.push_back(NormalVectors[3 * NormalIndex[i] + 1]);
                Buffer.push_back(NormalVectors[3 * NormalIndex[i] + 2]);
                //����
                Buffer.push_back(TextureCoords[2 * TextureIndex[i] + 0]);
                Buffer.push_back(1.0 - TextureCoords[2 * TextureIndex[i] + 1]);
                //������
                T = glm::cross(B, glm::vec3(NormalVectors[3 * NormalIndex[i] + 0], NormalVectors[3 * NormalIndex[i] + 1], NormalVectors[3 * NormalIndex[i] + 2]));
                Buffer.push_back(T[0]);
                Buffer.push_back(T[1]);
                Buffer.push_back(T[2]);
            }
            //���仺���
            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);
            //���ػ���
            glBindVertexArray(VAO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, Buffer.size() * sizeof(GLfloat), Buffer.data(), GL_STATIC_DRAW);
            //���õ������
            //����
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            //������
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);
            //��������
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
            glEnableVertexAttribArray(2);
            //��������
            glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(8 * sizeof(float)));
            glEnableVertexAttribArray(3);
        }
        else {
            for (int i = 0; i < NumElements; i++) {
                //�������
                Buffer.push_back(VertexCoords[3 * VertexIndex[i] + 0]);
                Buffer.push_back(VertexCoords[3 * VertexIndex[i] + 1]);
                Buffer.push_back(VertexCoords[3 * VertexIndex[i] + 2]);
                //����
                if (TextureIndex[i] == -1) {
                    Buffer.push_back(0.0);
                    Buffer.push_back(0.0);
                }
                else {
                    Buffer.push_back(TextureCoords[2 * TextureIndex[i] + 0]);
                    Buffer.push_back(1.0 - TextureCoords[2 * TextureIndex[i] + 1]);
                }
                //������
                Buffer.push_back(NormalVectors[3 * NormalIndex[i] + 0]);
                Buffer.push_back(NormalVectors[3 * NormalIndex[i] + 1]);
                Buffer.push_back(NormalVectors[3 * NormalIndex[i] + 2]);
            }
            //���仺���
            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);
            //���ػ���
            glBindVertexArray(VAO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, Buffer.size() * sizeof(GLfloat), Buffer.data(), GL_STATIC_DRAW);
            //���õ������
            //����
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
            glEnableVertexAttribArray(0);
            //��������
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);
            //������
            glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(5 * sizeof(float)));
            glEnableVertexAttribArray(2);
        }
    }
    //��������
    ~ObjLoader(void) {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }
    //��
    void Draw(void) {
        glEnable(GL_DEPTH_TEST);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, NumElements);
    }
};