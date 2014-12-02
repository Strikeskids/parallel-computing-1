#version 330

layout(location=0) in vec2 position;

smooth out vec2 complexPosition;

uniform vec4 rect;

void main() {
	gl_Position = vec4(position.x*2-1, position.y*2-1, 0, 1);
	complexPosition = vec2(position.x*rect.z+rect.x, position.y*rect.w+rect.y);
}

