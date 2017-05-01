#version 410

// You must specify what type of primitive is passed in
layout(lines) in;

// ... and what type of primitive you are outputing and how many vertices. The geometry shader is run once
// for each primitive so we will output three lines (6 vertices), one for each normal.
layout(triangle_strip, max_vertices = 4) out;

in vec3 tesInterpW[]; //Might need a 2 in here
in vec3 tesInterpSurfPos[];

out vec3 geomInterpSurfNorm;
out vec3 geomInterpSurfPos;

uniform mat4 projection_mat, view_mat;

float bladeWidth = 0.1;

void draw_grass() {
    vec3 edgeVec = vec3(gl_in[1].gl_Position -  gl_in[0].gl_Position);
    //Calculates the world coordinates by multiplying the local coordinates by the model, view, and projection matrices (in that order), using the built in gl_Position to hold the position
    //start vertex
    gl_Position = projection_mat * view_mat * gl_in[0].gl_Position;
    geomInterpSurfNorm = cross(tesInterpW[0], edgeVec);
    geomInterpSurfPos = tesInterpSurfPos[0];
    EmitVertex();
    
    //next vertex
    gl_Position = projection_mat * view_mat * gl_in[1].gl_Position;
    geomInterpSurfNorm =cross(tesInterpW[1], edgeVec);
    geomInterpSurfPos = tesInterpSurfPos[1];
    EmitVertex();
    
    //start + 0.1 vertex
    gl_Position = projection_mat * view_mat * (gl_in[0].gl_Position +  vec4(bladeWidth * tesInterpW[0], 0.0));
    geomInterpSurfNorm = cross(tesInterpW[0], edgeVec);
    geomInterpSurfPos = tesInterpSurfPos[0];
    EmitVertex();
    
    //next + 0.1 vertex
    gl_Position = projection_mat * view_mat  * (gl_in[1].gl_Position + vec4(bladeWidth * tesInterpW[1], 0.0));
    geomInterpSurfNorm = cross(tesInterpW[1], edgeVec);
    geomInterpSurfPos = tesInterpSurfPos[1];
    EmitVertex();
    
    EndPrimitive(); //Tells the geometry shader that you are done calculating a primitive, and draws lines between its vertices and sends it to the fragment shader
}

void main() {
    draw_grass();
}












