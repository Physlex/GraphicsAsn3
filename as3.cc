#include <Angel.h>
#include <iostream>

using namespace std;

//----------------------------------------------------------------------------

// locations to pass the normal matrix and the normal vector of each face
GLint normalMatLoc, normalVecLoc;

//----------------------------------------------------------------------------
GLfloat theta_x = 0.0, theta_y = 0.0, theta_z = 0.0;

vec4 pos(0.0, 0.0, 0.0, 0.0);
vec3 scale(1.0, 1.0, 1.0);
GLint faceColourLoc, modelLoc, viewLoc, projLoc;
GLuint vao[6], buffer[6];

bool use_perspective = false;
GLfloat camera_theta = 3*M_PI/2, camera_radius = 1.0;

const vec4 point[8] = {
  vec4(-0.5, -0.5, -0.5, 1), vec4(0.5, -0.5, -0.5, 1),
  vec4(0.5, 0.5, -0.5, 1), vec4(-0.5, 0.5, -0.5, 1),
  vec4(-0.25, -0.25, 0.5, 1), vec4(0.25, -0.25, 0.5, 1),
  vec4(0.25, 0.25, 0.5, 1), vec4(-0.25, 0.25, 0.5, 1)
};

// use this as the reflectance value for each face
const vec3 colour[6] = {
  vec3(1,0,0), // red
  vec3(1,1,0), // yellow
  vec3(0,1,0), // green
  vec3(0,1,1), // cyan
  vec3(0,0,1), // blue
  vec3(1,0,1)  // magenta
};

const int face[6][4] = {
  {0,3,2,1},  // bot
  {4,5,6,7},  // top
  {0,1,5,4},  // back
  {2,3,7,6},  // front
  {2,6,5,1},  // left
  {0,4,7,3}   // right
};

vec3 normalvec[6];

vec3 compute_normal(int face_i)
{
  // compute the normal vector of each face
  vec4 vertexInitial = point[face[face_i][0]];
  vec4 farOne = point[face[face_i][1]];
  vec4 farTwo = point[face[face_i][2]];

  vec3 final = cross((vertexInitial - farOne), (vertexInitial - farTwo));
  return final;
}

void setLighting(GLuint program)
{
  GLint lightPosLoc, lightAmbLoc, lightDiffLoc, lightSpecLoc, shininessLoc;

  // replace "x" below by appropriate values
  const vec4 lightPos(1, 1, 1, 1);
  const vec3 lightAmbient(0.2, 0.2, 0.2);
  const vec3 lightDiffuse(1.0, 1.0, 1.0);
  const vec3 lightSpecular(1.0, 1.0, 1.0);
  const GLfloat shininess = 50;
  
  lightPosLoc =  glGetUniformLocation(program, "uLightPos");
  lightAmbLoc =  glGetUniformLocation(program, "uLightAmbient");
  lightDiffLoc =  glGetUniformLocation(program, "uLightDiffuse");
  lightSpecLoc =  glGetUniformLocation(program, "uLightSpecular");
  shininessLoc =  glGetUniformLocation(program, "uShininess");

  // load light material properties into uniform variables since they
  // do not change for each face
  glUniform4fv(lightPosLoc, 1, lightPos);
  glUniform3fv(lightAmbLoc, 1, lightAmbient);
  glUniform3fv(lightDiffLoc, 1, lightDiffuse);
  glUniform3fv(lightSpecLoc, 1, lightSpecular);
  glUniform1f(shininessLoc, shininess);
}

void init()
{
  // Load shaders and use the resulting shader program
  GLuint program = InitShader("vshader.glsl", "fshader.glsl");
  glUseProgram(program);
  faceColourLoc = glGetUniformLocation(program, "uFaceColour");
  modelLoc = glGetUniformLocation(program, "uModel");
  viewLoc = glGetUniformLocation(program, "uView");
  projLoc = glGetUniformLocation(program, "uProjection");

  // Initialize the vertex position attribute from the vertex shader
  GLuint loc = glGetAttribLocation( program, "vPosition" );

  normalMatLoc = glGetUniformLocation(program, "uNormalMatrix");
  normalVecLoc = glGetUniformLocation(program, "uNormalVec");

  setLighting(program);
  
  // Create a vertex array object for each face
  glGenVertexArrays(6, vao);
  glGenBuffers(6, buffer);

  for (int i = 0; i < 6; i++) {
    glBindVertexArray(vao[i]);
  
    // initialize a buffer object
    glBindBuffer(GL_ARRAY_BUFFER, buffer[i]);

    vec4 A[4];
    for (int j = 0; j < 4; j++) {
      A[j] = point[face[i][j]];
    }
    glBufferData(GL_ARRAY_BUFFER, sizeof(A), A, GL_STATIC_DRAW);

    glEnableVertexAttribArray( loc );
    glVertexAttribPointer( loc, 4, GL_FLOAT, GL_FALSE, 0,
			   BUFFER_OFFSET(0) );

    normalvec[i] = compute_normal(i);
  }
  
  glClearColor( 0.0, 0.0, 0.0, 1.0 ); // black background

  glEnable(GL_DEPTH_TEST);
  glClearDepth(1.0);
  glDepthFunc(GL_LESS);
}


