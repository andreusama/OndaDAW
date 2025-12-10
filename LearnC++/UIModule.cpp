#include "UIModule.h"
#include <glm/gtc/matrix_transform.hpp>

UIModule::UIModule() : textRenderer_(std::make_unique<TextRenderer>())
{
}

UIModule::~UIModule()
{
}

void UIModule::Render(int viewportWidth, int viewportHeight)
{
    // Set up orthographic projection for 2D text rendering in screen space
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(viewportWidth),
                                       0.0f, static_cast<float>(viewportHeight));
    textRenderer_->SetProjection(projection);

    // Enable blending for text transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Example: Render some text
    // You can call this from anywhere you want to display text
    textRenderer_->RenderText("Hello World", 25.0f, 25.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));

    glDisable(GL_BLEND);
}
