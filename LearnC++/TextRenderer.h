#pragma once
#include <glew.h>    // MUST be included BEFORE any OpenGL headers
#include <glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Font.h"
#include <string>
#include "ShaderLoader.h"

class TextRenderer
{
public:
	TextRenderer();
	~TextRenderer();

	void RenderText(const std::string& text, float x, float y, float scale, const glm::vec3& color);
	int GetTextureFromText(const std::string& text, float x, float y, float scale, const glm::vec3& color);
	void SetProjection(const glm::mat4& projection);

	struct TextTextureBuffer {
	public:
		GLuint fbo = 0;
		GLuint color = 0;
		int width = 800;
		int height = 600;
	};

private:
	void InitializeShader();
		
	GLuint VAO_;
	GLuint VBO_;

	GLuint shaderProgram_;

	std::unique_ptr<Font> font_;

};
