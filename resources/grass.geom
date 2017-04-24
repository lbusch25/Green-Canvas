#version 330

// You must specify what type of primitive is passed in
layout(lines) in;

// ... and what type of primitive you are outputing and how many vertices. The geometry shader is run once
// for each primitive so we will output three lines (6 vertices), one for each normal.
layout(triangle_strip, max_vertices = 4) out;

//in vec3 position_world[2], normal_world[2];
//in vec2 texture_coordinates[2];
//
//out vec3 position_world[2], normal_world[2];
//out vec2 texture_coordinates[2];

uniform mat4 projection_mat, view_mat, model_mat;

void draw_grass() {
    //Calculates the world coordinates by multiplying the local coordinates by the model, view, and projection matrices (in that order), using the built in gl_Position to hold the position
    //start vertex
    gl_Position = projection_mat * view_mat * model_mat * gl_in[0].gl_Position;
    EmitVertex();
    
    //next vertex
    gl_Position = projection_mat * view_mat * model_mat * gl_in[1].gl_Position;
    EmitVertex();
    
    //start + 0.1 vertex
    gl_Position = projection_mat * view_mat * model_mat * (gl_in[0].gl_Position + vec4(0.1,0.0,0.0,0.0));
    EmitVertex();
    
    //next + 0.1 vertex
    gl_Position = projection_mat * view_mat * model_mat * (gl_in[1].gl_Position + vec4(0.1,0.0,0.0,0.0));
    EmitVertex();
    
    EndPrimitive(); //Tells the geometry shader that you are done calculating a primitive, and draws lines between its vertices and sends it to the fragment shader
}

void main() {
    draw_grass();
}












