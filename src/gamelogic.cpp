#include <chrono>
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <SFML/Audio/SoundBuffer.hpp>
#include <utilities/shader.hpp>
#include <glm/vec3.hpp>
#include <iostream>
#include <utilities/timeutils.h>
#include <utilities/mesh.h>
#include <utilities/glutils.h>
#include <utilities/shapes.h>
#include <SFML/Audio/Sound.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fmt/format.h>
#include "gamelogic.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "utilities/tiny_obj_loader.h"
#include "sceneGraph.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

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
SceneNode* solNode;

// Light nodes
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




bool mouseLeftPressed   = false;
bool mouseLeftReleased  = false;
bool mouseRightPressed  = false;
bool mouseRightReleased = false;

bool qWasPressed = false;
bool qIsPressed = false;
bool upWasPressed = false;
bool upIsPressed = false;
bool downWasPressed = false;
bool downIsPressed = false;

bool spinLights = false;
float spinLightTimer = 0;
float lightSpeed = 1;

glm::vec3 cameraPosition = glm::vec3(0, 2, 40);
glm::mat4 cameraTransform;
glm::mat4 VP;
glm::vec3 cameraFront = glm::vec3(0.0, 0.0, 1.0);
glm::vec3 cameraTarget = glm::vec3(0, 0, 0);
glm::vec3 up = glm::vec3(0, 1, 0);
float pitch = 0.0f;
float yaw = -90.0f;
float roll = 0.0f;
float M_PI = 3.14159265358979323846;

double firstFrameTime = 0;
double lastFrameTime = 0;

double mouseSensitivity = 1.0;
double lastMouseX = windowWidth / 2;
double lastMouseY = windowHeight / 2;

Gloom::Shader* shader1;
Gloom::Shader* shader2;

PNGImage texture;
unsigned int voronoiTextureID ;
GLint textureLocation;

PNGImage hatch_light_1;
unsigned int hatch_light_1_ID;
GLint hatch_light_location;

PNGImage hatch_light_2;
unsigned int hatch_light_2_ID;
GLint hatch_light_2_location;

