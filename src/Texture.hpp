#pragma once
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <Windows.h>
using namespace std;

#pragma region DDS

#define DDPF_ALPHAPIXELS    0x000001
#define DDPF_ALPHA          0x000002
#define DDPF_FOURCC         0x000004
#define DDPF_RGB            0x000040
#define DDPF_YUV            0x000200
#define DDPF_LUMINANCE      0x020000

#define D3DFMT_DXT1    (('D'<<0)|('X'<<8)|('T'<<16)|('1'<<24))
#define D3DFMT_DXT3    (('D'<<0)|('X'<<8)|('T'<<16)|('3'<<24))
#define D3DFMT_DXT5    (('D'<<0)|('X'<<8)|('T'<<16)|('5'<<24))

typedef struct{
    DWORD    dwSize;
    DWORD    dwFlags;
    DWORD    dwFourCC;
    DWORD    dwRGBBitCount;
    DWORD    dwRBitMask;
    DWORD    dwGBitMask;
    DWORD    dwBBitMask;
    DWORD    dwABitMask;
} DDS_PIXELFORMAT;

#define DDSD_CAPS           0x000001
#define DDSD_HEIGHT         0x000002
#define DDSD_WIDTH          0x000004
#define DDSD_PITCH          0x000008
#define DDSD_PIXELFORMAT    0x001000
#define DDSD_MIPMAPCOUNT    0x020000
#define DDSD_LINEARSIZE     0x080000
#define DDSD_DEPTH          0x800000

typedef struct{
    DWORD            dwSize;
    DWORD            dwFlags;
    DWORD            dwHeight;
    DWORD            dwWidth;
    DWORD            dwPitchOrLinearSize;
    DWORD            dwDepth;
    DWORD            dwMipMapCount;
    DWORD            dwReserved1[11];
    DDS_PIXELFORMAT  ddspf;
    DWORD            dwCaps;
    DWORD            dwCaps2;
    DWORD            dwCaps3;
    DWORD            dwCaps4;
    DWORD            dwReserved2;
} DDS_HEADER;

typedef struct{
    DWORD        dwMagic;
    DDS_HEADER   Header;
} DDS_FILEHEADER;

// For a compressed texture, the size of each mipmap level image is typically one-fourth the size of the previous, with a minimum of 8 (DXT1) or 16 (DXT2-5) bytes (for 
// square textures). Use the following formula to calculate the size of each level for a non-square texture:
#define SIZE_OF_DXT1(width, height)    ( max(1, ( (width + 3) >> 2 ) ) * max(1, ( (height + 3) >> 2 ) ) * 8 )
#define SIZE_OF_DXT2(width, height)    ( max(1, ( (width + 3) >> 2 ) ) * max(1, ( (height + 3) >> 2 ) ) * 16 )

#pragma endregion

void static BindData(GLenum TextureTarget, const string& FileName, bool GenMipMap = true);


