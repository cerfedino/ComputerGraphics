#version 300 es
precision mediump float;

in vec3 v_color;
in vec3 v_normal;
in vec3 v_view_direction;

// Directional ligth
uniform vec3 light_direction;
uniform vec3 lightColor;
//

const float ambientCoeff = 0.21;
const float diffuseCoeff = 0.5;
const float specularCoeff = 0.2;
const float shininessCoeff = 50.0;


const float alpha = 1.2f;
const float beta = 1.8f;
const float gamma = 1.8f;

// Exercise 3: add all the input variable passed from the vertex shader
//             if the do not include the light direction, you should add here an additional uniform for it
//             you can also add here constants for Phong shading model, e.g., light color, ambient, diffuse, and specular coefficients, gamma value, as well as shininess

out vec4 out_color;
void main(){

    // Exercise 3: add computation of Phong shading
    //             do not forget about: normalizing all vectors beforehand, (2) performing gamma correction at the end

    vec3 final_color = vec3(0,0,0);

    vec3 l = normalize(light_direction);
    float diffuse = max(dot(l, v_normal), 0.0); 
    vec3 h = normalize(l);

    float specular = pow(max(dot(v_normal, h), 0.0), 4.0*shininessCoeff);
    final_color += lightColor*(diffuseCoeff*diffuse*v_color + specularCoeff*specular);
    final_color += ambientCoeff*v_color;
    
    // Tone mapping
    vec3 tonemapped = alpha * pow(pow(final_color, vec3(beta,beta,beta)), vec3(1.0/gamma,1.0/gamma,1.0/gamma));
    out_color = vec4(clamp(tonemapped, vec3(0,0,0), vec3(1,1,1)), 1.0);

}
