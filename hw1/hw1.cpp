/*
  CSCI 420 Computer Graphics, Computer Science, USC
  Assignment 1: Height Fields with Shaders.
  C/C++ starter code

  Student username: <psrathod>
*/

#include "openGLHeader.h"
#include "glutHeader.h"
#include "openGLMatrix.h"
#include "imageIO.h"
#include "pipelineProgram.h"
#include "vbo.h"
#include "vao.h"

#include <chrono>
#include <ctime>
#include <iomanip>

#include <iostream>
#include <cstring>
#include <vector>

#if defined(WIN32) || defined(_WIN32)
  #ifdef _DEBUG
    #pragma comment(lib, "glew32d.lib")
  #else
    #pragma comment(lib, "glew32.lib")
  #endif
#endif

#if defined(WIN32) || defined(_WIN32)
  char shaderBasePath[1024] = SHADER_BASE_PATH;
#else
  char shaderBasePath[1024] = "../openGLHelper";
#endif

using namespace std;

int mousePos[2]; // x,y screen coordinates of the current mouse position

int leftMouseButton = 0; // 1 if pressed, 0 if not 
int middleMouseButton = 0; // 1 if pressed, 0 if not
int rightMouseButton = 0; // 1 if pressed, 0 if not

typedef enum { ROTATE, TRANSLATE, SCALE } CONTROL_STATE;
CONTROL_STATE controlState = ROTATE;

// Transformations of the terrain.
float terrainRotate[3] = { 0.0f, 0.0f, 0.0f }; 
// terrainRotate[0] gives the rotation around x-axis (in degrees)
// terrainRotate[1] gives the rotation around y-axis (in degrees)
// terrainRotate[2] gives the rotation around z-axis (in degrees)
float terrainTranslate[3] = { 0.0f, 0.0f, 0.0f };
float terrainScale[3] = { 1.0f, 1.0f, 1.0f };

// Width and height of the OpenGL window, in pixels.
int windowWidth = 1280;
int windowHeight = 720;
char windowTitle[512] = "CSCI 420 Homework 1";

// Stores the image loaded from disk.
ImageIO * heightmapImage;

// Number of vertices in the single triangle (starter code).
int numVertices;
int dispMode = 1; // default display points
int mode = 0;
float scale = 1.0f;
float exponent = 1.0f;

// CSCI 420 helper classes.
OpenGLMatrix matrix;
PipelineProgram * pipelineProgram = nullptr;
VBO * vboVertices = nullptr;
VBO * vboColors = nullptr;
VAO * vao = nullptr;

int numLineVertices;
VBO * vboLinesVertices = nullptr;
VBO * vboLinesColors = nullptr;
VAO * vaoLines = nullptr;

int numTrigVertices;
VBO * vboTrigVertices = nullptr;
VBO * vboTrigColors = nullptr;
VAO * vaoTrig = nullptr;

int numTrigSmooth;
VBO * vboCenterVertices = nullptr;
VBO * vboRightVertices = nullptr;
VBO * vboLeftVertices = nullptr;
VBO * vboUpVertices = nullptr;
VBO * vboDownVertices = nullptr;
VBO * vboSmoothColors = nullptr;
VAO * vaoSmooth = nullptr;

int countSS = 0;
int frameCount = 0;


// Write a screenshot to the specified filename.
void saveScreenshot(const char * filename)
{
  unsigned char * screenshotData = new unsigned char[windowWidth * windowHeight * 3];
  glReadPixels(0, 0, windowWidth, windowHeight, GL_RGB, GL_UNSIGNED_BYTE, screenshotData);

  ImageIO screenshotImg(windowWidth, windowHeight, 3, screenshotData);

  if (screenshotImg.save(filename, ImageIO::FORMAT_JPEG) == ImageIO::OK)
    cout << "File " << filename << " saved successfully." << endl;
  else cout << "Failed to save file " << filename << '.' << endl;

  delete [] screenshotData;
}

