#version 430 core

in layout(location = 0) vec3 position;
in layout(location = 1) vec3 normal_in;
in layout(location = 2) vec2 textureCoordinates_in;


uniform layout(location = 3) mat4 modelMatrix;
uniform layout(location = 7) mat3 normalMatrix;

uniform layout(location = 4) mat4 projection;
uniform layout(location = 5) mat4 cameraTransform;

out layout(location = 0) vec3 normal_out;
out layout(location = 1) vec2 textureCoordinates_out;
out layout(location = 2) vec4 vertexPos_in;
out layout(location = 3) vec4 projection_in;

void main()
{
    normal_out = normalize( normalMatrix * normal_in);

    textureCoordinates_out = textureCoordinates_in;

    vertexPos_in = modelMatrix * vec4(position, 1.0f);
    gl_Position = projection * cameraTransform * modelMatrix * vec4(position, 1.0f);
    
}
