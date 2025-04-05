#version 330 core

// Inputs
layout(location = 0) in vec2 inPosition;  // Vertex position
layout(location = 1) in vec2 inTexCoord;  // Texture coordinates
layout (location = 2) in vec3 inColor;

// Outputs to fragment shader
out vec2 fragTexCoord;  // Pass UV coordinates to fragment shader
out vec3 Color;         // Pass fragment color  

// Uniforms
uniform mat4 uProjection;  // Projection matrix

void main() 
{
    // Pass texture coordinates to fragment shader
    fragTexCoord = inTexCoord;
    Color = inColor;
    
    // Apply projection to vertex position
    gl_Position = uProjection * vec4(inPosition, 0.0, 1.0);
}