PNGImage hatch_dense;
unsigned int hatch_denseID;
GLint hatch_dense_location;


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

    // Test shader
    shader = new Gloom::Shader();
    shader->makeBasicShader("../res/shaders/lightBasedTextures.vert", "../res/shaders/lightBasedTextures.frag");
        //add voronoi texture to shader

    texture = loadPNGFile("../res/textures/voronoiTexture.png");
    //glActiveTexture(GL_TEXTURE0); // Activate first texture unit 0
    voronoiTextureID = getTextureID(texture);
    glBindTexture(GL_TEXTURE_2D, voronoiTextureID);
    textureLocation = shader->getUniformFromName("voronoiTexture");
    glUniform1i(textureLocation, 0);

    hatch_light_1 = loadPNGFile("../res/textures/hatch_light_2.png");
    glActiveTexture(GL_TEXTURE1); // Activate second texture unit 1
    hatch_light_1_ID = getTextureID(hatch_light_1);
    glBindTexture(GL_TEXTURE_2D, hatch_light_1_ID);
    hatch_light_location = shader->getUniformFromName("hatch_light_1");
    glUniform1i(hatch_light_location, 1);

    hatch_light_2 = loadPNGFile("../res/textures/hatch_semi_dense.png");
    glActiveTexture(GL_TEXTURE2); // Activate second texture unit 2
    hatch_light_2_ID = getTextureID(hatch_light_2);
    glBindTexture(GL_TEXTURE_2D, hatch_light_2_ID);
    hatch_light_2_location = shader->getUniformFromName("hatch_light_2");
    glUniform1i(hatch_light_2_location, 2);

    hatch_dense = loadPNGFile("../res/textures/hatch_dense.png");
    glActiveTexture(GL_TEXTURE3); // Activate texture unit  3
    hatch_denseID = getTextureID(hatch_dense);
    glBindTexture(GL_TEXTURE_2D, hatch_denseID);
    hatch_dense_location = shader->getUniformFromName("hatch_dense");
    glUniform1i(hatch_dense_location, 3);

    shader1 = shader;

    shader2 = new Gloom::Shader();
    shader2->makeBasicShader("../res/shaders/basicCrossHatch.vert", "../res/shaders/basicCrossHatch.frag");
    shader = shader2;
    shader->activate();


    // Fill buffers
    //Mesh box = cube(boxDimensions, glm::vec2(5), true, true);
    Mesh box = cube(glm::vec3(windowWidth, windowHeight, 0), glm::vec2(5), true, true);
    Mesh sphere = generateSphere(1.0, 40, 40);
    unsigned int sphereVAO = generateBuffer(sphere);
    unsigned int boxVAO = generateBuffer(box);

    // Construct scene
    rootNode = createSceneNode();
    boxNode = createSceneNode();
    sphereNode = createSceneNode();

    boxNode->vertexArrayObjectID  = boxVAO;
    boxNode->VAOIndexCount        = box.indices.size();
    boxNode->shinyness = 10;
    boxNode->position = glm::vec3(0, 0, 0);

    sphereNode->vertexArrayObjectID = sphereVAO;
    sphereNode->VAOIndexCount = sphere.indices.size();
    sphereNode->shinyness = 10;
    sphereNode->position = glm::vec3(0, 0, -10);

    solNode = getShapesFromFile("../res/models/SOL2.obj");
    solNode->shinyness = 10;
    solNode->position = glm::vec3(0, -10, 10);


    
    //rootNode->children.push_back(boxNode);
    rootNode->children.push_back(sphereNode);
    rootNode->children.push_back(solNode);



    // Set up lights
    lightNode1 = createSceneNode(POINT_LIGHT);
    lightNode2 = createSceneNode(POINT_LIGHT);
    lightNode3 = createSceneNode(POINT_LIGHT);
    lightNode4 = createSceneNode(POINT_LIGHT);
    lightNode5 = createSceneNode(POINT_LIGHT);
    //rootNode->children.push_back(lightNode1);
    rootNode->children.push_back(lightNode2);
    rootNode->children.push_back(lightNode3);

    lightNode1->sourceID = 1;
    lightNode1->color = glm::vec3(0,1,0);
    lightNode2->sourceID = 2;
    lightNode2->color = glm::vec3(0,0,1);

    lightNode3->sourceID = 0;
    lightNode3->color = glm::vec3(1, 1, 1);
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

    lightNode3->position.x = 5;
    lightNode3->position.y = 10;
    lightNode3->position.z = 20;

    lightNode4->position.x = 0;
    lightNode4->position.y = -45;
    lightNode4->position.z = -50;

    lightNode5->position.x = 15;
    lightNode5->position.y = -45;
    lightNode5->position.z = -50;


    glfwGetTime();

    std::cout << fmt::format("Initialized scene with {} SceneNodes.", totalChildren(rootNode)) << std::endl;

    std::cout << "Ready. Click to start!" << std::endl;
}

