#pragma once
#include <memory>
#include "TextRenderer.h"

class UIModule {
    public:
        UIModule();
        ~UIModule();

        void Render(int viewportWidth, int viewportHeight);
        TextRenderer* GetTextRenderer() { return textRenderer_.get(); }

    private:
        std::unique_ptr<TextRenderer> textRenderer_;
};  

