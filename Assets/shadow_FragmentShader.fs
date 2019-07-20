#version 330

in vec2 UV;
in vec4 lightSpace;

out vec4 fragColor;

uniform sampler2D tex;
uniform sampler2D shadowMap;

float getShadow(vec4 lightSpace)
{
    vec3 projCoords = lightSpace.xyz / lightSpace.w;
    projCoords = projCoords * 0.5 + 0.5; 
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    float currentDepth = projCoords.z;
    float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;
    return shadow;
}

void main() {
	vec3 color = texture(tex, UV).rgb;
	float shadow = getShadow(lightSpace);  
	fragColor = vec4((1.0 - shadow) * color, 1.0);
}
