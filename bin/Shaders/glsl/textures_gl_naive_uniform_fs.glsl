#version 410

// Uniforms / SSBO ----------------------------------------------------------------------------------------------------
uniform sampler2D gTex;

// Input --------------------------------------------------------------------------------------------------------------
in block {
    vec2 v2TexCoord;
} In;

//  Output ------------------------------------------------------------------------------------------------------------
layout(location = 0) out vec4 Out_v4Color;

// Functions ----------------------------------------------------------------------------------------------------------
void main()
{
    Out_v4Color = vec4(texture(gTex, In.v2TexCoord).xyz,  1);
}
