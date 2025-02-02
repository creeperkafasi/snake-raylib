#version 330

layout (location = 0) in vec2 vertexPosition;

uniform mat4 mvp;

void main() {
    gl_Position = mvp * vec4(vertexPosition, 0.0, 1.0);
} 
