#version 150

in vec3 position;
in vec3 center;
in vec3 right;
in vec3 left;
in vec3 up;
in vec3 down;

in vec4 color;
out vec4 col;

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform float scale;
uniform float exponent;
uniform int mode;

void main()
{
  // compute the transformed and projected vertex position (into gl_Position) 
  // compute the vertex color (into col)
  if (mode == 0){
    gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0f);
    col = color;
  } else {
    
    col = color;
    // col.x = pow(color.x, exponent);
    // col.y = pow(color.y, exponent);
    // col.z = pow(color.z, exponent);

    float a = 0.0;
    float x = col.x;
    if (x < 0.0) {
      col.x = 0;
      col.y = 0;
      col.z = 0;
    } else if (x < 0.125) {
      a = x / 0.125;
      col.x = 0;
      col.y = 0;
      col.z = 0.5 + 0.5 * a;
    } else if (x < 0.375){
      a = (x - 0.125) / 0.25;
      col.x = 0;
      col.y = a;
      col.z = 1;
    } else if (x < 0.625) {
      a = (x - 0.375) / 0.25;
      col.x = a;
      col.y = 1;
      col.z = 1 - a;
    } else if (x < 0.875) {
      a = (x - 0.625) / 0.25;
      col.x = 1;
      col.y = 1 - a;
      col.z = 0;
    } else if (x <= 1.0) {
      a = (x - 0.875) / 0.125;
      col.x = 1 - 0.5 * a;;
      col.y = 0;
      col.z = 0;
    } else {
      col.x = 1;
      col.y = 1;
      col.z = 1;
    }

    col.x = pow(col.x, exponent);
    col.y = pow(col.y, exponent);
    col.z = pow(col.z, exponent);

    vec3 avg_pos = (center + left + up + down + right) / 5;
    avg_pos.y = scale * pow(avg_pos.y, exponent);
    gl_Position = projectionMatrix * modelViewMatrix * vec4(avg_pos, 1.0f);
  }
}

