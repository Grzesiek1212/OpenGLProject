#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shaders/Shader.h"
#include "Model.h"
#include <iostream>
#include <fstream>
#include "Camera.h"  // Dodajemy obsługę kamery
#include "Sphere.h"

// Rozmiar okna
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// Kamera
enum CameraMode { DEFAULT, TOP, FOLLOW };
CameraMode activeCamera = DEFAULT; // Domyślna kamera

Camera topCamera(glm::vec3(0.0f, 20.0f, 10.0f));  // Kamera nad miastem
Camera followCamera(glm::vec3(0.0f, 2.0f, 5.0f)); // Kamera za samochodem
Camera camera(glm::vec3(0.0f, 2.0f, 10.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
float carX = 55.0f; // Startowa pozycja X samochodu

// Obsługa wejścia
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window, float deltaTime);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

int main()
{
    // Inicjalizacja GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Model Loader", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Obsługa myszy
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Ukrycie kursora

    // Inicjalizacja GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Włącz test głębokości
    glEnable(GL_DEPTH_TEST);

    // Załadowanie shaderów
    Shader shader("shaders/vertex_shader.glsl", "shaders/fragment_shader.glsl");

    // Wczytanie modeli
    Model carmodel("models/car/scene.gltf");
    Model cityModel("models/city/scene.gltf");
    Sphere sphere(1.0f);

    // Macierz projekcji
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    // Główna pętla renderująca
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window, deltaTime);

        // Czyszczenie ekranu
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();

        // Pobranie aktualnej macierzy widoku z kamery
        glm::mat4 view = camera.GetViewMatrix();
        shader.setMat4("view", view);


        shader.setMat4("projection", projection);

        // Macierz dla miasta - poprawna pozycja
        glm::mat4 cityModelMat = glm::mat4(1.0f);
        cityModelMat = glm::translate(cityModelMat, glm::vec3(0.0f, -2.0f, 0.0f)); // Przesunięcie w dół
        cityModelMat = glm::rotate(cityModelMat, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Obrót poprawiony
        shader.setMat4("model", cityModelMat);
        cityModel.Draw(shader);

        shader.setBool("isSphere", true);
        glm::mat4 sphereModelMat = glm::mat4(1.0f);
        sphereModelMat = glm::translate(sphereModelMat, glm::vec3(0.0f, 5.0f, 0.0f));
        sphereModelMat = glm::scale(sphereModelMat, glm::vec3(1.5f, 1.5f, 1.5f));
        shader.setMat4("model", sphereModelMat);
        sphere.Draw(shader);
        shader.setBool("isSphere", false);


        if (carX >= -70.0f)
            carX -= deltaTime * 6.0f; // Prędkość ruchu samochodu
        else
            carX = 55.0f;

        glm::mat4 carModelMat = glm::mat4(1.0f);
        carModelMat = glm::translate(carModelMat, glm::vec3(carX, -1.78f, 2.0f));
        carModelMat = glm::scale(carModelMat, glm::vec3(0.1f, 0.1f, 0.1f));
        carModelMat = glm::rotate(carModelMat, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        shader.setMat4("model", carModelMat);
        carmodel.Draw(shader);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Zamykanie aplikacji
    glfwTerminate();
    return 0;
}

// Obsługa zmiany rozmiaru okna
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// Obsługa wejścia z klawiatury
void processInput(GLFWwindow* window, float deltaTime)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Ruch w przód, tył, lewo, prawo
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    // Obsługa roll (obrót wokół osi Z)
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.ProcessRoll(-1.0f); // Obrót w lewo
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.ProcessRoll(1.0f); // Obrót w prawo

    // Obsługa ruchu w górę i w dół
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
        activeCamera = DEFAULT;
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
        activeCamera = TOP;
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
        activeCamera = FOLLOW;
}

// Obsługa ruchu myszką
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// Obsługa scrolla myszy
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}
