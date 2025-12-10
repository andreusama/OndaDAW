#pragma once
#include <glm.hpp>
#include <vector>
#include <memory>
#include "Texture.h"

class Square
{
public:
	Square(float size, glm::vec3 position, glm::vec3 color);
	Square(float size, glm::vec3 position, glm::vec3 color, std::unique_ptr<Texture> texture);
	~Square();

	const std::vector<float>& GetVertices() const { return vertices_; }
	const std::vector<unsigned int>& GetIndices() const { return indices_; }
	Texture* GetTexture() const { return texture_.get(); }
	bool HasTexture() const { return texture_ != nullptr; }

private:
	void GenerateGeometry();

	std::vector<float> vertices_;
	std::vector<unsigned int> indices_;

	float size_;
	glm::vec3 position_;
	glm::vec3 color_;

	std::unique_ptr<Texture> texture_;
};
