#pragma once
#include <glew.h>    // MUST be included BEFORE any OpenGL headers
#include <glm.hpp>

class Grid {
public:

	Grid();
	~Grid();

	void Draw3DLine(float x1, float y1, float z1, float x2, float y2, float z2, const glm::mat4& mvp);

private:
	void InitializeShader();

	GLuint VAO;
	GLuint VBO;
	GLuint shaderProgram_;
};
