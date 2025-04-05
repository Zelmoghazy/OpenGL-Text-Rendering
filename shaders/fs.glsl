#version 330 core

in vec2 fragTexCoord;  // Texture coordinates
in vec3 Color;         // Fragment Color

// Multiple outputs (important for subpixel rendering with blending)
layout(location = 0) out vec4 outColor0;  // For the RGB color component
layout(location = 1) out vec4 outColor1;  // For the alpha/intensity component

uniform sampler2D uAtlasTexture;  // Font atlas texture
uniform vec3 uTextColor;          // Base text color
uniform float uTime;              // Time for animation

void main() 
{
    vec4 texSample = texture(uAtlasTexture, fragTexCoord);
    
    float alpha = (texSample.r + texSample.g + texSample.b) / 3.0;
    
    if (alpha > 0.0) {
        alpha = 1.0;
    }
    
    vec3 finalColor;
    finalColor = Color;
    // finalColor = uTextColor;
    /* 
    finalColor.r = 1.0 + 0.5 * sin(uTime * 1.0);
    finalColor.g = 1.0 + 0.5 * sin(uTime * 1.3 + 2.0);
    finalColor.b = 1.0 + 0.5 * sin(uTime * 1.7 + 4.0);
    */

    outColor0 = vec4(finalColor, 1.0);
    
    outColor1 = vec4(texSample.rgb, alpha);
}