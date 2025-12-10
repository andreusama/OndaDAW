#pragma once
#include <ft2build.h>
#include <map>
#include <memory>
#include "glm.hpp"
#include "glew.h"
#include <glm/vec2.hpp>
#include FT_FREETYPE_H

class Font
{
public:
	Font();
	~Font();

	struct Character {
		unsigned int TextureID;  // ID handle of the glyph texture
		glm::ivec2   Size;       // Size of glyph
		glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
		unsigned int Advance;    // Offset to advance to next glyph
	};

	std::map<char, Character> Characters;
private: 

	FT_Library ft_;
	FT_Face face_;
};
