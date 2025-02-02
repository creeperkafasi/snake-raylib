#version 330

out vec4 fragColor;

uniform mat4 cameraTransform;
uniform vec2 screenSize;
uniform float gridSize = 50.0;

void main() {
    vec2 actual = gl_FragCoord.xy;
    actual.y = screenSize.y - actual.y;
    vec4 worldPos = inverse(cameraTransform) * vec4(actual, 0.0, 1.0);

    float lineX = mod(worldPos.x, gridSize) < 4.0 ? 1.0 : 0.0;
    float lineY = mod(worldPos.y, gridSize) < 4.0 ? 1.0 : 0.0;
    
    float grid = lineX * lineY;
    fragColor = vec4(0.2, 0.2, 0.3, grid);
}
