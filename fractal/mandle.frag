#version 330

smooth in vec2 complexPosition;

out vec4 outputColor;

void main() {
	outputColor = vec4(complexPosition.y<0?1:0, 0, complexPosition.x<0?1:0, 1.0f);
}

