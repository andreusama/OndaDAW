#include "Square.h"

Square::Square(float size, glm::vec3 position, glm::vec3 color)
	: size_(size), position_(position), color_(color), texture_(nullptr)
{
	GenerateGeometry();
}

Square::Square(float size, glm::vec3 position, glm::vec3 color, std::unique_ptr<Texture> texture)
	: size_(size), position_(position), color_(color), texture_(std::move(texture))
{
	GenerateGeometry();
}

Square::~Square()
{
}

void Square::GenerateGeometry()
{
	vertices_.clear();
	indices_.clear();

	float x = position_.x;
	float y = position_.y;
	float z = position_.z;

	// Add 4 vertices for this square (quad)
	// Bottom-left
	vertices_.push_back(x);
	vertices_.push_back(y);
	vertices_.push_back(z);
	vertices_.push_back(color_.r);
	vertices_.push_back(color_.g);
	vertices_.push_back(color_.b);
	vertices_.push_back(0.0f);
	vertices_.push_back(0.0f);

	// Bottom-right
	vertices_.push_back(x + size_);
	vertices_.push_back(y);
	vertices_.push_back(z);
	vertices_.push_back(color_.r);
	vertices_.push_back(color_.g);
	vertices_.push_back(color_.b);
	vertices_.push_back(1.0f);
	vertices_.push_back(0.0f);

	// Top-right
	vertices_.push_back(x + size_);
	vertices_.push_back(y);
	vertices_.push_back(z + size_);
	vertices_.push_back(color_.r);
	vertices_.push_back(color_.g);
	vertices_.push_back(color_.b);
	vertices_.push_back(1.0f);
	vertices_.push_back(1.0f);

	// Top-left
	vertices_.push_back(x);
	vertices_.push_back(y);
	vertices_.push_back(z + size_);
	vertices_.push_back(color_.r);
	vertices_.push_back(color_.g);
	vertices_.push_back(color_.b);
	vertices_.push_back(0.0f);
	vertices_.push_back(1.0f);

	// Add indices for two triangles forming the square
	// Triangle 1
	indices_.push_back(0);
	indices_.push_back(1);
	indices_.push_back(2);

	// Triangle 2
	indices_.push_back(0);
	indices_.push_back(2);
	indices_.push_back(3);
}
