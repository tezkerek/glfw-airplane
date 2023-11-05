#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/transform.hpp"
#include "loadShaders.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

GLuint VaoId, VboId, EboId, ProgramId, myMatrixLocation;
GLuint planeTexture, cloudTexture;
glm::mat4 myMatrix, matrRot;

int codCol;
float PI = 3.141592;
float width = 1200, height = 600, xMin = 0.0f, xMax = 1200.0f, yMin = 0.0f, yMax = 600.0f;

float planeWidth = 120.0f, planeHeight = 40.0f;
float planeX = 40.0f, planeY = 40.0f;
float planeOffsetX = 0.0f, planeOffsetY = 0.0f;
float planeSpeedX = 1.0f, planeSpeedY = 3.0f;

float cloudWidth = 190.0f, cloudHeight = 90.0f;
float cloudSpeedX = 0.0f;
std::vector<std::pair<float, float>> cloudsOffset = {{0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}};
std::vector<std::pair<float, float>> cloudsPosition = {{100.0f, 500.0f}, {200.0f, 300.0f}, {500.0f, 400.0f}, {1000, 400}, {700, 200},{300, 800} };

void glfwIdleFunc(GLFWAPI GLFWwindow *_window,
                  void function(GLFWAPI GLFWwindow *_window)) {
    while (!glfwWindowShouldClose(_window)) {
        // Your rendering and update code here
        function(_window);
        // Swap buffers (if you're using double buffering)
        glfwSwapBuffers(_window);

        // Poll for and process events (this is necessary for input handling)
        glfwPollEvents();
    }
}

void LoadTexture(const char *photoPath, GLuint &texture) {
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    //    Desfasurarea imaginii pe orizonatala/verticala in functie de
    //    parametrii de texturare;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int width, height, channels;
    unsigned char *image =
        stbi_load(photoPath, &width, &height, &channels, STBI_rgb_alpha);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA,
                 width,
                 height,
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 image);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(image);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void CreateShaders(void) {
    ProgramId = LoadShaders("1_plane.vert", "1_plane.frag");
    glUseProgram(ProgramId);
}

void CreateVBO(void) {

    // Coordonate;
    // Culori;
    // Coordonate de texturare;

    std::vector<float> Vertices = {
        // Track
        xMin, planeY, 0.0f, 1.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f,
        xMax, planeY, 0.0f, 1.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f,

        // Plane
        planeX, planeY, 0.0f, 1.0f,
        1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, // Stanga jos;
        planeX + planeWidth, planeY, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f,
        1.0f, 0.0f, // Dreapta jos;
        planeX + planeWidth, planeY + planeHeight, 0.0f, 1.0f,
        1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, // Dreapta sus;
        planeX, planeY + planeHeight, 0.0f, 1.0f,
        0.0f, 1.0f, 1.0f,
        0.0f, 1.0f, // Stanga sus;
    };

    std::vector<unsigned int> Indices = {0, 1, 2, 3, 4, 2, 4, 5};
    // Append cloud data
    unsigned int baseNumber = Vertices.size() / 9;
    for (size_t i = 0; i < cloudsPosition.size(); i++)
    {
        Indices.insert(Indices.end(), {baseNumber, baseNumber + 1, baseNumber + 2, baseNumber, baseNumber + 2, baseNumber + 3});
        baseNumber += 4;
    }
    // Append cloud data
    for (auto &&position : cloudsPosition)
    {
        Vertices.insert(Vertices.end(), {position.first, position.second, 0.0f, 1.0f});
        Vertices.insert(Vertices.end(), {1.0f, 0.0f, 0.0f});
        Vertices.insert(Vertices.end(), {0.0f, 0.0f});

        Vertices.insert(Vertices.end(), {position.first + cloudWidth, position.second, 0.0f, 1.0f});
        Vertices.insert(Vertices.end(), {0.0f, 1.0f, 0.0f});
        Vertices.insert(Vertices.end(), {1.0f, 0.0f});

        Vertices.insert(Vertices.end(), {position.first + cloudWidth, position.second + cloudHeight, 0.0f, 1.0f});
        Vertices.insert(Vertices.end(), {1.0f, 1.0f, 0.0f});
        Vertices.insert(Vertices.end(), {1.0f, 1.0f});

        Vertices.insert(Vertices.end(), {position.first, position.second + cloudHeight, 0.0f, 1.0f});
        Vertices.insert(Vertices.end(), {0.0f, 1.0f, 1.0f});
        Vertices.insert(Vertices.end(), {0.0f, 1.0f});
    }
    glGenVertexArrays(1, &VaoId);
    glBindVertexArray(VaoId);

    glGenBuffers(1, &VboId);
    glBindBuffer(GL_ARRAY_BUFFER, VboId);
    std::cout << sizeof(Vertices) << '\n';
    std::cout << sizeof(float) << '\n';
    glBufferData(GL_ARRAY_BUFFER, Vertices.size() * sizeof(float), Vertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &EboId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EboId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 sizeof(unsigned int) * Indices.size(),
                 Indices.data(),
                 GL_STATIC_DRAW);
    //  Se asociaza atributul (0 = coordonate) pentru shader;
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,
                          4,
                          GL_FLOAT,
                          GL_FALSE,
                          9 * sizeof(GLfloat),
                          (GLvoid *)0);
    //  Se asociaza atributul (1 = culoare) pentru shader;
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          9 * sizeof(GLfloat),
                          (GLvoid *)(4 * sizeof(GLfloat)));
    //  Se asociaza atributul (2 = texturare) pentru shader;
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2,
                          2,
                          GL_FLOAT,
                          GL_FALSE,
                          9 * sizeof(GLfloat),
                          (GLvoid *)(7 * sizeof(GLfloat)));
}

