#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
flat in int texIndex;
flat in int outside;
flat in int color;

uniform sampler2D myTexture;

void main()
{
	vec4 tex = texture(myTexture, TexCoord); 
    if (tex.a < 0.01) discard;
	FragColor = vec4(1.0f, 0.0f, 0.0f, 0.5f);
}