void idleFunc()
{
  // Do some stuff... 
  // For example, here, you can save the screenshots to disk (to make the animation).
  
  // Notify GLUT that it should call displayFunc.
  // commented out code to take 300 screenshots every (1 screenshot per 10 frames).
  // if (frameCount <= 3000){
  //   frameCount++;
  //     if (frameCount % 10 == 0){
  //     // evenly distribute time between each mode with 75 frames per mode.
  //     if (countSS == 75){
  //       dispMode = 2;
  //     } else if (countSS == 150){
  //       dispMode =3;
  //     } else if (countSS == 225){
  //       dispMode = 4;
  //       mode = 1;
  //     }
  //     char buf [9];
  //     sprintf(buf, "%03d.jpeg", countSS);

  //     // cout << buf << endl;
  //     // std::string fname = "" + countSS + ".jpeg";
  //     saveScreenshot(buf);
  //     countSS++;
  //   }
  // }

  glutPostRedisplay();
}

void reshapeFunc(int w, int h)
{
  glViewport(0, 0, w, h);

  // When the window has been resized, we need to re-set our projection matrix.
  matrix.SetMatrixMode(OpenGLMatrix::Projection);
  matrix.LoadIdentity();
  // You need to be careful about setting the zNear and zFar. 
  // Anything closer than zNear, or further than zFar, will be culled.
  const float zNear = 0.1f;
  const float zFar = 10000.0f;
  const float humanFieldOfView = 60.0f;
  matrix.Perspective(humanFieldOfView, 1.0f * w / h, zNear, zFar);
}

void mouseMotionDragFunc(int x, int y)
{
  // Mouse has moved, and one of the mouse buttons is pressed (dragging).

  // the change in mouse position since the last invocation of this function
  int mousePosDelta[2] = { x - mousePos[0], y - mousePos[1] };

  switch (controlState)
  {
    // translate the terrain
    case TRANSLATE:
      if (leftMouseButton)
      {
        // control x,y translation via the left mouse button
        terrainTranslate[0] += mousePosDelta[0] * 0.01f;
        terrainTranslate[1] -= mousePosDelta[1] * 0.01f;
      }
      if (middleMouseButton)
      {
        // control z translation via the middle mouse button
        terrainTranslate[2] += mousePosDelta[1] * 0.01f;
      }
      break;

    // rotate the terrain
    case ROTATE:
      if (leftMouseButton)
      {
        // control x,y rotation via the left mouse button
        terrainRotate[0] += mousePosDelta[1];
        terrainRotate[1] += mousePosDelta[0];
      }
      if (middleMouseButton)
      {
        // control z rotation via the middle mouse button
        terrainRotate[2] += mousePosDelta[1];
      }
      break;

    // scale the terrain
    case SCALE:
      if (leftMouseButton)
      {
        // control x,y scaling via the left mouse button
        terrainScale[0] *= 1.0f + mousePosDelta[0] * 0.01f;
        terrainScale[1] *= 1.0f - mousePosDelta[1] * 0.01f;
      }
      if (middleMouseButton)
      {
        // control z scaling via the middle mouse button
        terrainScale[2] *= 1.0f - mousePosDelta[1] * 0.01f;
      }
      break;
  }

  // store the new mouse position
  mousePos[0] = x;
  mousePos[1] = y;
}

void mouseMotionFunc(int x, int y)
{
  // Mouse has moved.
  // Store the new mouse position.
  mousePos[0] = x;
  mousePos[1] = y;
}

void mouseButtonFunc(int button, int state, int x, int y)
{
  // A mouse button has has been pressed or depressed.

  // Keep track of the mouse button state, in leftMouseButton, middleMouseButton, rightMouseButton variables.
  switch (button)
  {
    case GLUT_LEFT_BUTTON:
      leftMouseButton = (state == GLUT_DOWN);
    break;

    case GLUT_MIDDLE_BUTTON:
      middleMouseButton = (state == GLUT_DOWN);
    break;

    case GLUT_RIGHT_BUTTON:
      cout << "RIGHT CLICK" << endl;
      rightMouseButton = (state == GLUT_DOWN);
    break;
  }

  // Keep track of whether CTRL and SHIFT keys are pressed.
  switch (glutGetModifiers())
  {
    case GLUT_RIGHT_BUTTON:
      // cout << "TRANSLATE" << endl;
      controlState = TRANSLATE;
    break;

    case GLUT_ACTIVE_SHIFT:
      controlState = SCALE;
    break;

    // If CTRL and SHIFT are not pressed, we are in rotate mode.
    default:
      controlState = ROTATE;
    break;
  }

  // Store the new mouse position.
  mousePos[0] = x;
  mousePos[1] = y;
}

