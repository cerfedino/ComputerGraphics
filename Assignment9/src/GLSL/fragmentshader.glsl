#version 300 es
precision mediump float;

in vec3 v_color;
in vec3 v_normal;
in vec3 v_view_direction;

// Directional ligth
uniform vec3 light_direction;
uniform vec3 lightColor;
//

const float ambientCoeff = 0.05;
const float diffuseCoeff = 0.5;
const float specularCoeff = 0.5;
const float shininessCoeff = 0.1;

// Exercise 3: add all the input variable passed from the vertex shader
//             if the do not include the light direction, you should add here an additional uniform for it
//             you can also add here constants for Phong shading model, e.g., light color, ambient, diffuse, and specular coefficients, gamma value, as well as shininess

out vec4 out_color;
void main(){

    // Exercise 3: add computation of Phong shading
    //             do not forget about: normalizing all vectors beforehand, (2) performing gamma correction at the end

    // 1. IMPORTANT: normalize all the input vectors

    // vec3 l = normalize(light->position - point); // direction from the point to the light source

    // out_color = vec4(0,0,0,1);


    // vec3 diffuse = lightColor * diffuseCoeff * max(dot(v_normal, light_direction), 0.0);
    // vec3 ambient = lightColor * ambientCoeff;

    // vec3 specular = lightColor * specularCoeff * pow(max(dot(reflect(-light_direction, v_normal), v_view_direction), 0.0), shininessCoeff);

    // out_color = vec4(v_color * (ambient + diffuse + specular), 1.0);



    // // vec3 ambient
    vec3 final_color = vec3(0,0,0);
    

    vec3 l = -light_direction;
    float diffuse = max(dot(v_normal, l), 0.0);

    vec3 h = normalize(l + v_view_direction); // half vector

    float specular = pow(max(dot(v_normal, h), 0.0), shininessCoeff);
    final_color += lightColor*(diffuseCoeff*diffuse*v_color + specularCoeff*specular);
    final_color += ambientCoeff*v_color;
    
    
    // clamp the color to [0,1]
    out_color = vec4(clamp(final_color, vec3(0,0,0), vec3(1,1,1)), 1.0);
    

}

// glm::vec3 color(0.0);

// glm::vec3 l = glm::normalize(light->position - point); // direction from the point to the light source
// float diffuse = glm::dot(l, normal);
// if (diffuse < 0) diffuse = 0;

// glm::vec3 h = glm::normalize(l + view_direction); // half vector
// float specular = glm::pow(glm::dot(h,normal), 4*material.shininess);
// if (specular < 0) specular = 0;

// color += light->color * (material.diffuse*diffuse + material.specular*specular);

// color += ambient_light*material.ambient;

// // The final color has to be clamped so the values do not go beyond 0 and 1.
// color = glm::clamp(color, glm::vec3(0.0), glm::vec3(1.0));
// return color;
