#include <chrono>
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <SFML/Audio/SoundBuffer.hpp>
#include <utilities/shader.hpp>
#include <glm/vec3.hpp>
#include <iostream>
#include <utilities/timeutils.h>
#include <utilities/mesh.h>
#include <utilities/shapes.h>
#include <utilities/glutils.h>
#include <SFML/Audio/Sound.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fmt/format.h>
#include "gamelogic.h"
#include "sceneGraph.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

#include "utilities/imageLoader.hpp"
#include "utilities/glfont.h"

#include <iostream>

enum KeyFrameAction {
    BOTTOM, TOP
};

#include <timestamps.h>
unsigned int currentKeyFrame = 0;
unsigned int previousKeyFrame = 0;

SceneNode* rootNode;
SceneNode* boxNode;
SceneNode* sphereNode;

// LIght nodes
SceneNode* lightNode1;
SceneNode* lightNode2;
SceneNode* lightNode3;
SceneNode* lightNode4;
SceneNode* lightNode5;

// These are heap allocated, because they should not be initialised at the start of the program
sf::SoundBuffer* buffer;
Gloom::Shader* shader;
Gloom::Shader* sphereShader;
sf::Sound* sound;

const glm::vec3 boxDimensions(180, 90, 90);

CommandLineOptions options;

bool hasStarted        = false;
bool hasLost           = false;
bool jumpedToNextFrame = false;
bool isPaused          = false;

bool mouseLeftPressed   = false;
bool mouseLeftReleased  = false;
bool mouseRightPressed  = false;
bool mouseRightReleased = false;

// Modify if you want the music to start further on in the track. Measured in seconds.
const float debug_startTime = 0;
double totalElapsedTime = debug_startTime;
double gameElapsedTime = debug_startTime;

double mouseSensitivity = 1.0;
double lastMouseX = windowWidth / 2;
double lastMouseY = windowHeight / 2;

//void mouseCallback(GLFWwindow* window, double x, double y) {
//    int windowWidth, windowHeight;
//    glfwGetWindowSize(window, &windowWidth, &windowHeight);
//    glViewport(0, 0, windowWidth, windowHeight);
//
//    double deltaX = x - lastMouseX;
//    double deltaY = y - lastMouseY;
//
//    glfwSetCursorPos(window, windowWidth / 2, windowHeight / 2);
//}

//// A few lines to help you if you've never used c++ structs
 struct LightSource {
     bool a_placeholder_value;
 };
 LightSource lightSources[4/*Put number of light sources you want here*/];

