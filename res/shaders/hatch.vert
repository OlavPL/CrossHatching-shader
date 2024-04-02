#version 430 core

in layout(location = 0) vec3 position;
in layout(location = 1) vec3 normal_in;
in layout(location = 2) vec2 textureCoordinates_in;


uniform layout(location = 3) mat4 modelMatrix;
uniform layout(location = 7) mat3 normalMatrix;

uniform layout(location = 4) mat4 projection;
uniform layout(location = 5) mat4 cameraTransform;
uniform layout(location = 6) vec3 cameraPos_in;

out layout(location = 0) vec3 normal_out;
out layout(location = 1) vec2 textureCoordinates_out;
out layout(location = 2) vec4 vertexPos_out;
out layout(location = 3) vec3 cameraPos_out;
out layout(location = 14) mat4 projection_out;
out layout(location = 5) mat4 cameraTransform_out;
out layout(location = 10) mat4 modelMatrix_out;

void main()
{
//    normal_out = normalize( normalMatrix * normal_in);
    normal_out = normal_in;

    textureCoordinates_out = textureCoordinates_in;
    cameraPos_out = cameraPos_in;
    vertexPos_out = modelMatrix * vec4(position, 1.0f);
    cameraTransform_out = cameraTransform;
    modelMatrix_out = modelMatrix;
    projection_out = projection;
    gl_Position = projection * cameraTransform * modelMatrix * vec4(position, 1.0f);
    
}