void keyboardFunc(unsigned char key, int x, int y)
{
  switch (key)
  {
    case 27: // ESC key
      exit(0); // exit the program
    break;

    case '1':
      // cout << "Pressed 1 for Points" << endl;
      dispMode = 1;
      mode = 0;
      break;
    
    case '2':
      dispMode = 2;
      mode = 0;
      // cout << "Pressed 2 for Wirframe / Lines" << endl;
      break;
    
    case '3':
      dispMode = 3;
      mode = 0;
      // cout << "Pressed 3 for Solid Triangles" << endl;
      break;
    case '4':
      dispMode = 4;
      mode = 1;
      // cout << "Pressed 4" << endl;
      break;
    
    case '+':
      if (mode == 1){
        scale *= 2;
      }
      break;
    case '-':
      if (mode == 1){
        scale /= 2;
      }
      break;
    case '9':
      if (mode == 1){
        exponent *= 2;
      }
      break;
    case '0':
      if (mode == 1){
        exponent /= 2;
      }
      break;
    case ' ':
      cout << "You pressed the spacebar." << endl;
    break;
    case 't':
      // cout << "TRANSLATE" << endl;
      controlState = TRANSLATE;
      break;

    case 'x':
      // Take a screenshot.
      saveScreenshot("screenshot.jpg");
    break;
  }
}

// used in modes 1,2,3 scale the height with a constant of 0.2
void addVertex(int i, int j, std::vector<float>& positions, std::vector<float>& colors){
  int w = heightmapImage->getWidth(); 
  int h = heightmapImage->getHeight();
  float x = (1.0 * i / ((w) - 1));
  float y = (heightmapImage->getPixel(i, j, 0) / 255.0f);
  // double y = (heightmapImage->getPixel(i, j, 0) / 255.0f);
  float z = (-1.0 * j / ((h) - 1));
  // cout << x << " " << y << " " << z << endl;
  positions.push_back(x);
  positions.push_back(0.2 * y);
  positions.push_back(z);
  
  colors.push_back(y);
  colors.push_back(y);
  colors.push_back(y);
  colors.push_back(y);

}

// this function is seperate because we arenot scaling the height with a constant
void addSmoothVert(int i, int j, int w, int h, vector<float>& positions, vector<float> & colors){
  float x = (1.0 * i / ((w) - 1));
  float y = (heightmapImage->getPixel(i, j, 0) / 255.0f);
  // double y = (heightmapImage->getPixel(i, j, 0) / 255.0f);
  float z = (-1.0 * j / ((h) - 1));
  positions.push_back(x);
  positions.push_back(y);
  positions.push_back(z);
}

// based on the center add center, left, right, up down vertices
void addCenterPos(int i, int j, vector<float>& center, vector<float>& right, vector<float>& left, vector<float>& up, vector<float>& down, vector<float>& colors ){
  int w = heightmapImage->getWidth(); 
  int h = heightmapImage->getHeight();
  
  addSmoothVert (i, j, w, h, center, colors);

  // check if vertex below is within bounds of the image if not replace with the center
  if (i - 1 < 0){
    addSmoothVert (i, j, w, h, down, colors);
  } else {
    addSmoothVert (i-1, j, w, h, down, colors);
  }
  // check if vertex within is within bounds of the image if not replace with the center
  if (i + 1 >= h){
    addSmoothVert (i, j, w, h, up, colors);
  } else {
    addSmoothVert (i+1, j, w, h, up, colors);
  }

  // check if vertex to the left is within bounds of the image if not replace with the center
  if (j - 1 < 0){
    addSmoothVert (i, j, w, h, left, colors);
  } else {
    addSmoothVert (i, j-1, w, h, left, colors);
  }

  // check if vertex to the right is within bounds of the image if not replace with the center
  if (j + 1 >= w){
    addSmoothVert (i, j, w, h, right, colors);
  } else {
    addSmoothVert (i, j+1, w, h, right, colors);
  }

  float color = (heightmapImage->getPixel(i, j, 0) / 255.0f);
  colors.push_back(color);
  colors.push_back(color);
  colors.push_back(color);
  colors.push_back(color);
  
}