//----------------------------------------------------------------------------

mat4 get_projection(mat4 modelview)
{
  GLfloat z1 = 1e10, z2 = -1e10;
  
  for (int i = 0; i < 8; i++) {
    auto p = modelview * point[i];
    z1 = min(z1, -p.z);
    z2 = max(z2, -p.z);
  }

  GLfloat near, far;

  if (use_perspective) {
    near = z1 - 0.01;
    far = z2 + 0.01;
    return Perspective(90, 1.0, near, far);
  } else {
    near = z1 - 0.5;
    far = z2 + 0.5;
    return Ortho(-1.0, 1.0, -1.0, 1.0, near, far);
  }
}


void display( void )
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  mat4 rotate = RotateX(theta_x) * RotateY(theta_y) * RotateZ(theta_z);
  mat4 model = Translate(pos) * rotate * Scale(scale);

  GLfloat eye_x = camera_radius * cos(camera_theta);
  GLfloat eye_y = camera_radius * sin(camera_theta);
  vec4 eye(eye_x, eye_y, 0, 1);
  vec4 at(0,0,0,1);
  vec4 up(0,0,1,0);
  mat4 view = LookAt(eye, at, up);
  mat4 modelview = view * model;

  // need to compute the following
  mat3 normalMat = Normal(modelview);
  
  mat4 proj = get_projection(modelview);

  glUniformMatrix4fv(modelLoc, 1, GL_TRUE, model);
  glUniformMatrix4fv(viewLoc, 1, GL_TRUE, view);
  glUniformMatrix3fv(normalMatLoc, 1, GL_TRUE, normalMat);
  glUniformMatrix4fv(projLoc, 1, GL_TRUE, proj);
  
  for (int i = 0; i < 6; i++) {
    glUniform3fv(faceColourLoc, 1, colour[i]);
    glUniform3fv(normalVecLoc, 1, normalvec[i]);
    glBindVertexArray(vao[i]);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);    // draw the square
  }
  glutSwapBuffers();
}

//----------------------------------------------------------------------------

void
keyboard( unsigned char key, int x, int y )
{
  switch ( key ) {
  case 033:
    exit( EXIT_SUCCESS );
    break;
  case 'p':
    use_perspective = !use_perspective;
    glutPostRedisplay();
    break;
  }
}

void arrow(int key, int x, int y)
{
  switch (key) {
  case GLUT_KEY_LEFT:
    pos -= vec4(0.05, 0, 0, 0);
    break;
  case GLUT_KEY_RIGHT:
    pos += vec4(0.05, 0, 0, 0);
    break;
  case GLUT_KEY_UP:
    pos += vec4(0, 0, 0.05, 0);
    break;
  case GLUT_KEY_DOWN:
    pos -= vec4(0, 0, 0.05, 0);
    break;
  case GLUT_KEY_PAGE_UP:
    scale *= 1.05;
    break;
  case GLUT_KEY_PAGE_DOWN:
    scale *= 1.0/1.05;
    break;
  case GLUT_KEY_F4: //GLUT_KEY_HOME
    cout << "ROTATE" << endl;
    theta_x += 0.5;
    break;
  case GLUT_KEY_END:
    theta_y += 0.5;
    break;
  case GLUT_KEY_INSERT:
    theta_z += 0.5;
    break;
  }
  glutPostRedisplay();
}

void mouse_button(int button, int state, int x, int y)
{
  if (state == GLUT_DOWN) {
    switch (button) {
    case GLUT_LEFT_BUTTON:
      camera_theta -= 0.2;
      break;
    case GLUT_RIGHT_BUTTON:
      camera_theta += 0.2;
      break;
    case 3:
      camera_radius -= 0.1;
      if (camera_radius < 1.0) {
	camera_radius = 1.0;
      }
      break;
    case 4:
      camera_radius += 0.1;
      break;
    }
    glutPostRedisplay();
  }
}

// note: on some platform the following doesn't work
void mouse_wheel(int wheel, int dir, int x, int y)
{
  if (dir > 0) {
    camera_radius -= 0.1;
    if (camera_radius < 1.0)
      camera_radius = 1.0;
  } {
    camera_radius += 0.1;
  }
  glutPostRedisplay();
}

//----------------------------------------------------------------------------

int main( int argc, char **argv )
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
  glutInitWindowSize(512, 512);

  // If you are using freeglut, the next two lines will check if 
  // the code is truly 3.2. Otherwise, comment them out
    
  glutInitContextVersion(3, 2);
  glutInitContextProfile(GLUT_CORE_PROFILE);

  glutCreateWindow("Rotating Frustrum");

  glewInit();

  init();

  glutDisplayFunc(display);
  glutKeyboardFunc(keyboard);
  glutSpecialFunc(arrow);
  glutMouseFunc(mouse_button);
  glutMouseWheelFunc(mouse_wheel);
  
  glutMainLoop();
  return 0;
}