//  Elimina obiectele de tip shader dupa rulare;
void DestroyShaders(void) { glDeleteProgram(ProgramId); }

//  Eliminarea obiectelor de tip VBO dupa rulare;
void DestroyVBO(void) {
    //  Eliberarea atributelor din shadere (pozitie, culoare, texturare etc.);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);

    //  Stergerea bufferelor pentru VARFURI(Coordonate + Culori), INDICI;
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &VboId);
    glDeleteBuffers(1, &EboId);

    //  Eliberaea obiectelor de tip VAO;
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &VaoId);
}

//  Functia de eliberare a resurselor alocate de program;
void Cleanup(void) {
    DestroyShaders();
    DestroyVBO();
}

//  Setarea parametrilor necesari pentru fereastra de vizualizare;
void Initialize(void) {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    CreateShaders();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    myMatrixLocation = glGetUniformLocation(ProgramId, "myMatrix");
    glUniform1i(glGetUniformLocation(ProgramId, "myTexture"), 0);

    LoadTexture("plane.png", planeTexture);
    LoadTexture("cloud.png", cloudTexture);
}

void updatePlane() {
    if (planeOffsetX <= 100.0f)
    {
        planeSpeedX = 2.0f;
        planeSpeedY = 0.0f;
    }
    else if (planeOffsetX > 100.0f && planeOffsetX <= 300.0f)
    {
        planeSpeedX = 2.0f;
        planeSpeedY = 2.0f;
    }
    else if (planeOffsetX > 300.0f && planeOffsetX <= 700.0f)
    {
        planeSpeedX = 0.3f;
        planeSpeedY = 0.0f;
        cloudSpeedX = -0.7f;
    }
    else if (planeOffsetX > 700.0f && planeOffsetX <= 900.0f)
    {
        planeSpeedX = 2.0f;
        planeSpeedY = -2.0f;
    }
    else if (planeOffsetX > 1000.0f)
    {
        planeSpeedX = 0;
        cloudSpeedX = 0;
    }

    planeOffsetX += planeSpeedX;
    planeOffsetY = std::max(planeOffsetY + planeSpeedY, 0.0f);

    for (auto &&offset : cloudsOffset)
    {
        offset.first += cloudSpeedX;
    }
}

//  Functia de desenarea a graficii pe ecran;
void RenderFunction(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    CreateVBO();

    glm::mat4 resizeM = glm::ortho(xMin, xMax, yMin, yMax);

    GLint toggleTextureLocation = glGetUniformLocation(ProgramId, "toggleTexture");
    glUniform1i(glGetUniformLocation(ProgramId, "myTexture"), 0);

    // Draw clouds
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, cloudTexture);
    glUniform1i(toggleTextureLocation, 1);

    for (size_t i = 0; i < cloudsOffset.size(); i++)
    {
        glm::mat4 cloudTranslM = glm::translate(glm::vec3(cloudsOffset[i].first, cloudsOffset[i].second, 0.0f));
        myMatrix = resizeM * cloudTranslM;
        glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (const void *)((i * 6 + 8) * sizeof(GLuint)));
    }

    // Draw track
    myMatrix = resizeM;
    glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);

    glUniform1i(toggleTextureLocation, 0);

    glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, 0);

    // Draw plane
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, planeTexture);
    glUniform1i(toggleTextureLocation, 1);

    glm::mat4 planeTranslM = glm::translate(glm::vec3(planeOffsetX, planeOffsetY, 0.0f));
    myMatrix = resizeM * planeTranslM;
    glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (const void *)(2 * sizeof(GLuint)));

    glFlush();
}

int main() {
    GLenum res = glfwInit();
    if (res == GLFW_FALSE) {
        std::cerr << "GLFW initialization failed\n";
        exit(1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    auto _window = glfwCreateWindow(width, height, "Plane", nullptr, nullptr);

    if (_window == nullptr) {
        std::cerr << "Failed to create GLFW window \n";
        glfwTerminate();
        exit(-1);
    }
    glfwMakeContextCurrent(_window);

    glewInit();

    Initialize();

    while (!glfwWindowShouldClose(_window)) {
        updatePlane();
        RenderFunction();
        glfwSwapBuffers(_window);
        glfwPollEvents();
    }
    Cleanup();
    glfwTerminate();
    return 0;
}