// intepolate and add triangles to 5 vbos for positions and 1 for colors
void initSmoothArray(){

  // 5 separate vectors for 5 vbos
  std::vector<float> center;
  std::vector<float> left;
  std::vector<float> right;
  std::vector<float> up;
  std::vector<float> down;
  std::vector<float> colors;

  // interploating through all the pixels and passing the i,j index to an intermediate function
  // intermediate function will pushback the coordinates of center, left, right, up and down.
  int w = heightmapImage->getWidth(); 
  int h = heightmapImage->getHeight();
  for (int i = 0; i < h-1; i++){
    for (int j = 0; j < w-1; j++){
      addCenterPos (i, j, center, right, left, up, down, colors);
      addCenterPos (i, j+1, center, right, left, up, down, colors);
      addCenterPos (i+1, j, center, right, left, up, down, colors);

      addCenterPos (i+1, j, center, right, left, up, down, colors);
      addCenterPos (i+1, j+1, center, right, left, up, down, colors);
      addCenterPos (i, j+1, center, right, left, up, down, colors);
    }
  }
  // cout << "Center : " << center.size()/3 << endl;
  // cout << "Right : " << right.size()/3 << endl;
  // cout << "Left : " << left.size()/3 << endl;
  // cout << "Up : " << up.size()/3 << endl;
  // cout << "Down : " << down.size()/3 << endl;
  // cout << "Colors : " << colors.size()/4 << endl;

  vboCenterVertices = new VBO(center.size()/3, 3, center.data(), GL_STATIC_DRAW); // 3 values per position
  vboLeftVertices = new VBO(left.size()/3, 3, left.data(), GL_STATIC_DRAW);
  vboRightVertices = new VBO(right.size()/3, 3, right.data(), GL_STATIC_DRAW);
  vboUpVertices = new VBO(up.size()/3, 3, up.data(), GL_STATIC_DRAW);
  vboDownVertices = new VBO(down.size()/3, 3, down.data(), GL_STATIC_DRAW);
  

  vboSmoothColors = new VBO(colors.size()/4, 4, colors.data(), GL_STATIC_DRAW); // 4 values per color
  vaoSmooth = new VAO();
  vaoSmooth->ConnectPipelineProgramAndVBOAndShaderVariable(pipelineProgram, vboCenterVertices, "center");
  vaoSmooth->ConnectPipelineProgramAndVBOAndShaderVariable(pipelineProgram, vboLeftVertices, "left");
  vaoSmooth->ConnectPipelineProgramAndVBOAndShaderVariable(pipelineProgram, vboRightVertices, "right");
  vaoSmooth->ConnectPipelineProgramAndVBOAndShaderVariable(pipelineProgram, vboUpVertices, "up");
  vaoSmooth->ConnectPipelineProgramAndVBOAndShaderVariable(pipelineProgram, vboDownVertices, "down");
  vaoSmooth->ConnectPipelineProgramAndVBOAndShaderVariable(pipelineProgram, vboSmoothColors, "color");
  
}

