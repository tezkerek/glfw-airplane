#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/transform.hpp"
#include "loadShaders.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

GLuint VaoId, VboId, EboId, ProgramId, myMatrixLocation, viewLocation,
    projLocation, matrRotlLocation, codColLocation;
GLuint texture;
glm::mat4 myMatrix, matrRot;

int codCol;
float PI = 3.141592;
float width = 500, height = 500, xMin = -200.f, xMax = 200.0f, yMin = -200.0f,
      yMax = 200.0f;

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

void LoadTexture(const char *photoPath) {
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

    GLfloat Vertices[] = {
        // Coordonate;                  Culori;              Coordonate de
        // texturare;
        -70.0f, -10.0f, 0.0f, 1.0f, 1.0f,
        0.0f,   0.0f,   0.0f, 0.0f, // Stanga jos;
        70.0f,  -10.0f, 0.0f, 1.0f, 0.0f,
        1.0f,   0.0f,   1.0f, 0.0f, // Dreapta jos;
        70.0f,  10.0f,  0.0f, 1.0f, 1.0f,
        1.0f,   0.0f,   1.0f, 1.0f, // Dreapta sus;
        -70.0f, 10.0f,  0.0f, 1.0f, 0.0f,
        1.0f,   1.0f,   0.0f, 1.0f // Stanga sus;
    };

    GLuint Indices[] = {0, 1, 2, 0, 2, 3};

    glGenVertexArrays(1, &VaoId);
    glBindVertexArray(VaoId);

    glGenBuffers(1, &VboId);
    glBindBuffer(GL_ARRAY_BUFFER, VboId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &EboId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EboId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 sizeof(Indices),
                 Indices,
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
}

//  Functia de desenarea a graficii pe ecran;
void RenderFunction(void) {
    glClear(GL_COLOR_BUFFER_BIT);

    glm::mat4 resizeM = glm::ortho(xMin, xMax, yMin, yMax);
    myMatrix = resizeM;

    CreateVBO();

    LoadTexture("plane.png");
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);

    glUniform1i(glGetUniformLocation(ProgramId, "myTexture"), 0);
    glUniform1i(glGetUniformLocation(ProgramId, "toggleTexture"), 1);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glFlush();
}

int main(int argc, char *argv[]) {
    GLenum res = glfwInit();
    if (res == GLFW_FALSE) {
        std::cerr << "GLFW initialization failed\n";
        exit(1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    auto _window = glfwCreateWindow(500, 500, "Lab1", nullptr, nullptr);

    if (_window == nullptr) {
        std::cerr << "Failed to create GLFW window \n";
        glfwTerminate();
        exit(-1);
    }
    glfwMakeContextCurrent(_window);

    glewInit();

    Initialize();

    while (!glfwWindowShouldClose(_window)) {
        RenderFunction();
        glfwSwapBuffers(_window);
        glfwPollEvents();
    }
    Cleanup();
    glfwTerminate();
    return 0;
}