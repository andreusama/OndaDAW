#pragma once
#include "Panel.h"
#include <memory>

class ViewportPanel : public Panel
{
public:
	ViewportPanel(int w, int h);
	~ViewportPanel();

	void CreateViewportFramebuffer();
	void RecreateFramebuffer();
	void Render() override;
	void RenderScene();

	struct Viewport {
	public:
		GLuint fbo = 0;
		GLuint color = 0;
		GLuint depth = 0;
		int width = 800;
		int height = 600;
	};

	std::unique_ptr<Viewport> viewport_;
};
