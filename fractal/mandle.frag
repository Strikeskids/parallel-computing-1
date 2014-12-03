#version 330

smooth in vec2 complexPosition;

out vec4 outputColor;

uniform int iterations;

void main() {
	vec2 z = vec2(0, 0);
	vec2 zo = vec2(0, 0);
	int iter;
	for (iter=0; iter<iterations && length(z) < 2; ++iter) {
		zo.xy = z.xy;
		z.x = zo.x*zo.x - zo.y*zo.y + complexPosition.x;
		z.y = 2*zo.x*zo.y + complexPosition.y;
	}
	if (iter == iterations) {
		outputColor = vec4(0, 0, 0, 0);
	} else {
		float val = iter;
		val /= iterations;
		outputColor = vec4(val, val, val, 1.0f); 
	}
}

