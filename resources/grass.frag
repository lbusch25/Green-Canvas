#version 410

in vec3 geomInterpSurfPos, geomInterpSurfNorm;

uniform sampler2D diffuseRamp;
//uniform sampler2D specularRamp;

uniform vec3 eye_world;

uniform vec4 lightPosition;

uniform vec3 ambientReflection;
uniform vec3 diffuseReflection;
//uniform vec3 specularReflection;

uniform vec3 diffuseIntensity;
uniform vec3 ambientIntensity;
//uniform vec3 specularIntensity;

//uniform float specularExp;

out vec4 fragment_colour;

void main () {
    vec3 normal = normalize(geomInterpSurfNorm);
    vec3 lightDirection = normalize(lightPosition.xyz - geomInterpSurfPos.xyz);
    vec3 eyeDirection = normalize(eye_world - geomInterpSurfPos.xyz);
    vec3 Hvec = normalize(eyeDirection + lightDirection);
    
    float dotNL = dot(lightDirection, normal);
    float dotHN = dot(Hvec, normal);
    
    vec3 final_color = vec3(0.0, 0.0, 0.0);
    vec2 diffuseTex = vec2(max(0, dotNL), 0.0);
    //vec2 specularTex = vec2(pow(max(0,dotHN), specularExp), 0.0);
    
    vec3 ambientColor = ambientReflection * ambientIntensity;
    vec3 diffuseColor = diffuseReflection * diffuseIntensity * texture(diffuseRamp, diffuseTex).xyz;
//    vec3 specularColor = vec3 specularColor = specularReflection *specularIntensity * texture(specularRamp, specularTex).xyz;
    
    
//    final_colour = ambientColor + diffuseColor + specularColor;
    
    final_color = ambientColor + diffuseColor;
	
    fragment_colour = vec4(final_color, 1.0);
}
