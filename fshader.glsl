#version 150

in vec3 vPosition2;
in vec3 vNormal2;
in vec3 vLightPos2;

uniform vec3 uLightAmbient;
uniform vec3 uLightDiffuse;
uniform vec3 uLightSpecular;
uniform vec3 uFaceColour;
uniform float uShininess;

out vec4  fColour;

void
main()
{
    vec3 L = normalize(vLightPos2 - vPosition2);
    vec3 V = normalize(-vPosition2);
    vec3 H = normalize(V + L); //Equivalent to approximated 'r', not n.
    
    vec3 ambient = uLightAmbient * uFaceColour;
    vec3 diffuse = uLightDiffuse * uFaceColour * max(dot(L, vNormal2), 0.0);
    vec3 specular = uLightSpecular * uFaceColour * pow(max(dot(vNormal2, H), 0.0), uShininess * 4);
    fColour.rgb = (ambient + diffuse + specular); 
    fColour.a = 1.0;
}
