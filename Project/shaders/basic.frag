#version 410 core

in vec3 fNormal;
in vec4 fPosEye;
in vec2 fTexCoords;
in vec4 fragPosLightSpace;

out vec4 fColor;

//lighting
uniform	vec3 lightDir;
uniform	vec3 lightColor;
uniform vec3 lightPosition;
uniform int lightType;
float attenuation = 1.0f;  //for positional light 

//texture
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;
float shininess = 32.0f;

//fog
float fogEnd = 10.0f;
vec3 fogColor = vec3(0.37,0.34,0.30);
uniform float fogDensity;

float getFogFactor(){
	float fogCoordinate = abs(fPosEye.z/fPosEye.w);
	float fogFactor = exp(-fogDensity * fogCoordinate);
	
	fogFactor = 1.0 - clamp(fogFactor,0.0,1.0);
	return fogFactor;
}


void computeLightComponents()
{		
	vec3 cameraPosEye = vec3(0.0f);//in eye coordinates, the viewer is situated at the origin
	
	//transform normal
	vec3 normalEye = normalize(fNormal);	
	
	//compute light direction
	vec3 lightDirN = normalize(lightDir);

	//If positional light selected
    if (lightType == 1) {
        lightDirN  = normalize(lightPosition - fragPosLightSpace.xyz);
		float distance = length(lightPosition - fragPosLightSpace.xyz);
		//for a distance of 7
		float constant = 1.0f;
		float liniar = 0.7f;
		float quadratic = 1.8f;
		attenuation = 1.0f/(constant + liniar * distance + quadratic*(distance * distance));
    }
	
	//compute view direction 
	vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
		
	//compute ambient light
	ambient = ambientStrength * lightColor;
	
	//compute diffuse light
	diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;
	
	//compute specular light
	vec3 reflection = reflect(-lightDirN, normalEye);
	float specCoeff = pow(max(dot(viewDirN, reflection), 0.0f), shininess);
	specular = specularStrength * specCoeff * lightColor;
}

float computeShadow(){
	vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	normalizedCoords = normalizedCoords * 0.5f + 0.5f;

	float closestDepth = texture(shadowMap, normalizedCoords.xy).r;
	float currentDepth = normalizedCoords.z;
	
	float bias = 0.005f;
	//float shadow = (currentDepth - bias > closestDepth)? 1.0f : 0.0f;
	
	float shadow=0.0f;
	//PCF
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, normalizedCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;

	if(normalizedCoords.z > 1.0f ){
		return 0;
	}

	return shadow;
}

void main() 
{
	computeLightComponents();
	
	vec3 baseColor = vec3(0.9f, 0.35f, 0.0f);//orange
	
	ambient *= texture(diffuseTexture, fTexCoords).rgb * attenuation;
	diffuse *= texture(diffuseTexture, fTexCoords).rgb * attenuation;
	specular *= texture(specularTexture, fTexCoords).rgb * attenuation;

	float shadow = computeShadow();
	
	vec3 color = min((ambient + (1.0f - shadow) * diffuse) + (1.0f - shadow) * specular, 1.0f);
    
    //fColor = vec4(color, 1.0f);
	fColor = mix(vec4(color, 1.0f),vec4(fogColor,1.0f),getFogFactor());
}
