#version 330

smooth in vec2 windowPosition;

out vec4 outputColor;

uniform vec4 zoomRect;
uniform int iterations;

float normal(float x, float mean, float stdev) {
	float z = (x-mean)/stdev;
	return exp(-z*z/2);
}

float sqmag(vec2 v) {
	return v.x*v.x + v.y*v.y;
}

void main() {
	vec2 complexPosition = (windowPosition - vec2(0.5, 0.5)) * zoomRect.zw + zoomRect.xy;

	vec2 z = vec2(0,0);
	vec2 zo = vec2(0, 0);
	int iter;
	for (iter=0; iter<iterations && sqmag(z) < 100; ++iter) {
		zo.xy = z.xy;
		z.x = zo.x*zo.x - zo.y*zo.y + complexPosition.x;
		z.y = 2*zo.x*zo.y + complexPosition.y;
	}
	if (iter == iterations) {
		outputColor = vec4(0, 0, 0, 0);
	} else {
		float val = clamp(iter + 1 - log(log(sqrt(sqmag(z)))) / log(2.0f), 2, iterations);
		float prod = 3;
		val = log(prod*log(val))/log(prod*log(1.0*iterations));
		outputColor = vec4(normal(val, 0.8, 0.3), normal(val, 0.5, 0.3), normal(val, 0.2, 0.3), 1.0f); 
	}
}

