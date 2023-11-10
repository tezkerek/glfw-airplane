#version 330 core

layout (location = 0) in vec4 in_Position;
layout (location = 1) in vec4 in_Color;

out vec4 ex_Color;

uniform float weight;
uniform vec4 targetColor;
uniform mat4 matrix;

void main(void) {
  gl_Position = matrix * in_Position;
  ex_Color = mix(in_Color, targetColor, weight);
} 