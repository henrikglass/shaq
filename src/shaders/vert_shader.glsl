R""(

#version 450 core

layout (location = 0) in vec3 in_xyz;

void main(void)
{
    gl_Position = vec4(in_xyz, 1.0);
}

)""

