#version 410

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec3 vertex_edgeVector;
layout (location = 2) in vec3 vertex_wVector;
layout (location = 3) in vec3 vertex_normal;

uniform mat4 projection_mat, view_mat, model_mat, normal_mat;

//out vec3 position_world, normal_world; //edgeVector_world, wVector_world;
//out vec2 texture_coordinates;

out vec3 vInterpSurfPos;

out vec3 vInterpSurfNorm;

void main () {
	vec4 position_world = model_mat * vec4 (vertex_position, 1.0);
//	normal_world = normalize(vec3 ( model_mat * vec4 (vertex_normal, 0.0)));
////    edgeVector_world = vec3(model_mat * vec3(vertex_edgeVector, 0.0));
////    wVector_world = vec3(model_mat * vec3(vertex_wVector, 0.0));
//	texture_coordinates = vertex_texcoord;
//	gl_Position = projection_mat * view_mat * vec4 (position_world, 1.0);
    //vPosition = position_world;
    vInterpSurfPos = vec3(position_world);
    vInterpSurfNorm = normal_mat * vertex_normal;
    gl_Position = position_world;
}
