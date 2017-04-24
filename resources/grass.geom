#version 330

// You must specify what type of primitive is passed in
layout(line_strip) in;

// ... and what type of primitive you are outputing and how many vertices. The geometry shader is run once
// for each primitive so we will output three lines (6 vertices), one for each normal.
layout(triangle_strip, max_vertices = 12) out;

uniform mat4 projection_mat, view_mat, model_mat;

void main() {

    for(int i = 0; i < 3; i++) {
        //Calculates the world coordinates by multiplying the local coordinates by the model, view, and projection matrices (in that order), using the built in gl_Position to hold the position
        //i vertext
        gl_Position = projection_mat * view_mat * model_mat * gl_in[i].gl_Position;
        EmitVertex();
        
        //i.x + 0.1 vertex
        gl_Position = projection_mat * view_mat * model_mat * (gl_in[i].gl_Position + vec4(0.1,0.0,0.0,0.0));
        EmitVertex();
        
        //[i+1] vertex
        gl_Position = projection_mat * view_mat * model_mat * gl_in[i+1].gl_Position;
        EmitVertex();
        
        //[i+1].x + 0.1 vertex
        gl_Position = projection_mat * view_mat * model_mat * (gl_in[i+1].gl_Position + vec4(0.1,0.0,0.0,0.0));
        EmitVertex();
        
    }
    EndPrimitive(); //Tells the geometry shader that you are done calculating a primitive, and draws lines between its vertices and sends it to the fragment shader
}