//纹理对象
class Texture {
public:
    Texture(GLenum TextureTarget, const string& FileName) : TextureTarget(TextureTarget) {
        //TextureTarget为GL_TEXTURE_1D, GL_TEXTURE_2D, etc.
        //绑定纹理前先要保存之前绑定过的纹理
        GLint LastTextureID;
        if (TextureTarget == GL_TEXTURE_1D) glGetIntegerv(GL_TEXTURE_BINDING_1D, &LastTextureID);
        else if (TextureTarget == GL_TEXTURE_2D) glGetIntegerv(GL_TEXTURE_BINDING_2D, &LastTextureID);
        else throw exception("Invalid Texture Target");
        //分配纹理
        glGenTextures(1, &TextureID);
        glBindTexture(TextureTarget, TextureID);
        //为当前绑定的纹理对象设置环绕、过滤方式
        glTexParameteri(TextureTarget, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(TextureTarget, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(TextureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);//GL_LINEAR_MIPMAP_LINEAR
        glTexParameteri(TextureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        //加载并生成纹理
        BindData(TextureTarget, FileName);
        //恢复之前的纹理绑定
        glBindTexture(TextureTarget, LastTextureID);
    }
    ~Texture(void) {
        glDeleteTextures(1, &TextureID);
    }
    void Bind(GLenum TextureIndex) {
        //TextureIndex: GL_TEXTURE0, GL_TEXTURE1, GL_TEXTURE2, etc.
        glActiveTexture(TextureIndex);
        glBindTexture(TextureTarget, TextureID);
    }
private:
    GLuint TextureID;
    GLuint TextureTarget;
};


//从外部读取并绑定纹理数据
void static BindData(GLenum TextureTarget, const string& FileName, bool GenMipMap) {
    //TextureTarget为GL_TEXTURE_1D, GL_TEXTURE_2D, etc.
    if (FileName.substr(FileName.length() - 3) == "dds") {
        FILE* fp;
        int   size;
        void* pBuffer;
        if ((fp = fopen(FileName.c_str(), "rb")) == NULL)
            throw exception(("Unable to load " + FileName).c_str());
        fseek(fp, 0, SEEK_END);
        size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        pBuffer = malloc(size);
        if (!pBuffer) {
            fclose(fp);
            throw exception("Not enough memory");
        }
        if (fread(pBuffer, size, 1, fp) != 1) {
            free(pBuffer);
            fclose(fp);
            throw exception(("Unable to read " + FileName).c_str());
        }
        fclose(fp);
        // Load DDS to GL texture
        DDS_FILEHEADER* header;
        DWORD           compressFormat;
        GLvoid* data;
        GLsizei         imageSize;
        header = (DDS_FILEHEADER*)pBuffer;
        if (header->dwMagic != 0x20534444)
            throw exception("Bad dds file");
        if (header->Header.dwSize != 124)
            throw exception("Bad header size");
        if (!(header->Header.dwFlags & DDSD_LINEARSIZE))
            throw exception("Bad file type");
        if (!(header->Header.ddspf.dwFlags & DDPF_FOURCC))
            throw exception("Bad pixel format");
        compressFormat = header->Header.ddspf.dwFourCC;
        if (compressFormat != D3DFMT_DXT1 &&
            compressFormat != D3DFMT_DXT3 &&
            compressFormat != D3DFMT_DXT5)
            throw exception("Bad compress format");
        data = (GLvoid*)(header + 1);    // header data skipped
        switch (compressFormat) {
        case D3DFMT_DXT1:
            imageSize = SIZE_OF_DXT1(header->Header.dwWidth, header->Header.dwHeight);
            glCompressedTexImage2DARB(TextureTarget, 0, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, header->Header.dwWidth, header->Header.dwHeight, 0, imageSize, data);
            break;
        case D3DFMT_DXT3:
            imageSize = SIZE_OF_DXT2(header->Header.dwWidth, header->Header.dwHeight);
            glCompressedTexImage2DARB(TextureTarget, 0, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, header->Header.dwWidth, header->Header.dwHeight, 0, imageSize, data);
            break;
        case D3DFMT_DXT5:
            imageSize = SIZE_OF_DXT2(header->Header.dwWidth, header->Header.dwHeight);
            glCompressedTexImage2DARB(TextureTarget, 0, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, header->Header.dwWidth, header->Header.dwHeight, 0, imageSize, data);
            break;
        }
        if (GenMipMap) glGenerateMipmap(TextureTarget);
        free(pBuffer);
    }
    else {
        int width, height, nrChannels;
        unsigned char* data = stbi_load(FileName.c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            if (nrChannels == 4) glTexImage2D(TextureTarget, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            else glTexImage2D(TextureTarget, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            if (GenMipMap) glGenerateMipmap(TextureTarget);
        }
        else
            throw exception(("Unable to load " + FileName).c_str());
        stbi_image_free(data);
    }
}

