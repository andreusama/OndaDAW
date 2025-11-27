#pragma once
//Panel.h
#include <vector>
#include <string>
#include <functional>
#include "glew.h"

class Panel 
{ 
public: 
	Panel(const std::string& panelName);
	~Panel();

	virtual void Render(float deltaTime) = 0;

	const std::string& GetName() const { return name; }

protected:
	std::string name; 
};