void initGame(GLFWwindow* window, CommandLineOptions gameOptions) {
    buffer = new sf::SoundBuffer();
    if (!buffer->loadFromFile("../res/Hall of the Mountain King.ogg")) {
        return;
    }

    options = gameOptions;
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    //glfwSetCursorPosCallback(window, mouseCallback);

    shader = new Gloom::Shader();
    shader->makeBasicShader("../res/shaders/simple.vert", "../res/shaders/simple.frag");

    sphereShader = new Gloom::Shader();
    sphereShader->makeBasicShader("../res/shaders/smallHatch.vert", "../res/shaders/smallHatch.frag");
    sphereShader->activate();

    // Fill buffers
    Mesh box = cube(boxDimensions, glm::vec2(5), true, true);
    Mesh sphere = generateSphere(1.0, 40, 40);
    unsigned int sphereVAO = generateBuffer(sphere);
    unsigned int boxVAO = generateBuffer(box);

    // Construct scene
    rootNode = createSceneNode();
    boxNode = createSceneNode();

    sphereNode = createSceneNode();
    sphereNode->vertexArrayObjectID = sphereVAO;
    sphereNode->VAOIndexCount = sphere.indices.size();
    sphereNode->shinyness = 10;
    rootNode->children.push_back(sphereNode);

    sphereNode->position = glm::vec3(0, 0, 5);

    rootNode->children.push_back(boxNode);

    boxNode->vertexArrayObjectID  = boxVAO;
    boxNode->VAOIndexCount        = box.indices.size();
    boxNode->shinyness = 10;

    // Set up lights
    lightNode1 = createSceneNode(POINT_LIGHT);
    lightNode2 = createSceneNode(POINT_LIGHT);
    lightNode3 = createSceneNode(POINT_LIGHT);
    lightNode4 = createSceneNode(POINT_LIGHT);
    lightNode5 = createSceneNode(POINT_LIGHT);
    rootNode->children.push_back(lightNode1);
    rootNode->children.push_back(lightNode2);

    lightNode1->sourceID = 1;
    lightNode1->color = glm::vec3(0,1,0);
    lightNode2->sourceID = 2;
    lightNode2->color = glm::vec3(0,0,1);

    lightNode3->sourceID = 0;
    lightNode3->color = glm::vec3(1, 0, 0);
    lightNode4->sourceID = 1;
    lightNode4->color = glm::vec3(0, 1, 0);
    lightNode5->sourceID = 2;
    lightNode5->color = glm::vec3(0, 0, 1);

    //Static light positions
    lightNode1->position.x = -boxDimensions.x / 2 + 5;
    lightNode1->position.y = -boxDimensions.y / 2 + 5;
    lightNode1->position.z = -70;

    lightNode2->position.x = boxDimensions.x / 2 - 10;
    lightNode2->position.y = boxDimensions.y / 2 - 20;
    lightNode2->position.z = -115;

    lightNode3->position.x = -15;
    lightNode3->position.y = -45;
    lightNode3->position.z = -50;

    lightNode4->position.x = 0;
    lightNode4->position.y = -45;
    lightNode4->position.z = -50;

    lightNode5->position.x = 15;
    lightNode5->position.y = -45;
    lightNode5->position.z = -50;


    getTimeDeltaSeconds();

    std::cout << fmt::format("Initialized scene with {} SceneNodes.", totalChildren(rootNode)) << std::endl;

    std::cout << "Ready. Click to start!" << std::endl;
}

void updateFrame(GLFWwindow* window) {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    double timeDelta = getTimeDeltaSeconds();


    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1)) {
        mouseLeftPressed = true;
        mouseLeftReleased = false;
    } else {
        mouseLeftReleased = mouseLeftPressed;
        mouseLeftPressed = false;
    }
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2)) {
        mouseRightPressed = true;
        mouseRightReleased = false;
    } else {
        mouseRightReleased = mouseRightPressed;
        mouseRightPressed = false;
    }

    if(!hasStarted) {
        if (mouseLeftPressed) {
            if (options.enableMusic) {
                sound = new sf::Sound();
                sound->setBuffer(*buffer);
                sf::Time startTime = sf::seconds(debug_startTime);
                sound->setPlayingOffset(startTime);
                sound->play();
            }
            totalElapsedTime = debug_startTime;
            gameElapsedTime = debug_startTime;
            hasStarted = true;
        }

    } else {
        totalElapsedTime += timeDelta;
            gameElapsedTime += timeDelta;

            if (mouseRightReleased) {
                isPaused = true;
                if (options.enableMusic) {
                    sound->pause();
                }
            }
            // Get the timing for the beat of the song
            for (unsigned int i = currentKeyFrame; i < keyFrameTimeStamps.size(); i++) {
                if (gameElapsedTime < keyFrameTimeStamps.at(i)) {
                    continue;
                }
                currentKeyFrame = i;
            }

            jumpedToNextFrame = currentKeyFrame != previousKeyFrame;
            previousKeyFrame = currentKeyFrame;

            double frameStart = keyFrameTimeStamps.at(currentKeyFrame);
            double frameEnd = keyFrameTimeStamps.at(currentKeyFrame + 1); // Assumes last keyframe at infinity

            double elapsedTimeInFrame = gameElapsedTime - frameStart;
            double frameDuration = frameEnd - frameStart;
            double fractionFrameComplete = elapsedTimeInFrame / frameDuration;

            double ballYCoord;

            KeyFrameAction currentOrigin = keyFrameDirections.at(currentKeyFrame);
            KeyFrameAction currentDestination = keyFrameDirections.at(currentKeyFrame + 1);
           
    }

    glm::mat4 projection = glm::perspective(glm::radians(80.0f), float(windowWidth) / float(windowHeight), 0.1f, 350.f);

    glm::vec3 cameraPosition = glm::vec3(0, 2, -20);
    // Camera movement speed
    float cameraSpeed = 2.5f * getTimeDeltaSeconds();

    // Move forward
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPosition += cameraSpeed * glm::vec3(0, 0, 1);
    // Move backward
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPosition -= cameraSpeed * glm::vec3(0, 0, 1);
    // Move left
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPosition -= glm::normalize(glm::cross(glm::vec3(0, 0, 1), glm::vec3(0, 1, 0))) * cameraSpeed;
    // Move right
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPosition += glm::normalize(glm::cross(glm::vec3(0, 0, 1), glm::vec3(0, 1, 0))) * cameraSpeed;

    // camera look at center
    glm::vec3 cameraTarget = glm::vec3(0, 0, 0);
    glm::vec3 up = glm::vec3(0, 1, 0);
    glm::mat4 cameraTransform = glm::lookAt(cameraPosition, cameraTarget, up);

    glm::mat4 VP = projection * cameraTransform;

    // Move and rotate various SceneNodes
    glUniform3fv(10, 1, glm::value_ptr(lightNode1->position));
    glUniform3fv(11, 1, glm::value_ptr(lightNode2->position));
    glUniformMatrix4fv(4, 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(5, 1, GL_FALSE, glm::value_ptr(cameraPosition));
    updateNodeTransformations(rootNode, glm::mat4(1.0f));
}