// creating VBOs and VAO for triangle mode (3 on keyboard)
void initTrigArray(std::vector<float>& positions, std::vector<float>& colors){
  int w = heightmapImage->getWidth(); 
  int h = heightmapImage->getHeight();
  // numVertices = (4 * w * h) - (2 * w) - (2 * h); // Each point connects to the next one, so we need 2 vertices per line segment

  // 6 vertices when adding traingles 
  positions.clear();
  colors.clear();
  for (int i = 0; i < h-1; i++){
    for (int j = 0; j < w-1; j++){
      addVertex(i, j, positions, colors);
      addVertex(i, j+1, positions, colors);
      addVertex(i+1, j, positions, colors);

      addVertex(i+1, j, positions, colors);
      addVertex(i+1, j+1, positions, colors);
      addVertex(i, j+1, positions, colors);

    }
  }
  numTrigVertices = positions.size()/3;

  // cout << "NUM Vertices " << numVertices << " Positions " << positions.size()/3 << endl;
  vboTrigVertices = new VBO(numTrigVertices, 3, positions.data(), GL_STATIC_DRAW); // 3 values per position
  vboTrigColors = new VBO(numTrigVertices, 4, colors.data(), GL_STATIC_DRAW); // 4 values per color

  vaoTrig = new VAO();
  vaoTrig->ConnectPipelineProgramAndVBOAndShaderVariable(pipelineProgram, vboTrigVertices, "position");
  vaoTrig->ConnectPipelineProgramAndVBOAndShaderVariable(pipelineProgram, vboTrigColors, "color");
}
// creating VBOs and VAO for lines/wireframe mode (2 on keyboard)
void initLinesArray(std::vector<float>& positions, std::vector<float>& colors){
  int w = heightmapImage->getWidth(); 
  int h = heightmapImage->getHeight();
  // numVertices = (4 * w * h) - (2 * w) - (2 * h); // Each point connects to the next one, so we need 2 vertices per line segment


  positions.clear();
  colors.clear();
  for (int i = 0; i < h-1; i++){
    for (int j = 0; j < w-1; j++){
      addVertex(i, j, positions, colors);
      addVertex(i, j+1, positions, colors);
      addVertex(i, j, positions, colors);
      addVertex(i+1, j, positions, colors);

    }
  }
  numLineVertices = positions.size()/3;
  // cout << "NUM Vertices " << numVertices << " Positions " << positions.size()/3 << endl;
  vboLinesVertices = new VBO(numLineVertices, 3, positions.data(), GL_STATIC_DRAW); // 3 values per position
  vboLinesColors = new VBO(numLineVertices, 4, colors.data(), GL_STATIC_DRAW); // 4 values per color

  vaoLines = new VAO();
  vaoLines->ConnectPipelineProgramAndVBOAndShaderVariable(pipelineProgram, vboLinesVertices, "position");
  vaoLines->ConnectPipelineProgramAndVBOAndShaderVariable(pipelineProgram, vboLinesColors, "color");
}

// creating VBOs and VAO for points mode (1 on keyboard)
void initPointsArray(std::vector<float>& positions, std::vector<float>& colors) {
  int w = heightmapImage->getWidth(); 
  int h = heightmapImage->getHeight();
  numVertices = w * h;
  // float * positions = (float*) malloc (numVertices * 3 * sizeof(float));
  // float * colors = (float*) malloc (numVertices * 4 * sizeof(float));
  int count = 0;
  int count2 = 0;
  for (int i = 0; i < w; i++){
    for (int j = 0; j < h; j++){
      addVertex(i, j, positions, colors);
    }
  }

  vboVertices = new VBO(numVertices, 3, positions.data(), GL_STATIC_DRAW); // 3 values per position
  vboColors = new VBO(numVertices, 4, colors.data(), GL_STATIC_DRAW); // 4 values per color
  vao = new VAO();

  // Set up the relationship between the "position" shader variable and the VAO.
  // Important: any typo in the shader variable name will lead to malfunction.
  vao->ConnectPipelineProgramAndVBOAndShaderVariable(pipelineProgram, vboVertices, "position");

  // Set up the relationship between the "color" shader variable and the VAO.
  // Important: any typo in the shader variable name will lead to malfunction.
  vao->ConnectPipelineProgramAndVBOAndShaderVariable(pipelineProgram, vboColors, "color");
}

