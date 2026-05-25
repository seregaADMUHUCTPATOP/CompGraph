#define GLEW_DLL
#define GLFW_DLL
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "GpuProgram.h"
#include "Model.h" 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Позиция камеры настроена для удобного обзора башни сверху-спереди
glm::vec3 cameraPos = glm::vec3(1.1f, 2.7f, 5.5f);
glm::vec3 cameraFront = glm::vec3(0.0f, -0.25f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

bool firstMouse = true;
float yaw = -90.0f, pitch = 0.0f, lastX = 400, lastY = 300, fov = 45.0f;
float deltaTime = 0.0f, lastFrame = 0.0f;

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos; lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity; yoffset *= sensitivity;
    yaw += xoffset; pitch += yoffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

glm::mat4 OX1_Pos = glm::mat4(1.0f);
glm::mat4 OX2_Pos = glm::mat4(1.0f);
glm::mat4 OX3_Pos = glm::mat4(1.0f);

float OX1_Angle = 0.0f;
float OX2_Translation = 0.0f;
float OX3_Translation = 0.0f;

void processInput(GLFWwindow* window) {
    float cameraSpeed = 2.5f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);

    // 1. Поворот башни вокруг своей оси (N / M)
    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
        OX1_Angle -= deltaTime * 45.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
        OX1_Angle += deltaTime * 45.0f;
    }

    // 2. Движение дула по вертикали (K / L)
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
        OX2_Translation += deltaTime * 0.3f;
    }
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
        OX2_Translation -= deltaTime * 0.3f;
    }
    // Сохраняем асимметричное ограничение, чтобы дуло не врезалось вниз в корпус танка
    if (OX2_Translation < -0.02f) OX2_Translation = -0.02f;
    if (OX2_Translation > 0.25f)  OX2_Translation = 0.25f;

    // 3. Движение пушек поршня вперед-назад (O / P)
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
        OX3_Translation += deltaTime * 0.3f;
    }
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
        OX3_Translation -= deltaTime * 0.3f;
    }
    // Сохраняем минимальный откат назад (-0.05f), чтобы пушки не въезжали внутрь башни
    if (OX3_Translation < -0.05f) OX3_Translation = -0.05f;
    if (OX3_Translation > 0.20f)  OX3_Translation = 0.20f;

    // Сборка матриц трансформаций
    OX1_Pos = glm::rotate(glm::mat4(1.0f), glm::radians(OX1_Angle), glm::vec3(0.0f, 1.0f, 0.0f));
    OX2_Pos = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, OX2_Translation, 0.0f));
    OX3_Pos = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, OX3_Translation));
}

int main() {
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Lab 7 - Hierarchical Transforms", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) return -1;
    glEnable(GL_DEPTH_TEST);

    GpuProgram shader("vertex.vert", "fragment.frag");
    Model ourModel("resources/models/Lab_3_VAR_14.obj");

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.Use();

        // Передача параметров освещения
        glm::vec3 lightPos(4.0f, 5.0f, 6.0f);
        shader.SetUniform("light.position", lightPos.x, lightPos.y, lightPos.z);
        shader.SetUniform("viewPos", cameraPos.x, cameraPos.y, cameraPos.z);

        shader.SetUniform("light.ambient", 0.3f, 0.3f, 0.3f);
        shader.SetUniform("light.diffuse", 0.8f, 0.8f, 0.8f);
        shader.SetUniform("light.specular", 1.0f, 1.0f, 1.0f);

        shader.SetUniform("material.ambient", 0.1f, 0.4f, 0.2f);
        shader.SetUniform("material.diffuse", 0.2f, 0.6f, 0.3f);
        shader.SetUniform("material.specular", 0.4f, 0.4f, 0.4f);
        shader.SetUniform("material.shininess", 32.0f);

        // Матрицы камеры и проекции
        glm::mat4 projection = glm::perspective(glm::radians(fov), 800.0f / 600.0f, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        shader.SetUniform("projection", glm::value_ptr(projection));
        shader.SetUniform("view", glm::value_ptr(view));

        // Отрисовка иерархической модели (без лишних юниформов "transform" и "objectColor")
        ourModel.Draw(shader, OX1_Pos, OX2_Pos, OX3_Pos);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}