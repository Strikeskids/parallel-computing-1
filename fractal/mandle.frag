#version 330

smooth in vec2 uv;

out vec4 outputColor;

uniform sampler2D iterationsTexture;
uniform int iterations;
uniform int minimumIterations;

float normal(float x, float mean, float stdev) {
	float z = (x-mean)/stdev;
	return exp(-z*z/2);
}

void main() {
//	outputColor = vec4(uv, float(int(iterationsTexture)), 1); //texture(iterationsTexture, uv);
	vec4 iterData = texture(iterationsTexture, uv);
	int iter = int(iterData[1] * 255 * 256 + iterData[0] * 255 + 0.5);
	if (iter == iterations) {
		outputColor = vec4(0, 0, 0, 0);
	} else {
		float newIterations = iterations-minimumIterations+1;
		float val = clamp(iter-minimumIterations+1, 2, newIterations);
		float prod = 3;
		val = log(val)/log(newIterations);
		//val = log(prod*log(val))/log(prod*log(newIterations));
		outputColor = vec4(normal(val, 0.8, 0.3), normal(val, 0.5, 0.3), normal(val, 0.2, 0.3), 1.0f); 
	}
}

