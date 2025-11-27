#include "Grid.h"
#include "ShaderLoader.h"

Grid::Grid(int rows, int columns)
{
	if (VAO != 0) glDeleteVertexArrays(1, &VAO);
	if (VBO != 0) glDeleteBuffers(1, &VBO);
	VAO = 0;
	VBO = 0;

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	InitializeShader();

	vertices_ = Create3DGrid(rows, columns);
	vertexCount_ = vertices_.size() / 3; // 3 components per vertex

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	//Bind and upload data to VBO
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices_.size() * sizeof(float), vertices_.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(
		0, //Attribute 0 (matches "layour(location = 0)" - TODO: Check more info about this
		3, //3 components per vertex (x,y,z)
		GL_FLOAT, // Type of data
		GL_FALSE, //Don't normalize, Pure flag
		3 * sizeof(float),// Stride: 12 bytes 3 floats between each vertex
		(void*)0 //Offset TODO: Search more info about this
	);

	glEnableVertexAttribArray(0); //Enable attribute 0
	glBindVertexArray(0); //Unbind VAO

}

Grid::~Grid()
{
}

void Grid::InitializeShader()
{
	shaderProgram_ = ShaderLoader::LoadShaders(
		"../LearnC++/shaders/grid.vert",
		"../LearnC++/shaders/grid.frag"
	);
}

std::vector<float> Grid::Create3DGrid(int rows, int columns)
{
	std::vector<float> vertices;
	float spacing = 1.0f; // Spacing between grid lines
	// Create lines along the X axis
	for (int i = -rows / 2; i <= rows / 2; ++i) {
		vertices.push_back(-columns / 2 * spacing); // x1
		vertices.push_back(0.0f);                    // y1
		vertices.push_back(i * spacing);             // z1
		vertices.push_back(columns / 2 * spacing);  // x2
		vertices.push_back(0.0f);                    // y2
		vertices.push_back(i * spacing);             // z2
	}
	// Create lines along the Z axis
	for (int j = -columns / 2; j <= columns / 2; ++j) {
		vertices.push_back(j * spacing);             // x1
		vertices.push_back(0.0f);                    // y1
		vertices.push_back(-rows / 2 * spacing);    // z1
		vertices.push_back(j * spacing);             // x2
		vertices.push_back(0.0f);                    // y2
		vertices.push_back(rows / 2 * spacing);     // z2
	}
	return vertices;
}

void Grid::DrawGrid(glm::mat4 &mvp)
{
	//Shader program (vertex + fragment)
	glUseProgram(shaderProgram_); //Use shader program

	//set mvp matrix
	GLint mvpLoc = glGetUniformLocation(shaderProgram_, "MVP");
	glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, &mvp[0][0]);

	//set line color
	GLint colorLoc = glGetUniformLocation(shaderProgram_, "gridColor");
	glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, 1.0f); // White

	glBindVertexArray(VAO);
	glDrawArrays(GL_LINES, 0, vertexCount_);
	glBindVertexArray(0);
}
