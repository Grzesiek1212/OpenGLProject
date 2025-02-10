#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shaders/Shader.h"
#include "Model.h"
#include <iostream>
#include <fstream>
#include "Camera.h"
#include "StreetLamp.h"

// Rozmiar okna
const unsigned int SCR_WIDTH = 1300;
const unsigned int SCR_HEIGHT = 900;

// Kamera
enum CameraMode { DEFAULT, TOP, FOLLOW };
CameraMode activeCamera = DEFAULT;

Camera topCamera(glm::vec3(0.0f, 20.0f, 10.0f));
Camera followCamera(glm::vec3(0.0f, 2.0f, 5.0f));
Camera camera(glm::vec3(0.0f, 2.0f, 10.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
glm::vec3 carPosition = glm::vec3(-30.0f, -1.78f, 1.5f);
float carRotation = -90.0f;
enum CarState { MOVING_FORWARD, TURNING_RIGHT, MOVING_FORWARD_AFTER_TURN };
CarState carState = MOVING_FORWARD;
bool isNight = false;


// Obsługa wejścia
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window, float deltaTime);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void updateCarMovement(float deltaTime);

int main()
{
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
    Model sphere("models/sphere/scene.gltf");

    // Macierz projekcji
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    StreetLamp streetLamp(glm::vec3(-5.7f, 2.3f, 5.4f), // Pozycja
        glm::vec3(0.0f, -1.0f, 0.0f), // Kierunek
        glm::vec3(1.0f, 0.8f, 0.6f),  // Kolor
        3.0f,                          // Intensywność
        glm::cos(glm::radians(35.0f)), // Wewnętrzny kąt
        glm::cos(glm::radians(35.5f)), // Zewnętrzny kąt
        7.5f); // NOWE: Promień wpływu światła


    // Główna pętla renderująca
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window, deltaTime);

        updateCarMovement(deltaTime);

        isNight ? glClearColor(0.02f, 0.02f, 0.1f, 1.0f) : glClearColor(0.6f, 0.8f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();

        if (isNight) {
            glm::vec3 lightColor = glm::vec3(0.2f, 0.2f, 0.5f);
            glm::vec3 ambientColor = glm::vec3(0.05f, 0.05f, 0.1f);
            glm::vec3 lightDirection = glm::vec3(0.1f, -1.0f, 0.2f);
            shader.setVec3("lightDir", lightDirection);
            shader.setVec3("lightColor", lightColor);
            shader.setVec3("ambientColor", ambientColor);
        }
        else {
            glm::vec3 lightColor = glm::vec3(1.2f, 1.1f, 0.9f);
            glm::vec3 ambientColor = glm::vec3(0.5f, 0.5f, 0.5f);
            glm::vec3 lightDirection = glm::vec3(-0.2f, -1.0f, -0.3f);

            shader.setVec3("lightDir", lightDirection);
            shader.setVec3("lightColor", lightColor);
            shader.setVec3("ambientColor", ambientColor);
        }

        if (isNight) {
            shader.setVec3("streetLight.position", streetLamp.position);
            shader.setVec3("streetLight.direction", streetLamp.direction);
            shader.setVec3("streetLight.color", streetLamp.color * streetLamp.intensity);
            shader.setFloat("streetLight.cutoff", streetLamp.cutoff);
            shader.setFloat("streetLight.outerCutoff", streetLamp.outerCutoff);
            shader.setFloat("streetLight.radius", streetLamp.radius);
        }
        else {
            shader.setVec3("streetLight.color", glm::vec3(0.0f)); // Wyłączenie latarni w dzień
        }

        glm::mat4 view;

        if (activeCamera == TOP) {
            glm::vec3 topViewPosition = glm::vec3(-68.0f, 12.0f, -11.0f);
            glm::vec3 upDirection(0.0f, 1.0f, 0.0f);

            view = glm::lookAt(topViewPosition, carPosition, upDirection);
        }

        else if (activeCamera == FOLLOW) {
            glm::vec3 defaultOffset(13.0f, 2.0f, 1.8f);
            float deltaAngle = glm::radians(carRotation - (-90.0f));
            glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), deltaAngle, glm::vec3(0.0f, 1.0f, 0.0f));
            glm::vec3 rotatedOffset = glm::vec3(rotationMatrix * glm::vec4(defaultOffset, 1.0f));
            glm::vec3 followViewPosition = carPosition + rotatedOffset;
            glm::vec3 upDirection(0.0f, 1.0f, 0.0f);
            view = glm::lookAt(followViewPosition, carPosition, upDirection);
            followCamera.Position = followViewPosition;
            followCamera.Up = upDirection;
        }
        else {
            // Domyśln kamera
            view = camera.GetViewMatrix();
        }

        shader.setMat4("view", view);
        shader.setVec3("viewPos", camera.Position);

        shader.setMat4("projection", projection);

        // Macierz dla miasta - poprawna pozycja
        glm::mat4 cityModelMat = glm::mat4(1.0f);
        cityModelMat = glm::translate(cityModelMat, glm::vec3(0.0f, -2.0f, 0.0f)); // Przesunięcie w dół
        cityModelMat = glm::rotate(cityModelMat, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Obrót poprawiony
        shader.setMat4("model", cityModelMat);
        cityModel.Draw(shader);

        // Kula
        glm::mat4 sphereModelMat = glm::mat4(1.0f);
        sphereModelMat = glm::translate(sphereModelMat, glm::vec3(0.0f, 5.0f, 0.0f));
        sphereModelMat = glm::scale(sphereModelMat, glm::vec3(1.5f, 1.5f, 1.5f));
        sphereModelMat = glm::scale(sphereModelMat, glm::vec3(0.1f, 0.1f, 0.1f));
        shader.setMat4("model", sphereModelMat);
        sphere.Draw(shader);


        // Rysowanie samochodu
        glm::mat4 carModelMat = glm::mat4(1.0f);
        carModelMat = glm::translate(carModelMat, carPosition);
        carModelMat = glm::scale(carModelMat, glm::vec3(0.1f, 0.1f, 0.1f));
        carModelMat = glm::rotate(carModelMat, glm::radians(carRotation), glm::vec3(0.0f, 1.0f, 0.0f));

        shader.setMat4("model", carModelMat);
        carmodel.Draw(shader);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Zamykanie aplikacji
    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window, float deltaTime)
{
    static bool keyPressed = false;
    static bool keyNPPressed = false;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.ProcessRoll(-1.0f);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.ProcessRoll(1.0f);

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && !keyPressed) {
        activeCamera = DEFAULT;
        keyPressed = true;
        std::cout << "Active Camera: DEFAULT" << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && !keyPressed) {
        activeCamera = TOP;
        keyPressed = true;
        std::cout << "Active Camera: TOP" << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS && !keyPressed) {
        activeCamera = FOLLOW;
        keyPressed = true;
        std::cout << "Active Camera: FOLLOW" << std::endl;
    }

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_RELEASE &&
        glfwGetKey(window, GLFW_KEY_2) == GLFW_RELEASE &&
        glfwGetKey(window, GLFW_KEY_3) == GLFW_RELEASE) {
        keyPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS && !keyNPPressed) {
        isNight = !isNight; // Przełącz tryb
        keyNPPressed = true; // Zablokowanie zmiany do momentu puszczenia klawisza
        std::cout << "Tryb: " << (isNight ? "Noc" : "Dzień") << std::endl;
    }

    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_RELEASE) {
        keyNPPressed = false; // Reset flagi po puszczeniu klawisza
    }
}

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

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

