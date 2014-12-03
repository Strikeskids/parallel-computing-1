#version 330

layout(location=0) in vec2 position;

smooth out vec2 complexPosition;

uniform vec4 zoomRect;

void main() {
	gl_Position = vec4(position.x*2-1, position.y*2-1, 0, 1);
	complexPosition = (position - vec2(0.5, 0.5)) * zoomRect.zw + zoomRect.xy;
}