void displayFunc()
{
  // This function performs the actual rendering.

  // First, clear the screen.
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Set up the camera position, focus point, and the up vector.
  matrix.SetMatrixMode(OpenGLMatrix::ModelView);
  matrix.LoadIdentity();
  // matrix.LookAt(0.0, 0.0, 5.0,
  //               0.0, 0.0, 0.0,
  //               0.0, 1.0, 0.0);
  matrix.LookAt(0.5, 1.5, 0.3,
                0.5, 0.5, -0.5,
                0.0, 1.0, 0.0);

  // In here, you can do additional modeling on the object, such as performing translations, rotations and scales.
  // ...
  matrix.Rotate(terrainRotate[0],1, 0, 0);
  matrix.Rotate(terrainRotate[1],0, 1, 0);
  matrix.Rotate(terrainRotate[2],0, 0, 1);

  matrix.Scale(terrainScale[0], terrainScale[1], terrainScale[2]);

  matrix.Translate(terrainTranslate[0], terrainTranslate[1], terrainTranslate[2]);

  
  

  // Read the current modelview and projection matrices from our helper class.
  // The matrices are only read here; nothing is actually communicated to OpenGL yet.
  float modelViewMatrix[16];
  matrix.SetMatrixMode(OpenGLMatrix::ModelView);
  matrix.GetMatrix(modelViewMatrix);

  float projectionMatrix[16];
  matrix.SetMatrixMode(OpenGLMatrix::Projection);
  matrix.GetMatrix(projectionMatrix);

  // Upload the modelview and projection matrices to the GPU. Note that these are "uniform" variables.
  // Important: these matrices must be uploaded to *all* pipeline programs used.
  // In hw1, there is only one pipeline program, but in hw2 there will be several of them.
  // In such a case, you must separately upload to *each* pipeline program.
  // Important: do not make a typo in the variable name below; otherwise, the program will malfunction.
  pipelineProgram->SetUniformVariableMatrix4fv("modelViewMatrix", GL_FALSE, modelViewMatrix);
  pipelineProgram->SetUniformVariableMatrix4fv("projectionMatrix", GL_FALSE, projectionMatrix);
  pipelineProgram->SetUniformVariablef("scale", scale);
  pipelineProgram->SetUniformVariablef("exponent", exponent);
  pipelineProgram->SetUniformVariablei("mode", mode);
  
  // Execute the rendering.
  // Bind the VAO that we want to render. Remember, one object = one VAO. 
  if (dispMode == 1){
    // display points
    vao->Bind();
    glDrawArrays(GL_POINTS, 0, numVertices); // Render the VAO, by rendering "numVertices", starting from vertex 0.
  } else if (dispMode == 2){
    // display lines/wireframe
    vaoLines->Bind();
    glDrawArrays(GL_LINES, 0, numLineVertices);
  } else if (dispMode == 3){
    // display triangles
    vaoTrig->Bind();
    glDrawArrays(GL_TRIANGLES, 0, numTrigVertices);
  } else if (dispMode == 4){
    // smooth mode 
    vaoSmooth->Bind();
    glDrawArrays(GL_TRIANGLES, 0, numTrigVertices);
  }

  // Swap the double-buffers.
  glutSwapBuffers();
}

