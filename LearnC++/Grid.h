#pragma once
#include <glew.h>    // MUST be included BEFORE any OpenGL headers
#include <glm.hpp>
#include <vector>

class Grid {
public:

	Grid(int rows, int columns);
	~Grid();

	void Draw3DLine(float x1, float y1, float z1, float x2, float y2, float z2, const glm::mat4& mvp);
	void DrawGrid(glm::mat4& mvp);

	std::vector<float> Create3DGrid(int rows, int columns);

private:
	void InitializeShader();

	GLuint VAO;
	GLuint VBO;
	GLuint shaderProgram_;

	std::vector<float> vertices_;
	int vertexCount_;              // Number of vertices (not floats!)
};
