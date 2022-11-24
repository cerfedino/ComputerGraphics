#version 300 es
precision mediump float;

in vec3 v_color;
in vec3 v_normal;
in vec3 v_view_direction;
out vec4 out_color;


// Directional ligth
uniform vec3 dlight_direction;
uniform vec3 dlightColor;
//
// Point light
in vec3 v_direction_to_plight;
in float v_distance;
uniform vec3 plightColor;



const float ambientCoeff = 0.1;
const float diffuseCoeff = 0.5;
const float specularCoeff = 0.2;
const float shininessCoeff = 50.0;


const float alpha = 1.2f;
const float beta = 1.8f;
const float gamma = 1.8f;


vec3 ComputeDirectionalLighting() {
    vec3 final_color = vec3(0,0,0);

    vec3 l = normalize(dlight_direction);
    float diffuse = max(dot(l, v_normal), 0.0); 
    vec3 h = normalize(l);

    float specular = pow(max(dot(v_normal, h), 0.0), 4.0*shininessCoeff);
    final_color += dlightColor*(diffuseCoeff*diffuse*v_color + specularCoeff*specular);
    return final_color;
}


vec3 ComputePointLighting() {
    vec3 final_color = vec3(0,0,0);

    vec3 l = normalize(v_direction_to_plight); // direction from the point to the light source
    float diffuse = max(dot(l, v_normal), 0.0); 
    vec3 h = normalize(l+v_view_direction);
    float specular = pow(max(dot(h,v_normal), 0.0), 4.0*shininessCoeff);
    final_color += plightColor * (diffuseCoeff*diffuse + specularCoeff*specular);

    float attenuation = 1.0/pow(v_distance,2.0);
    return final_color*attenuation;
}


void main(){
    vec3 final_color = ComputeDirectionalLighting() + ComputePointLighting() + ambientCoeff*v_color;;
    
    // Tone mapping
    vec3 tonemapped = alpha * pow(pow(final_color, vec3(beta,beta,beta)), vec3(1.0/gamma,1.0/gamma,1.0/gamma));
    out_color = vec4(clamp(tonemapped, vec3(0,0,0), vec3(1,1,1)), 1.0);
}
