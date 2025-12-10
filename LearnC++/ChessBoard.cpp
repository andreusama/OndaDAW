#include "ChessBoard.h"
#include "ShaderLoader.h"

ChessBoard::ChessBoard()
    : whiteVAO_(0), whiteVBO_(0), whiteEBO_(0), whiteIndexCount_(0),
      blackVAO_(0), blackVBO_(0), blackEBO_(0), blackIndexCount_(0),
      shaderProgram_(0)
{
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Load textures for white and black squares
    whiteSquareTexture_ = std::make_unique<Texture>("../LearnC++/textures/white_square.png");
    blackSquareTexture_ = std::make_unique<Texture>("../LearnC++/textures/container.jpg");

    InitializeShader();
    GenerateBoardGeometry();

    // Setup White Squares VAO
    glGenVertexArrays(1, &whiteVAO_);
    glGenBuffers(1, &whiteVBO_);
    glGenBuffers(1, &whiteEBO_);

    glBindVertexArray(whiteVAO_);
    glBindBuffer(GL_ARRAY_BUFFER, whiteVBO_);
    glBufferData(GL_ARRAY_BUFFER, whiteVertices_.size() * sizeof(float), whiteVertices_.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, whiteEBO_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, whiteIndices_.size() * sizeof(unsigned int), whiteIndices_.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    // Setup Black Squares VAO
    glGenVertexArrays(1, &blackVAO_);
    glGenBuffers(1, &blackVBO_);
    glGenBuffers(1, &blackEBO_);

    glBindVertexArray(blackVAO_);
    glBindBuffer(GL_ARRAY_BUFFER, blackVBO_);
    glBufferData(GL_ARRAY_BUFFER, blackVertices_.size() * sizeof(float), blackVertices_.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, blackEBO_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, blackIndices_.size() * sizeof(unsigned int), blackIndices_.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

ChessBoard::~ChessBoard()
{
    if (whiteVAO_ != 0) glDeleteVertexArrays(1, &whiteVAO_);
    if (whiteVBO_ != 0) glDeleteBuffers(1, &whiteVBO_);
    if (whiteEBO_ != 0) glDeleteBuffers(1, &whiteEBO_);
    if (blackVAO_ != 0) glDeleteVertexArrays(1, &blackVAO_);
    if (blackVBO_ != 0) glDeleteBuffers(1, &blackVBO_);
    if (blackEBO_ != 0) glDeleteBuffers(1, &blackEBO_);
    if (shaderProgram_ != 0) glDeleteProgram(shaderProgram_);
}

void ChessBoard::InitializeShader()
{
    shaderProgram_ = ShaderLoader::LoadShaders(
        "../LearnC++/shaders/textured.vert",
        "../LearnC++/shaders/textured.frag"
    );
}

void ChessBoard::GenerateBoardGeometry()
{
    whiteVertices_.clear();
    whiteIndices_.clear();
    blackVertices_.clear();
    blackIndices_.clear();
    squares_.clear();

    // Calculate board offset to center it at origin
    float boardOffset = (BOARD_SIZE * SQUARE_SIZE) / 2.0f;

    unsigned int whiteVertexCount = 0;
    unsigned int blackVertexCount = 0;

    // Generate vertices for each square
    for (int row = 0; row < BOARD_SIZE; ++row) {
        for (int col = 0; col < BOARD_SIZE; ++col) {
            // Determine square color (alternating pattern)
            bool isWhite = (row + col) % 2 == 0;
            glm::vec3 color = isWhite ? glm::vec3(0.9f, 0.9f, 0.9f) : glm::vec3(0.2f, 0.2f, 0.2f);

            // Calculate square position
            float x = col * SQUARE_SIZE - boardOffset;
            float z = row * SQUARE_SIZE - boardOffset;
            float y = 0.0f;

            // Create a Square object (no texture in constructor)
            glm::vec3 position(x, y, z);
            squares_.push_back(std::make_unique<Square>(SQUARE_SIZE, position, color));

            // Get vertices and indices from the Square object
            const auto& squareVertices = squares_.back()->GetVertices();
            const auto& squareIndices = squares_.back()->GetIndices();

            if (isWhite) {
                // Add to white squares buffer
                whiteVertices_.insert(whiteVertices_.end(), squareVertices.begin(), squareVertices.end());
                for (unsigned int index : squareIndices) {
                    whiteIndices_.push_back(whiteVertexCount + index);
                }
                whiteVertexCount += 4;
            } else {
                // Add to black squares buffer
                blackVertices_.insert(blackVertices_.end(), squareVertices.begin(), squareVertices.end());
                for (unsigned int index : squareIndices) {
                    blackIndices_.push_back(blackVertexCount + index);
                }
                blackVertexCount += 4;
            }
        }
    }

    whiteIndexCount_ = whiteIndices_.size();
    blackIndexCount_ = blackIndices_.size();
}

void ChessBoard::DrawBoard(glm::mat4& mvp)
{
    glUseProgram(shaderProgram_);

    // Set MVP matrix
    GLint mvpLoc = glGetUniformLocation(shaderProgram_, "MVP");
    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, &mvp[0][0]);

    // Set texture uniform location
    GLint texLoc = glGetUniformLocation(shaderProgram_, "texture1");
    glUniform1i(texLoc, 0);

    // Draw white squares
    whiteSquareTexture_->Bind(0);
    glBindVertexArray(whiteVAO_);
    glDrawElements(GL_TRIANGLES, whiteIndexCount_, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Draw black squares
    blackSquareTexture_->Bind(0);
    glBindVertexArray(blackVAO_);
    glDrawElements(GL_TRIANGLES, blackIndexCount_, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
