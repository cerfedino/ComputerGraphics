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
out vec3 v_direction_to_plight;
out float v_distance;

uniform vec3 plight_position;

void main(){
    v_color = a_color;
    v_normal = a_normal;
    mat4 modelMatrix = translationMatrix * rotationMatrix * scaleMatrix;

    v_direction_to_plight = normalize(plight_position - vec3(modelMatrix * vec4(a_position, 1.0)));

    v_distance = max(0.1f, length(plight_position - vec3(modelMatrix * vec4(a_position, 1.0))));
    
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(a_position,1.0);
    v_normal = vec3(rotationMatrix * vec4(a_normal, 0.0));
    v_view_direction = -vec3(modelMatrix * vec4(a_position, 1.0));
    
    
    
}