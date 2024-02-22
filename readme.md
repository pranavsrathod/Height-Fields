# CSCI 420 Programming Assignment 1: Height Fields Using Shaders
#### Pranav Rathod
#### psrathod@usc.edu

## Overview
This program utilizes OpenGL's core profile to create and manipulate a height field based on user-specified image data. The height field is represented visually as a 3D terrain, and the user can perform transformations like rotation, translation, and scaling. Additionally, a vertex shader is implemented to smoothen the geometry and adjust the terrain color. The program supports rendering the height field as points, lines, or solid triangles.

## Setup

### Operating System
This assignment was completed on Apple Silicon.

### Additional Setup Instructions
For detailed setup instructions, please refer to the [assignment page](https://viterbi-web.usc.edu/~jbarbic/cs420-s24/assign1/index.html).


## Features and Controls

### Render Modes:
- Press `1` for point rendering.
- Press `2` for wireframe (line) rendering.
- Press `3` for solid triangle rendering.

### Transformation:
- Default mouse control set to rotate by the origin.
- Left-click for rotation along the y-axis and x-axis, middle mouse button for rotation along the z-axis.
- Hold down `Shift` and use the mouse for scaling.
- Hold down `t` and use the mouse for translation.

### Smoothing Mode:
- Press `4` to activate smoothing mode.
- In this mode, press `+` to double the terrain scale, `-` to halve the scale.
- Press `9` to double the exponent, `0` to halve the exponent.

## Extra Credit.
- When using key "4", Implemented the JetColorMap function in the vertex shader. Speficially, changed the grayscale color x to JetColorMap(x). 
- Note that even after color is computed with JetColorMap outputColor <--- pow(y, exponent) is applied to the resulting color.# Height-Fields

