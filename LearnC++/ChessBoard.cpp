#include "ChessBoard.h"
#include "ShaderLoader.h"

ChessBoard::ChessBoard() : VAO(0), VBO(0), EBO(0), shaderProgram_(0), indexCount_(0)
{
    // Clean up any existing buffers
    if (VAO != 0) glDeleteVertexArrays(1, &VAO);
    if (VBO != 0) glDeleteBuffers(1, &VBO);
    if (EBO != 0) glDeleteBuffers(1, &EBO);
    VAO = 0;
    VBO = 0;
    EBO = 0;

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    InitializeShader();
    GenerateBoardGeometry();

    // Generate and bind VAO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // Upload vertex data to VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices_.size() * sizeof(float), vertices_.data(), GL_STATIC_DRAW);

    // Upload index data to EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_.size() * sizeof(unsigned int), indices_.data(), GL_STATIC_DRAW);

    // Position attribute (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Color attribute (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

ChessBoard::~ChessBoard()
{
    if (VAO != 0) glDeleteVertexArrays(1, &VAO);
    if (VBO != 0) glDeleteBuffers(1, &VBO);
    if (EBO != 0) glDeleteBuffers(1, &EBO);
    if (shaderProgram_ != 0) glDeleteProgram(shaderProgram_);
}

void ChessBoard::InitializeShader()
{
    shaderProgram_ = ShaderLoader::LoadShaders(
        "../LearnC++/shaders/chessboard.vert",
        "../LearnC++/shaders/chessboard.frag"
    );
}

void ChessBoard::GenerateBoardGeometry()
{
    vertices_.clear();
    indices_.clear();

    // Calculate board offset to center it at origin
    float boardOffset = (BOARD_SIZE * SQUARE_SIZE) / 2.0f;

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

            // Calculate vertex index offset for this square
            unsigned int vertexOffset = (row * BOARD_SIZE + col) * 4;

            // Add 4 vertices for this square (quad)
            // Bottom-left
            vertices_.push_back(x);
            vertices_.push_back(y);
            vertices_.push_back(z);
            vertices_.push_back(color.r);
            vertices_.push_back(color.g);
            vertices_.push_back(color.b);

            // Bottom-right
            vertices_.push_back(x + SQUARE_SIZE);
            vertices_.push_back(y);
            vertices_.push_back(z);
            vertices_.push_back(color.r);
            vertices_.push_back(color.g);
            vertices_.push_back(color.b);

            // Top-right
            vertices_.push_back(x + SQUARE_SIZE);
            vertices_.push_back(y);
            vertices_.push_back(z + SQUARE_SIZE);
            vertices_.push_back(color.r);
            vertices_.push_back(color.g);
            vertices_.push_back(color.b);

            // Top-left
            vertices_.push_back(x);
            vertices_.push_back(y);
            vertices_.push_back(z + SQUARE_SIZE);
            vertices_.push_back(color.r);
            vertices_.push_back(color.g);
            vertices_.push_back(color.b);

            // Add indices for two triangles forming the square
            // Triangle 1
            indices_.push_back(vertexOffset + 0);
            indices_.push_back(vertexOffset + 1);
            indices_.push_back(vertexOffset + 2);

            // Triangle 2
            indices_.push_back(vertexOffset + 0);
            indices_.push_back(vertexOffset + 2);
            indices_.push_back(vertexOffset + 3);
        }
    }

    indexCount_ = indices_.size();
}

void ChessBoard::DrawBoard(glm::mat4& mvp)
{
    glUseProgram(shaderProgram_);

    // Set MVP matrix
    GLint mvpLoc = glGetUniformLocation(shaderProgram_, "MVP");
    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, &mvp[0][0]);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount_, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
