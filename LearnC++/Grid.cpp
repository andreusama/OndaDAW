#include "Grid.h"
#include "ShaderLoader.h"

Grid::Grid()
{
	InitializeShader();
}

Grid::~Grid()
{
}

void Grid::InitializeShader()
{
	shaderProgram_ = ShaderLoader::LoadShaders(
		"LearnC++/shaders/grid.vert",
		"LearnC++/shaders/grid.frag"
	);

}

void Grid::Draw3DLine(float x1, float y1, float z1, float x2, float y2, float z2, const glm::mat4& mvp)
{
	float vertices[] = {
		x1, y1, z1,
		x2, y2, z2
	};

	//Creates vao and then binds it (record)
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	//Bind and upload data to VBO
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(
		0, //Attribute 0 (matches "layour(location = 0)" - TODO: Check more info about this
		3, //3 components per vertex (x,y,z)
		GL_FLOAT, // Type of data
		GL_FALSE, //Don't normalize, Pure flag
		3 * sizeof(float),// Stride: 12 bytes 3 floats between each vertex
		(void*)0 //Offset TODO: Search more info about this
	);

	glEnableVertexAttribArray(0); //Enable attribute 0

	//Shader program (vertex + fragment)
	glUseProgram(shaderProgram_); //Use shader program

	//set mvp matrix
	GLint mvpLoc = glGetUniformLocation(shaderProgram_, "MVP");
	glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, &mvp[0][0]);

	//set line color
	GLint colorLoc = glGetUniformLocation(shaderProgram_, "gridColor");
	glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, 1.0f); //White color

	//DRAW!
	glDrawArrays(GL_LINES, 0, 2); //GL_LINES mode, starting at 0, draw 2 vertices

	glBindVertexArray(0); //Unbind VAO

	//CAREFULL, WE DIDN'T CLEAN UP VAO AND VBO as they are STATIC now!
}
