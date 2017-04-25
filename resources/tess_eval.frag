#version 410

layout(isolines) in; //Tells shader that it is tessellation line objects
in vec3 tcPosition[];
uniform mat4 projection_mat, view_mat;

vec3 bezier(vec3 p0, vec3 p1, vec3 p2, vec3 p3, float t) {
    vec3 position = pow((1.0-t), 3.0) * p0 + 3.0 * pow((1.0-t), 2.0) * t * p1 +
        3.0 * (1.0-t) * pow(t, 2.0) * p2 + pow(t, 3.0) * p3;
    return position;
}

void main() {
    float t = gl_TessCoord.x; //Tells us where we are in the line patch, only need x because it is a line
    vec3 tesPosition = bezier(tcPosistion[0], tcPosition[1], tcPosition[2], tcPosition[3], t);
    gl_Position = projection_mat * view_mat * vec4(tesPosition, 1.0);
    }
}