void initScene(int argc, char *argv[])
{
  // Load the image from a jpeg disk file into main memory.
  heightmapImage = new ImageIO();
  if (heightmapImage->loadJPEG(argv[1]) != ImageIO::OK)
  {
    cout << "Error reading image " << argv[1] << "." << endl;
    exit(EXIT_FAILURE);
  }

  // Set the background color.
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black color.

  // Enable z-buffering (i.e., hidden surface removal using the z-buffer algorithm).
  glEnable(GL_DEPTH_TEST);

  // Create a pipeline program. This operation must be performed BEFORE we initialize any VAOs.
  // A pipeline program contains our shaders. Different pipeline programs may contain different shaders.
  // In this homework, we only have one set of shaders, and therefore, there is only one pipeline program.
  // In hw2, we will need to shade different objects with different shaders, and therefore, we will have
  // several pipeline programs (e.g., one for the rails, one for the ground/sky, etc.).
  pipelineProgram = new PipelineProgram(); // Load and set up the pipeline program, including its shaders.
  // Load and set up the pipeline program, including its shaders.
  if (pipelineProgram->BuildShadersFromFiles(shaderBasePath, "vertexShader.glsl", "fragmentShader.glsl") != 0)
  {
    cout << "Failed to build the pipeline program." << endl;
    throw 1;
  } 
  cout << "Successfully built the pipeline program." << endl;
    
  // Bind the pipeline program that we just created. 
  // The purpose of binding a pipeline program is to activate the shaders that it contains, i.e.,
  // any object rendered from that point on, will use those shaders.
  // When the application starts, no pipeline program is bound, which means that rendering is not set up.
  // So, at some point (such as below), we need to bind a pipeline program.
  // From that point on, exactly one pipeline program is bound at any moment of time.
  pipelineProgram->Bind();

  // Prepare the triangle position and color data for the VBO. 
  // The code below sets up a single triangle (3 vertices).
  // The triangle will be rendered using GL_TRIANGLES (in displayFunc()).

  std::vector<float> positions;
  std::vector<float> colors;

  initPointsArray(positions, colors);
  initLinesArray(positions, colors);
  initTrigArray(positions, colors);

  initSmoothArray();
  
  // Create the VBOs. 
  // We make a separate VBO for vertices and colors. 
  // This operation must be performed BEFORE we initialize any VAOs.

  // Create the VAOs. There is a single VAO in this example.
  // Important: this code must be executed AFTER we created our pipeline program, and AFTER we set up our VBOs.
  // A VAO contains the geometry for a single object. There should be one VAO per object.
  // In this homework, "geometry" means vertex positions and colors. In homework 2, it will also include
  // vertex normal and vertex texture coordinates for texture mapping.

  // // Set up the relationship between the "position" shader variable and the VAO.
  // // Important: any typo in the shader variable name will lead to malfunction.
  
  // // Set up the relationship between the "color" shader variable and the VAO.
  // // Important: any typo in the shader variable name will lead to malfunction.

  // Check for any OpenGL errors.
  std::cout << "GL error status is: " << glGetError() << std::endl;
}

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    cout << "The arguments are incorrect." << endl;
    cout << "usage: ./hw1 <heightmap file>" << endl;
    exit(EXIT_FAILURE);
  }

  cout << "Initializing GLUT..." << endl;
  glutInit(&argc,argv);

  cout << "Initializing OpenGL..." << endl;

  #ifdef __APPLE__
    glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
  #else
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
  #endif

  glutInitWindowSize(windowWidth, windowHeight);
  glutInitWindowPosition(0, 0);  
  glutCreateWindow(windowTitle);

  cout << "OpenGL Version: " << glGetString(GL_VERSION) << endl;
  cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << endl;
  cout << "Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

  #ifdef __APPLE__
    // This is needed on recent Mac OS X versions to correctly display the window.
    glutReshapeWindow(windowWidth - 1, windowHeight - 1);
  #endif

  // Tells GLUT to use a particular display function to redraw.
  glutDisplayFunc(displayFunc);
  // Perform animation inside idleFunc.
  glutIdleFunc(idleFunc);
  // callback for mouse drags
  glutMotionFunc(mouseMotionDragFunc);
  // callback for idle mouse movement
  glutPassiveMotionFunc(mouseMotionFunc);
  // callback for mouse button changes
  glutMouseFunc(mouseButtonFunc);
  // callback for resizing the window
  glutReshapeFunc(reshapeFunc);
  // callback for pressing the keys on the keyboard
  glutKeyboardFunc(keyboardFunc);

  // init glew
  #ifdef __APPLE__
    // nothing is needed on Apple
  #else
    // Windows, Linux
    GLint result = glewInit();
    if (result != GLEW_OK)
    {
      cout << "error: " << glewGetErrorString(result) << endl;
      exit(EXIT_FAILURE);
    }
  #endif

  // Perform the initialization.
  initScene(argc, argv);

  // Sink forever into the GLUT loop.
  glutMainLoop();
}

