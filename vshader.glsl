#version 150

in vec4 vPosition;

// uniform means this is the same value for all vertices
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform mat3 uNormalMatrix;
uniform vec3 uNormalVec;
uniform vec4 uLightPos;

// position of the vertex in camera space
out vec3 vPosition2;

// normal vector in camera space
out vec3 vNormal2;

// light position in camera space
out vec3 vLightPos2;

void main()
{
  mat4 modelView = uView * uModel;
  vec4 vPositionMV = uView * vPosition;   // compute position in camera space
  gl_Position = uProjection * modelView * vPosition;
  vNormal2 =  uNormalMatrix * uNormalVec;
  vLightPos2 = (uView * uLightPos).xyz;

  // don't forget to do perspective division
  vPosition2 = vPositionMV.xyz / vPositionMV.w;
}
