#pragma once
#include "Panel.h"
#include <memory>
#include "Grid.h"
#include "ChessBoard.h"
#include "Camera.h"

class InputModule;

class ViewportPanel : public Panel
{
public:
	ViewportPanel(int w, int h, InputModule* inputModule);
	~ViewportPanel();

	void CreateViewportFramebuffer();
	void RecreateFramebuffer();
	void Render(float deltaTime) override;
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
	std::unique_ptr<Grid> grid_;
	std::unique_ptr<ChessBoard> chessBoard_;
	std::unique_ptr<Camera> camera_;
	InputModule* inputModule_;
};
