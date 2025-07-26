#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <windows.h>

#include <GL/glew.h>
#include <glfw3.h>
GLFWwindow* window;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
using namespace glm;

#include "shader.hpp"
#include "../obj_loader.hpp"

std::string VertexShader = R"(#version 330 core
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec3 VertColor;

uniform mat4 MVP;
uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;

out vec3 FragPos;
out vec3 Normal;
out vec3 Color;

void main(){
    FragPos = vec3(ModelMatrix * vec4(vertexPosition_modelspace, 1.0));
    Normal = mat3(transpose(inverse(ModelMatrix))) * vertexNormal;
    gl_Position = MVP * vec4(vertexPosition_modelspace,1);
    Color = VertColor;
})";

std::string FragmentShader = R"(#version 330 core
in vec3 FragPos;
in vec3 Normal;
in vec3 Color;

out vec4 FragColor;

void main() {
    vec3 lightPos = vec3(0, 20, 0);
    vec3 lightColor = vec3(0.4);

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 ambient = 1 * Color;
    vec3 result = ambient + diffuse * Color;
    FragColor = vec4(result, 1);
})";



// Object class
class Object {
private:
    GLuint VAO, VBO, EBO;
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
public:
    glm::mat4 Model;
    glm::vec3 Color;

    Object() {
        glm::mat4 Model = glm::mat4(1.0f);

        // init buffers
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
    }
    void LoadModel(std::string path) {
        ParseOBJ(path, &vertices, &indices);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);
    }
    void Render(GLuint MatrixID, GLuint ModelID, GLuint ViewID, glm::mat4 View, glm::mat4 Projection) {
        glm::mat4 MVP = Projection * View * Model;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelID, 1, GL_FALSE, &Model[0][0]);
        glUniformMatrix4fv(ViewID, 1, GL_FALSE, &View[0][0]);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
};

std::string GetExecutableDirectory() {
    char buffer[512];
    GetModuleFileNameA(NULL, buffer, 512);
    std::string fullPath(buffer);
    return fullPath.substr(0, fullPath.find_last_of("\\/"));
}

std::string GetFileInExecutableDir(const std::string& filename) {
    return GetExecutableDirectory() + "\\" + filename;
}

int main() {
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(1024, 768, "OpenGL", NULL, NULL);
    if (!window) return -1;
    glfwMakeContextCurrent(window);

    glewExperimental = true;
    if (glewInit() != GLEW_OK) return -1;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
    glfwSwapInterval(1);

    GLuint programID = LoadShaders(VertexShader, FragmentShader);

    GLuint MatrixID = glGetUniformLocation(programID, "MVP");
    GLuint ModelID = glGetUniformLocation(programID, "ModelMatrix");
    GLuint ViewID = glGetUniformLocation(programID, "ViewMatrix");

    glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);

    glm::mat4 View = glm::lookAt(glm::vec3(-4, 5, -10), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

    Object cube = Object();
    cube.LoadModel(GetFileInExecutableDir("model.obj"));

    Object floor = Object();
    floor.LoadModel(GetFileInExecutableDir("floor.obj"));
    floor.Model = glm::scale(glm::mat4(1.0f), glm::vec3(3, 1, 3));

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(programID);

        floor.Render(MatrixID, ModelID, ViewID, View, Projection);
        cube.Render(MatrixID, ModelID, ViewID, View, Projection);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
