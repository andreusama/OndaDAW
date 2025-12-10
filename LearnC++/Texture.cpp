#define STB_IMAGE_IMPLEMENTATION
#include "Texture.h"
#include <stb_image.h>
#include <iostream>

Texture::Texture(const char* filepath)
	: textureID_(0), width_(0), height_(0), channels_(0)
{
	// Load image using stb_image
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(filepath, &width_, &height_, &channels_, 0);

	if (data)
	{
		// Generate OpenGL texture
		glGenTextures(1, &textureID_);
		glBindTexture(GL_TEXTURE_2D, textureID_);

		// Set texture parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Determine format based on number of channels
		GLenum format;
		if (channels_ == 1)
			format = GL_RED;
		else if (channels_ == 3)
			format = GL_RGB;
		else if (channels_ == 4)
			format = GL_RGBA;
		else
			format = GL_RGB; // Default fallback

		// Upload texture data to GPU
		glTexImage2D(GL_TEXTURE_2D, 0, format, width_, height_, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		// Unbind texture
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	else
	{
		std::cout << "Failed to load texture: " << filepath << std::endl;
	}

	// Free image data
	stbi_image_free(data);
}

Texture::~Texture()
{
	if (textureID_ != 0)
	{
		glDeleteTextures(1, &textureID_);
	}
}

void Texture::Bind(unsigned int slot)
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, textureID_);
}

void Texture::Unbind()
{
	glBindTexture(GL_TEXTURE_2D, 0);
}