void updateNodeTransformations(SceneNode* node, glm::mat4 transformationThusFar) {
    glm::mat4 transformationMatrix =
              glm::translate(node->position)
            * glm::translate(node->referencePoint)
            * glm::rotate(node->rotation.y, glm::vec3(0,1,0))
            * glm::rotate(node->rotation.x, glm::vec3(1,0,0))
            * glm::rotate(node->rotation.z, glm::vec3(0,0,1))
            * glm::scale(node->scale)
            * glm::translate(-node->referencePoint);

    node->currentTransformationMatrix = transformationThusFar * transformationMatrix;
    node->transformationMatrix = transformationMatrix;


    switch(node->nodeType) {
        case GEOMETRY: break;
        case POINT_LIGHT:
            if (node->sourceID >= 0)
            {
                GLint posLocation = shader->getUniformFromName(fmt::format("lightArray[{}].position", node->sourceID));
                GLint colorLocation = shader->getUniformFromName(fmt::format("lightArray[{}].color", node->sourceID));
                glUniform3fv(posLocation, 1, glm::value_ptr(glm::vec3(node->position.x, node->position.y, node->position.z)));
                glUniform3fv(colorLocation, 1, glm::value_ptr(node->color));
            }
            break;
        case SPOT_LIGHT: break;
    }

    for(SceneNode* child : node->children) {
        updateNodeTransformations(child, node->currentTransformationMatrix);
    }
}

void renderNode(SceneNode* node) {
    if (node == sphereNode) {
        shader->deactivate();
        sphereShader->activate();
    }
    else {
        sphereShader->deactivate();
        shader->activate();
    }

    glUniformMatrix4fv(3, 1, GL_FALSE, glm::value_ptr(node->currentTransformationMatrix));
    //Normal matrix
    glad_glUniformMatrix3fv(7, 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::transpose(glm::inverse(node->transformationMatrix)))));

    switch(node->nodeType) {
        case GEOMETRY:
            if(node->vertexArrayObjectID != -1) {
                glBindVertexArray(node->vertexArrayObjectID);
                glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);
                glUniform1f(8, node->shinyness);
            }
            break;
        case POINT_LIGHT: break;
        case SPOT_LIGHT: break;
    }

    for(SceneNode* child : node->children) {
        renderNode(child);
    }
}

void renderFrame(GLFWwindow* window) {
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    glViewport(0, 0, windowWidth, windowHeight);

    renderNode(rootNode);
}