void updateCarMovement(float deltaTime) {
    static float turnProgress = 0.0f; 
    static float currentSpeed = 6.0f;
    float normalSpeed = 6.0f;
    float turnSpeed = 2.0f;
    float speedChangeRate = 3.0f;

    float turnStartX = -63.5f; 
    float turnEndX = -64.0f; 
    float startZ = 1.5f; 
    float endZ = 11.0f; 

    switch (carState) {
    case MOVING_FORWARD:
        if (carPosition.x > turnStartX) {
            carPosition.x -= currentSpeed * deltaTime; 
        }
        else {
            carState = TURNING_RIGHT;
            turnProgress = 0.0f;
        }
        break;

    case TURNING_RIGHT:
        if (turnProgress < 1.0f) {
            currentSpeed = glm::mix(normalSpeed, turnSpeed, turnProgress);

            float t = turnProgress;
            float arcOffset = sin(t * glm::pi<float>()) * 1.0f;
            carPosition.x = glm::mix(turnStartX, turnEndX, t);
            carPosition.z = glm::mix(startZ, endZ, t) + arcOffset;
            carRotation = glm::mix(-90.0f, 0.0f, t);

            turnProgress += (currentSpeed / normalSpeed) * deltaTime * 0.7f;
        }
        else {
            carPosition.x = turnEndX;
            carPosition.z = endZ; 
            carState = MOVING_FORWARD_AFTER_TURN;
            currentSpeed = turnSpeed;
        }
        break;

    case MOVING_FORWARD_AFTER_TURN:
        currentSpeed = glm::mix(currentSpeed, normalSpeed, deltaTime * speedChangeRate);
        carPosition.z += currentSpeed * deltaTime;

        if (carPosition.z > 48.0f) { 
            carPosition = glm::vec3(55.0f, -1.78f, 2.0f);
            carRotation = -90.0f;
            carState = MOVING_FORWARD;
            currentSpeed = normalSpeed;
        }
        break;
    }
}



