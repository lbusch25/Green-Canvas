#version 410

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec3 vertex_w;

uniform mat4 model_mat;

//out vec3 position_world, normal_world; //edgeVector_world, wVector_world;
//out vec2 texture_coordinates;

out vec3 vInterpSurfPos;
out vec3 vInterpW;

void main () {
	vec4 position_world = model_mat * vec4 (vertex_position, 1.0);
    vInterpSurfPos = vec3(position_world);
    vInterpW = vec3(model_mat * vec4 (vertex_w, 0.0));
    gl_Position = position_world;
}
