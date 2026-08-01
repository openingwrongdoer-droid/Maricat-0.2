#version 330 core
layout (location = 0) in vec2 aPos;

out vec2 TexCoord;

uniform vec2 pos;
uniform vec2 size;
uniform float scale;
uniform vec2 costCenter;
uniform int issvg;
uniform float layer;
uniform float dir;
uniform int rotStyle;

void main()
{
	vec2 pixelSize = vec2(1.0f/480, 1.0f/360);
	vec2 invertY = vec2(1.0f, -1.0f);
	vec2 invertX = vec2(-1.0f, 1.0f);
	float s = scale / 100;
	
    vec2 p = aPos * size * (1 + issvg);
	
	p -= costCenter * (1 + issvg);
	
	float c = cos(-dir);
    float si = sin(-dir);

    switch (rotStyle) {
		case 0 :
		p *= vec2(si < 0 ? 1.0f : -1.0f, 1.0f);
		break;
		case 2 :
		p = vec2(
			c * p.x - si * p.y,
			si * p.x + c * p.y
		);
		break;
	}
	
	p *= scale / 100;
	
	p += pos * (1 + float(issvg)) * vec2(1.0f, 1.0f);
	
	p *= pixelSize;

	gl_Position = vec4(p, layer, 1.0f);
    TexCoord = aPos;
	
}