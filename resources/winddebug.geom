layout(points) in;
layout(line_strip, max_vertices = 2) out;

uniform mat4 projection_mat, view_mat, model_mat;

in vec3 windVel[];

out vec3 color;

void main() {

    gl_Position = projection_mat * view_mat * model_mat * gl_in[0].gl_Position;
    color = vec3(1.0, 0.0, 0.0);
    EmitVertex();

    gl_Position = projection_mat * view_mat * model_mat * (gl_in[0].gl_Position + windVel[0]);
    color = vec3(0.0, 1.0, 0.0);
    EmitVertex();

    EndPrimitive();  
}