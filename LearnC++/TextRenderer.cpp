#include "TextRenderer.h"


TextRenderer::TextRenderer() : font_(std::make_unique<Font>())
{
	InitializeShader();

	// Configure VAO/VBO for texture quads
	glGenVertexArrays(1, &VAO_);
	glGenBuffers(1, &VBO_);
	glBindVertexArray(VAO_);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

TextRenderer::~TextRenderer()
{
	glDeleteVertexArrays(1, &VAO_);
	glDeleteBuffers(1, &VBO_);
	glDeleteProgram(shaderProgram_);
}

int TextRenderer::GetTextureFromText(const std::string& text, float x, float y, float scale, const glm::vec3& color)
{
    std::unique_ptr<TextTextureBuffer> textureTextBuffer = std::make_unique<TextTextureBuffer>();
    
    glGenFramebuffers(1, &textureTextBuffer.get()->fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, textureTextBuffer.get()->fbo);

    glGenTextures(1, &textureTextBuffer.get()->color);
    glBindTexture(GL_TEXTURE_2D, textureTextBuffer.get()->color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, textureTextBuffer.get()->width, textureTextBuffer.get()->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureTextBuffer.get()->color, 0);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, textureTextBuffer.get()->width, textureTextBuffer.get()->height);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return 0;
}

void TextRenderer::RenderText(const std::string& text, float x, float y, float scale, const glm::vec3& color)
{
    // activate corresponding render state	
    glUseProgram(shaderProgram_);
    glUniform3f(glGetUniformLocation(shaderProgram_, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO_);

    // iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Font::Character ch = font_.get()->Characters[*c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        // update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBO_);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void TextRenderer::InitializeShader()
{
	shaderProgram_ = ShaderLoader::LoadShaders(
		"../LearnC++/shaders/glyph.vert",
		"../LearnC++/shaders/glyph.frag"
	);
}

void TextRenderer::SetProjection(const glm::mat4& projection)
{
	glUseProgram(shaderProgram_);
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram_, "projection"), 1, GL_FALSE, &projection[0][0]);
}
