#include "ViewportPanel.h"
#include <glew.h>
#include "imgui.h"


ViewportPanel::ViewportPanel(int w, int h) : Panel("Viewport"), viewport_(std::make_unique<Viewport>(0,0,0, w, h))
{
	CreateViewportFramebuffer();
}

ViewportPanel::~ViewportPanel()
{
}

void ViewportPanel::CreateViewportFramebuffer()
{
	glGenFramebuffers(1, &viewport_.get()->fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, viewport_.get()->fbo);

	glGenTextures(1, &viewport_.get()->color);
	glBindTexture(GL_TEXTURE_2D, viewport_.get()->color);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, viewport_.get()->width, viewport_.get()->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, viewport_.get()->color, 0);

	glGenRenderbuffers(1, &viewport_.get()->depth);
	glBindRenderbuffer(GL_RENDERBUFFER, viewport_.get()->depth);

	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, viewport_.get()->width, viewport_.get()->height);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, viewport_.get()->depth);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ViewportPanel::RecreateFramebuffer()
{
	glBindFramebuffer(GL_FRAMEBUFFER, viewport_->fbo);

	// Resize color texture
	glBindTexture(GL_TEXTURE_2D, viewport_->color);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, viewport_->width, viewport_->height, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	// Resize depth renderbuffer
	glBindRenderbuffer(GL_RENDERBUFFER, viewport_->depth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8,
		viewport_->width, viewport_->height);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ViewportPanel::Render() 
{
	ImGui::Begin("3D Viewport");
	ImVec2 panelSize = ImGui::GetContentRegionAvail();
	int newWidth = (int)panelSize.x;
	int newHeight = (int)panelSize.y;

	if (newWidth != viewport_->width || newHeight != viewport_->height) {
		viewport_->width = newWidth;
		viewport_->height = newHeight;
		RecreateFramebuffer();
	}

	// 1. Bind FBO
	glBindFramebuffer(GL_FRAMEBUFFER, viewport_.get()->fbo);
	glViewport(0, 0, viewport_.get()->width, viewport_.get()->height);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// 2. Render your 3D scene
	RenderScene();

	// 3. Unbind FBO
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	ImGui::Image((void*)(intptr_t)viewport_->color,
		ImVec2((float)viewport_->width, (float)viewport_->height),
		ImVec2(0, 1), ImVec2(1, 0));	
	ImGui::End();
}

void ViewportPanel::RenderScene() 
{
	// Placeholder for actual 3D rendering code
}
