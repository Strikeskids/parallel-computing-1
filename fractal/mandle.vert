#version 330

layout(location=0) in vec2 position;

smooth out vec2 uv;

void main() {
	uv = position;
	gl_Position = vec4(position.x*2-1, position.y*2-1, 0, 1);
}

