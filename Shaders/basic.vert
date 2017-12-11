#version 140
#extension GL_ARB_explicit_attrib_location : require

layout (location = 0) in vec3 a_vertex;
layout (location = 1) in vec3 a_normal;

uniform mat4 rotation;
uniform mat4 transform;

out vec3 g_vertex;
out vec3 g_normal;
void main()
{
    vec4 v =  transform * rotation*vec4(a_vertex, 1.0);
    g_vertex = v.xyz/v.w;
    g_normal = a_normal;
    gl_Position =  v;
}
