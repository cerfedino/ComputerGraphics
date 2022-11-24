#version 300 es
in vec3 a_position;
in vec3 a_color;
in vec3 a_normal;

out vec3 v_color;
out vec3 v_normal;


// Object transformation(s)
uniform mat4 scaleMatrix;
uniform mat4 rotationMatrix;
uniform mat4 translationMatrix;

// Camera rotation
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

// Light
out vec3 v_view_direction;


// Exercise 3: add input attribute for normals
//             add output variables required for light computation, e.g., normal, view direction etc.
//             add here also a uniform for light direction, unless you pass it directly to the fragment shader

void main(){
    v_color = a_color;
    v_normal = a_normal;

    // compute all the variables required for light computation in the fragment shader
    // remember that all the locations and vectors have to be in a common space, e.g., eye/camera space

    mat4 modelMatrix = translationMatrix * rotationMatrix * scaleMatrix;

    // TODO: calculate view direction
    // replace the rotationMatrix with correct transformations
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(a_position,1.0);
    v_normal = vec3(rotationMatrix * vec4(a_normal, 0.0));
    v_view_direction = normalize(vec3(gl_Position));
}