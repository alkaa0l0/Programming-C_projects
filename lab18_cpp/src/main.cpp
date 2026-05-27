#include "paint_core.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <stack>

std::stack<Canvas> undoStack;

enum class Tool { BRUSH, RECTANGLE, LINE, ERASER };

int main() {
    const int UI_HEIGHT = 120;
    sf::RenderWindow window(
        sf::VideoMode(WIDTH * PIXEL_SIZE, HEIGHT * PIXEL_SIZE + UI_HEIGHT),
        "Paint++"
    );

    auto desktop = sf::VideoMode::getDesktopMode();
    window.setPosition(sf::Vector2i(
        (desktop.width - window.getSize().x) / 2,
        (desktop.height - window.getSize().y) / 2
    ));

    Canvas canvas(HEIGHT, std::vector<sf::Color>(WIDTH, sf::Color::White));
    clearCanvas(canvas);

    Tool currentTool = Tool::RECTANGLE;
    sf::Color currentColor = sf::Color::Black;
    int brushRadius = 3;

    std::vector<sf::Color> palette = {
        sf::Color::Black, sf::Color::White, sf::Color::Red,
        sf::Color::Green, sf::Color::Blue, sf::Color::Yellow,
        sf::Color::Cyan, sf::Color::Magenta, sf::Color(255, 128, 0),
        sf::Color(128, 0, 128)
    };
    int selectedPaletteIndex = 0;

    bool isDrawing = false;
    int startX = 0, startY = 0, lastX = 0, lastY = 0;

    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {}

    sf::Text status;
    status.setFont(font);
    status.setCharacterSize(14);
    status.setFillColor(sf::Color::Black);
    status.setPosition(10, HEIGHT * PIXEL_SIZE + 10);

    std::cout << "=== PAINT with STL ===\n"
              << "Tools:\n  B - Brush   R - Rectangle   L - Line   E - Eraser\n"
              << "  +/- - Brush size\n  C - Clear canvas\n  Z - Undo (Ctrl+Z)\n"
              << "Filters:\n  G - Grayscale   N - Negative   M - Blur\n"
              << "Colors: 0..9 or click on palette\n";

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::KeyPressed) {
                switch (event.key.code) {
                    case sf::Keyboard::B: currentTool = Tool::BRUSH; break;
                    case sf::Keyboard::R: currentTool = Tool::RECTANGLE; break;
                    case sf::Keyboard::L: currentTool = Tool::LINE; break;
                    case sf::Keyboard::E: currentTool = Tool::ERASER; break;
                    case sf::Keyboard::C: clearCanvas(canvas); break;

                    case sf::Keyboard::Z:
                        if (event.key.control) undo(canvas);
                        break;

                    case sf::Keyboard::G:
                        grayscale(canvas);
                        break;
                    case sf::Keyboard::N:
                        negative(canvas);
                        break;
                    case sf::Keyboard::M:
                        blur(canvas);
                        break;

                    case sf::Keyboard::Add:
                    case sf::Keyboard::Equal:
                        if (brushRadius < 20) ++brushRadius;
                        break;

                    case sf::Keyboard::Hyphen:
                    case sf::Keyboard::Subtract:
                        if (brushRadius > 1) --brushRadius;
                        break;

                    case sf::Keyboard::Num0:
                    case sf::Keyboard::Num1:
                    case sf::Keyboard::Num2:
                    case sf::Keyboard::Num3:
                    case sf::Keyboard::Num4:
                    case sf::Keyboard::Num5:
                    case sf::Keyboard::Num6:
                    case sf::Keyboard::Num7:
                    case sf::Keyboard::Num8:
                    case sf::Keyboard::Num9: {
                        int idx = event.key.code - sf::Keyboard::Num0;
                        if (idx >= 0 && idx < static_cast<int>(palette.size())) {
                            selectedPaletteIndex = idx;
                            currentColor = palette[idx];
                        }
                        break;
                    }

                    default:
                        break;
                }
            }

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                int paletteY = HEIGHT * PIXEL_SIZE + 50;
                bool clickedPalette = false;

                if (event.mouseButton.y >= paletteY && event.mouseButton.y <= paletteY + 30) {
                    for (size_t i = 0; i < palette.size(); ++i) {
                        int boxX = 10 + static_cast<int>(i) * 35;
                        if (event.mouseButton.x >= boxX && event.mouseButton.x <= boxX + 30) {
                            selectedPaletteIndex = static_cast<int>(i);
                            currentColor = palette[i];
                            clickedPalette = true;
                            break;
                        }
                    }
                }

                if (!clickedPalette) {
                    isDrawing = true;
                    startX = event.mouseButton.x / PIXEL_SIZE;
                    startY = event.mouseButton.y / PIXEL_SIZE;
                    lastX = startX;
                    lastY = startY;

                    saveToUndo(canvas);

                    if (currentTool == Tool::RECTANGLE || currentTool == Tool::LINE) {
                    } else {
                        sf::Color drawColor = (currentTool == Tool::ERASER) ? sf::Color::White : currentColor;
                        drawBrush(canvas, startX, startY, drawColor, brushRadius);
                    }
                }
            }

            if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
                if (isDrawing && (currentTool == Tool::RECTANGLE || currentTool == Tool::LINE)) {
                    int endX = event.mouseButton.x / PIXEL_SIZE;
                    int endY = event.mouseButton.y / PIXEL_SIZE;
                    sf::Color drawColor = (currentTool == Tool::ERASER) ? sf::Color::White : currentColor;

                    if (currentTool == Tool::RECTANGLE)
                        drawRectangle(canvas, startX, startY, endX, endY, drawColor);
                    else if (currentTool == Tool::LINE)
                        drawLine(canvas, startX, startY, endX, endY, drawColor);
                }
                isDrawing = false;
            }

            if (event.type == sf::Event::MouseMoved && isDrawing) {
                int x = event.mouseMove.x / PIXEL_SIZE;
                int y = event.mouseMove.y / PIXEL_SIZE;

                if ((currentTool == Tool::BRUSH || currentTool == Tool::ERASER) && (x != lastX || y != lastY)) {
                    sf::Color drawColor = (currentTool == Tool::ERASER) ? sf::Color::White : currentColor;
                    drawBrush(canvas, x, y, drawColor, brushRadius);
                    lastX = x;
                    lastY = y;
                }
            }
        }

        window.clear(sf::Color(200, 200, 200));
        renderToWindow(window, canvas);

        for (size_t i = 0; i < palette.size(); ++i) {
            sf::RectangleShape box(sf::Vector2f(30, 30));
            box.setFillColor(palette[i]);
            box.setPosition(10 + i * 35, HEIGHT * PIXEL_SIZE + 50);
            box.setOutlineThickness(2);
            box.setOutlineColor((i == selectedPaletteIndex) ? sf::Color::Red : sf::Color::Black);
            window.draw(box);
        }

        std::string toolStr;
        switch (currentTool) {
            case Tool::BRUSH: toolStr = "BRUSH"; break;
            case Tool::RECTANGLE: toolStr = "RECT"; break;
            case Tool::LINE: toolStr = "LINE"; break;
            case Tool::ERASER: toolStr = "ERASER"; break;
        }

        status.setString(toolStr + "  |  radius:" + std::to_string(brushRadius) +
                         "  |  undo stack:" + std::to_string(undoStack.size()));
        window.draw(status);
        window.display();
    }

    return 0;
}