void updateFrame(GLFWwindow* window) {
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);

    double timeDelta = glfwGetTime();
    double deltaTime = timeDelta - lastFrameTime;
    lastFrameTime = timeDelta;


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
    // Camera movement speed
    float cameraSpeed = 40 * deltaTime;

    // Move 
    // Move forward
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        cameraPosition += cameraFront * cameraSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        cameraPosition -= cameraSpeed * cameraFront;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        cameraPosition -= glm::normalize(glm::cross(cameraFront, up)) * cameraSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        cameraPosition.y -= cameraSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		cameraPosition.y += cameraSpeed;
	}
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        cameraPosition += glm::normalize(glm::cross(cameraFront, up)) * cameraSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        yaw -= 1.5 * cameraSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        yaw += 1.5 * cameraSpeed;
    }

    // Changing shaders
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
    {
        shader->deactivate();
        shader = shader1;
        shader->activate();
        //shader1->activate();

    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
    {
        shader->deactivate();
        shader = shader2;
        shader->activate();
		//shader2->activate();
	}

    // If Q is pressed this frame, spin lights
    qWasPressed = qIsPressed;
    qIsPressed = glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS;
    if (qIsPressed && !qWasPressed)
    {
        spinLights = !spinLights;
    }
    upWasPressed = upIsPressed;
    upIsPressed = glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS;
    if (upIsPressed && !upWasPressed)
    {
        lightSpeed += 0.1;
	}
    downWasPressed = downIsPressed;
    downIsPressed = glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS;
    if (downIsPressed && !downWasPressed)
    {
        lightSpeed -= 0.1;
    }

    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    {
        lightSpeed = 1;
        spinLightTimer = 0;

        lightNode3->position.x = sin(spinLightTimer) * 35;
        lightNode3->position.z = cos(spinLightTimer) * 35;
        lightNode3->position.y = sin(spinLightTimer * 0.3) * 25;

        sphereNode->position.x = sin(spinLightTimer) * 35;
        sphereNode->position.z = cos(spinLightTimer) * 35;
        sphereNode->position.y = sin(spinLightTimer * 0.2) * 25;
    }

    glm::vec3 cameraRotation = glm::vec3(
        (cos(toRadians(yaw)) * cos(toRadians(pitch))),
        (sin(toRadians(pitch))),
        (sin(toRadians(yaw)) * cos(toRadians(pitch)))
        );

    cameraFront = normalize(cameraRotation);

    //boxNode->position = glm::vec3(0, 0, -80);
    ////sphereNode->position = glm::vec3(0, 0, -10);
    //solNode->position = glm::vec3(0, 0, 0);

    if (spinLights)
    {
        spinLightTimer += deltaTime * lightSpeed;

        lightNode3->position.x = sin(spinLightTimer ) * 35;
        lightNode3->position.z = cos(spinLightTimer) * 35;
        lightNode3->position.y = sin(spinLightTimer *0.2) * 45;

        sphereNode->position.x = sin(spinLightTimer) * 35;
        sphereNode->position.z = cos(spinLightTimer) * 35;
        sphereNode->position.y = sin(spinLightTimer * 0.2) * 45;
    }

    glm::mat4 projection = glm::perspective(glm::radians(80.0f), float(windowWidth) / float(windowHeight), 0.1f, 350.f);

    // camera look at center
    // update camera for movement relative to camera
    cameraTransform = glm::lookAt(cameraPosition, (cameraPosition + cameraFront), up);

    VP = projection * cameraTransform;
    updateNodeTransformations(boxNode, VP);
    glUniform2fv(9, 1, glm::value_ptr(glm::vec2(boxNode->position.x, boxNode->position.y)));
    glUniform2fv(12, 1, glm::value_ptr(glm::vec2(windowWidth, windowHeight)));

    glUniform1i(textureLocation, 0);
    glUniform1i(hatch_light_location, 1);
    glUniform1i(hatch_light_2_location, 2);
    glUniform1i(hatch_dense_location, 3);

    // Move and rotate various SceneNodes
    //glUniform3fv(10, 1, glm::value_ptr(lightNode1->position));
    //glUniform3fv(11, 1, glm::value_ptr(lightNode2->position));
    glUniformMatrix4fv(4, 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(5, 1, GL_FALSE, glm::value_ptr(cameraTransform));
    glUniformMatrix4fv(6, 1, GL_FALSE, glm::value_ptr(cameraPosition));
    updateNodeTransformations(rootNode, glm::mat4(1));
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
    //if (node == sphereNode) {
    //    //shader->deactivate();
    //    sphereShader->activate();
    //}
    //else {
    //    //sphereShader->deactivate();
    //    shader->activate();
    //}

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

SceneNode* getShapesFromFile(std::string inputfile)
{
    // Load the .obj file
    tinyobj::ObjReaderConfig reader_config;
    reader_config.mtl_search_path = "../res/models"; // Path to material files
    //reader_config.triangulate = true;

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(inputfile, reader_config)) {
        if (!reader.Error().empty()) {
            std::cerr << "TinyObjReader: " << reader.Error();
        }
        exit(1);
    }

    if (!reader.Warning().empty()) {
        std::cout << "TinyObjReader: " << reader.Warning();
    }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    auto& materials = reader.GetMaterials();
    

    SceneNode* tempRoot = createSceneNode();
    // Loop over shapes
    for (size_t s = 0; s < shapes.size(); s++) 
    {
        Mesh mesh = Mesh();
        SceneNode* tempNode = createSceneNode();
        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) 
        {
            size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);



            // Loop over vertices in the face.
            for (size_t v = 0; v < fv; v++) 
            {
                // access to vertex
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

                size_t new_vertex_index = mesh.vertices.size();
                mesh.vertices.push_back(glm::vec3(vx, vy, vz));
                mesh.indices.push_back(new_vertex_index);

                // Check if `normal_index` is zero or positive. negative = no normal data
                // Also adding directoinField
                if (idx.normal_index >= 0) {
                    tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
                    tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
                    tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
                    mesh.normals.push_back(glm::vec3(nx, ny, nz));

                    // Calculate direction field
                    glm::vec3 normal = glm::vec3(nx, ny, nz);
                    glm::vec3 direction;
                    if (normal.x != 0 || normal.y != 0) {
                        direction = glm::vec3(normal.y, -normal.x, 0);
                    }
                    else {
                        direction = glm::vec3(0, normal.z, -normal.y);
                    }

                    // Calculate the tangent vector by taking the cross product of the normal vector and the arbitrary vector
                    glm::vec3 tangent = glm::cross(normal, direction);
                    // Calculate the principal curvature direction by taking the cross product of the normal vector and the tangent vector
                    glm::vec3 principalCurvatureDirection = glm::cross(normal, tangent);
                    mesh.directionField.push_back(principalCurvatureDirection);


                }
                else {
                    mesh.normals.push_back(glm::vec3(0.0f, 0.0f, 0.0f)); // default normal
                    mesh.directionField.push_back(glm::vec3(0.0f, 0.0f, 0.0f)); // default direction field
                }

                // Check if `texcoord_index` is zero or positive. negative = no texcoord data
                if (idx.texcoord_index >= 0) {
                    tinyobj::real_t tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
                    tinyobj::real_t ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
                    mesh.textureCoordinates.push_back(glm::vec2(tx, ty));
                }
                else {
                    mesh.textureCoordinates.push_back(glm::vec2(0.0f, 0.0f)); // default texcoord
                }
                // Optional: vertex colors
                // tinyobj::real_t red   = attrib.colors[3*size_t(idx.vertex_index)+0];
                // tinyobj::real_t green = attrib.colors[3*size_t(idx.vertex_index)+1];
                // tinyobj::real_t blue  = attrib.colors[3*size_t(idx.vertex_index)+2];
            }
            index_offset += fv;

            // per-face material
            shapes[s].mesh.material_ids[f];
        }
        unsigned int meshVAO = generateBuffer(mesh);
        tempNode->vertexArrayObjectID = meshVAO;
        tempNode->VAOIndexCount = mesh.indices.size();
        tempNode->position = glm::vec3(0, 0, 0);
        tempNode->scale = glm::vec3(50, 50, 50);
        tempRoot->children.push_back(tempNode);
    }
    return tempRoot;
}

//static float ToRadians(float angle)
//{
//    return (float)(PI / 180) * angle;
//}
float toRadians(float degrees)
{
    return degrees * M_PI / 180.0;
}

unsigned int getTextureID(PNGImage image) {
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    // Set texture wrapping to GL_REPEAT (usually basic wrapping method)
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // Set texture filtering to linear
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Load and generate the texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.pixels.data());
	glGenerateMipmap(GL_TEXTURE_2D);

	return textureID;
}