#version 330

layout(location=0) in vec2 position;

smooth out vec2 complexPosition;

uniform vec4 zoomRect;

void main() {
	gl_Position = vec4(position.x*2-1, position.y*2-1, 0, 1);
	complexPosition = vec2(position.x*zoomRect.z+zoomRect.x, position.y*zoomRect.w+zoomRect.y);
	//complexPosition = vec2(position.x*4-2, position.y*3-1.5);
}

