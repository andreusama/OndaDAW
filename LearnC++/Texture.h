#pragma once
#include <glew.h>
#include <string>

class Texture
{
public:
	Texture(const char* filepath);
	~Texture();

	void Bind(unsigned int slot = 0);
	void Unbind();
	GLuint GetID() const { return textureID_; }

private:
	GLuint textureID_;
	int width_;
	int height_;
	int channels_;
};
