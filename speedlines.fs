#version 330 core

in vec2 fragCoord;
out vec4 finalColor;

uniform vec2 iResolution;
uniform float iTime;

#define PI 3.1415
#define RADIUS 0.7

vec3 hash(vec3 p) {
    p = vec3(dot(p, vec3(127.1, 311.7, 74.7)),
             dot(p, vec3(269.5, 183.3, 246.1)),
             dot(p, vec3(113.5, 271.9, 173.3)));
    return -1.0 + 2.0 * fract(sin(p) * 43758.5453123);
}

float smoothNoise(vec3 p) {
    vec3 i = floor(p);
    vec3 f = fract(p);
    vec3 u = f * f * (3.0 - 2.0 * f);

    float a = dot(hash(i + vec3(0.0)), f - vec3(0.0));
    float b = dot(hash(i + vec3(1.0,0.0,0.0)), f - vec3(1.0,0.0,0.0));
    float c = dot(hash(i + vec3(0.0,1.0,0.0)), f - vec3(0.0,1.0,0.0));
    float d = dot(hash(i + vec3(1.0,1.0,0.0)), f - vec3(1.0,1.0,0.0));
    float e = dot(hash(i + vec3(0.0,0.0,1.0)), f - vec3(0.0,0.0,1.0));
    float f2 = dot(hash(i + vec3(1.0,0.0,1.0)), f - vec3(1.0,0.0,1.0));
    float g = dot(hash(i + vec3(0.0,1.0,1.0)), f - vec3(0.0,1.0,1.0));
    float h = dot(hash(i + vec3(1.0,1.0,1.0)), f - vec3(1.0,1.0,1.0));

    return mix(mix(mix(a, b, u.x), mix(c, d, u.x), u.y),
               mix(mix(e, f2, u.x), mix(g, h, u.x), u.y), u.z);
}

void main() {
    // Convert to Shadertoy-like coordinates
    vec2 uv = (gl_FragCoord.xy / iResolution.xy - 0.5) * 2.0;
    
    // Main shader logic
    float noise = smoothNoise(vec3(normalize(uv) * 30.0, iTime * 10.0));
    noise = noise < 0.2 ? 0.0 : 1.0;
    
    vec4 col = vec4(noise * (length(uv) - RADIUS) / RADIUS);
    if (length(uv) < RADIUS) col = vec4(0.0);
    
    finalColor = col;
}
