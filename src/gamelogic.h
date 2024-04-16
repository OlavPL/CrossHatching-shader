#pragma once

#include <utilities/window.hpp>
#include "sceneGraph.hpp"
#include <utilities/mesh.h>
#include <utilities/tiny_obj_loader.h>
#include "utilities/imageLoader.hpp"

void updateNodeTransformations(SceneNode* node, glm::mat4 transformationThusFar);
void initGame(GLFWwindow* window, CommandLineOptions options);
void updateFrame(GLFWwindow* window);
void renderFrame(GLFWwindow* window);
SceneNode* getShapesFromFile(std::string inputfile, float scale);
float toRadians(float degrees);
unsigned int getTextureID(PNGImage image);