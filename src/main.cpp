#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.h"
#include "Model.h"
#include <iostream>
#include <fstream>
#include "StreetLamp.h"
#include "CarHeadlight.h"
#include "Camera.h"
#include "Renderer.h"

// Rozmiar okna
const unsigned int SCR_WIDTH = 1300;
const unsigned int SCR_HEIGHT = 900;

enum CameraMode { DEFAULT, TOP, FOLLOW };
CameraMode activeCamera = DEFAULT;
Camera topCamera(glm::vec3(0.0f, 20.0f, 10.0f));
Camera followCamera(glm::vec3(0.0f, 2.0f, 5.0f));
Camera camera(glm::vec3(0.0f, 2.0f, 10.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
glm::vec3 carPosition = glm::vec3(55.0f, -1.78f, 1.5f);
float carRotation = -90.0f;
enum CarState { MOVING_FORWARD, TURNING_RIGHT, MOVING_FORWARD_AFTER_TURN };
CarState carState = MOVING_FORWARD;
bool isNight = false;
glm::vec3 headlightDirection = glm::vec3(0.0f, -0.3f, 1.0f);
float headlightIntensity = 0.5f;
bool usePhongShading = true;


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window, float deltaTime);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void updateCarMovement(float deltaTime);

int main()
{
    GLFWwindow* window = Renderer::Initialize();
    if (!window) return -1;


    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    Shader shader("shaders/vertex_shader.glsl", "shaders/fragment_shader.glsl");
    Model carmodel("models/car/scene.gltf");
    Model cityModel("models/city/scene.gltf");
    Model sphere("models/sphere/scene.gltf");

    StreetLamp streetLamp(glm::vec3(-5.7f, 2.3f, 5.4f),
        glm::vec3(0.0f, -1.0f, 0.0f),
        glm::vec3(1.0f, 0.8f, 0.6f),
        5.0f,
        glm::cos(glm::radians(35.0f)),
        glm::cos(glm::radians(35.5f)),
        3.0);

    CarHeadlight leftHeadlight(
        glm::vec3(carPosition.x - 0.4f, carPosition.y + 0.2f, carPosition.z + 1.0f),
        glm::vec3(0.0f, -0.2f, 1.0f),
        glm::vec3(0.9f, 0.85f, 0.7f),
        0.5f,
        glm::cos(glm::radians(16.0f)),
        glm::cos(glm::radians(22.0f)),
        4.0f
    );
    CarHeadlight rightHeadlight(
        glm::vec3(carPosition.x + 0.4f, carPosition.y + 0.2f, carPosition.z + 1.0f),
        glm::vec3(0.0f, -0.2f, 1.0f),
        glm::vec3(0.9f, 0.85f, 0.7f),
        0.5f,
        glm::cos(glm::radians(16.0f)),
        glm::cos(glm::radians(22.0f)),
        4.0f
    );

    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000000.0f);

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
        shader.setInt("shadingMode", usePhongShading ? 1 : 0);
        shader.setMat4("projection", projection);

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

        // latarina
        if (isNight) {
            shader.setVec3("streetLight.position", streetLamp.position);
            shader.setVec3("streetLight.direction", streetLamp.direction);
            shader.setVec3("streetLight.color", streetLamp.color * streetLamp.intensity);
            shader.setFloat("streetLight.cutoff", streetLamp.cutoff);
            shader.setFloat("streetLight.outerCutoff", streetLamp.outerCutoff);
            shader.setFloat("streetLight.radius", streetLamp.radius);
        }
        else {
            shader.setVec3("streetLight.color", glm::vec3(0.0f));
        }

        // reflektory

        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(carRotation), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::vec3 transformedHeadlightDirection = glm::vec3(rotationMatrix * glm::vec4(headlightDirection, 0.0f));

        glm::vec3 headlightOffsetLeft = glm::vec3(1.2f, 0.3f, -5.2f);
        glm::vec3 headlightOffsetRight = glm::vec3(2.2f, 0.3f, -5.2f);
        leftHeadlight.position = carPosition + glm::vec3(rotationMatrix * glm::vec4(headlightOffsetLeft, 1.0f));
        rightHeadlight.position = carPosition + glm::vec3(rotationMatrix * glm::vec4(headlightOffsetRight, 1.0f));

        leftHeadlight.direction = glm::normalize(transformedHeadlightDirection);
        rightHeadlight.direction = glm::normalize(transformedHeadlightDirection);
        leftHeadlight.intensity = headlightIntensity;
        rightHeadlight.intensity = headlightIntensity;

        shader.setVec3("headlightLeft.position", leftHeadlight.position);
        shader.setVec3("headlightLeft.direction", leftHeadlight.direction);
        shader.setVec3("headlightLeft.color", leftHeadlight.color* leftHeadlight.intensity);
        shader.setFloat("headlightLeft.cutoff", leftHeadlight.cutoff);
        shader.setFloat("headlightLeft.outerCutoff", leftHeadlight.outerCutoff);
        shader.setFloat("headlightLeft.radius", leftHeadlight.radius);

        shader.setVec3("headlightRight.position", rightHeadlight.position);
        shader.setVec3("headlightRight.direction", rightHeadlight.direction);
        shader.setVec3("headlightRight.color", rightHeadlight.color* rightHeadlight.intensity);
        shader.setFloat("headlightRight.cutoff", rightHeadlight.cutoff);
        shader.setFloat("headlightRight.outerCutoff", rightHeadlight.outerCutoff);
        shader.setFloat("headlightRight.radius", rightHeadlight.radius);


        // Kamera
        glm::mat4 view;
        if (activeCamera == TOP) {
            glm::vec3 topViewPosition = glm::vec3(-68.0f, 12.0f, -11.0f);
            glm::vec3 upDirection(0.0f, 1.0f, 0.0f);

            view = glm::lookAt(topViewPosition, carPosition, upDirection);
            shader.setVec3("viewPos", topViewPosition);
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

            shader.setVec3("viewPos", followViewPosition);
        }
        else {
            view = camera.GetViewMatrix();
            shader.setVec3("viewPos", camera.Position);
        }
        shader.setMat4("view", view);

        // mgła
        if (isNight) {
            shader.setFloat("fogDensity", 0.035f);
            shader.setVec3("fogColor", glm::vec3(0.1f, 0.1f, 0.2f));
        }
        else {
            shader.setFloat("fogDensity", 0.02f);
            shader.setVec3("fogColor", glm::vec3(0.6f, 0.7f, 0.8f));
        }
        
        // Miasto
        glm::mat4 cityModelMat = glm::mat4(1.0f);
        cityModelMat = glm::translate(cityModelMat, glm::vec3(0.0f, -2.0f, 0.0f));
        cityModelMat = glm::rotate(cityModelMat, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        shader.setMat4("model", cityModelMat);
        cityModel.Draw(shader);

        // Kula
        glm::mat4 sphereModelMat = glm::mat4(1.0f);
        sphereModelMat = glm::translate(sphereModelMat, glm::vec3(0.0f, 5.0f, 0.0f));
        sphereModelMat = glm::scale(sphereModelMat, glm::vec3(1.5f, 1.5f, 1.5f));
        sphereModelMat = glm::scale(sphereModelMat, glm::vec3(0.1f, 0.1f, 0.1f));
        shader.setMat4("model", sphereModelMat);
        sphere.Draw(shader);

        // Samochód
        glm::mat4 carModelMat = glm::mat4(1.0f);
        carModelMat = glm::translate(carModelMat, carPosition);
        carModelMat = glm::scale(carModelMat, glm::vec3(0.1f, 0.1f, 0.1f));
        carModelMat = glm::rotate(carModelMat, glm::radians(carRotation), glm::vec3(0.0f, 1.0f, 0.0f));
        shader.setMat4("model", carModelMat);
        carmodel.Draw(shader);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

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
    static bool keyGPPressed = false;

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
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.ProcessRoll(-1.0f);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.ProcessRoll(1.0f);

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && !keyPressed) {
        activeCamera = DEFAULT;
        keyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && !keyPressed) {
        activeCamera = TOP;
        keyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS && !keyPressed) {
        activeCamera = FOLLOW;
        keyPressed = true;
    }

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_RELEASE &&
        glfwGetKey(window, GLFW_KEY_2) == GLFW_RELEASE &&
        glfwGetKey(window, GLFW_KEY_3) == GLFW_RELEASE) {
        keyPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        headlightDirection.y += 0.005f;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        headlightDirection.y -= 0.005f;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        headlightDirection.x -= 0.005f;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        headlightDirection.x += 0.005f;

    if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS) 
        headlightIntensity = glm::min(1.0f, headlightIntensity + 0.005f);
    if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS)
        headlightIntensity = glm::max(0.0f, headlightIntensity - 0.005f);
    

    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS && !keyNPPressed) {
        isNight = !isNight;
        keyNPPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_RELEASE) {
        keyNPPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS && !keyGPPressed) {
        usePhongShading = !usePhongShading;
        keyGPPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_RELEASE) {
        keyGPPressed = false;
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