#include "paint_core.hpp"
#include <algorithm>   // std::min, std::max, std::transform, std::fill
#include <cmath>       // std::abs
#include <stack>       // std::stack

extern std::stack<Canvas> undoStack;

// ========== Сохранение в undo ==========
void saveToUndo(const Canvas& canvas) {
    undoStack.push(canvas);
    if (undoStack.size() > 30) {
        std::stack<Canvas> tmp;
        while (undoStack.size() > 1) {
            tmp.push(std::move(undoStack.top()));
            undoStack.pop();
        }
        undoStack.pop();              
        while (!tmp.empty()) {
            undoStack.push(std::move(tmp.top()));
            tmp.pop();
        }
    }
}

// ========== 1. Очистка холста   ==========
void clearCanvas(Canvas& canvas) {
    for (auto& row : canvas) {
        std::fill(row.begin(), row.end(), sf::Color::White);
    }
}

// ========== 2. Пиксель ==========
void drawPixel(Canvas& canvas, int x, int y, const sf::Color& color) {
    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
        canvas[y][x] = color;
    }
}

// ========== 3. Круглая кисть ==========
void drawBrush(Canvas& canvas, int x, int y, const sf::Color& color, int radius) {
    for (int dy = -radius; dy <= radius; ++dy) {
        for (int dx = -radius; dx <= radius; ++dx) {
            if (dx * dx + dy * dy <= radius * radius) {
                drawPixel(canvas, x + dx, y + dy, color);
            }
        }
    }
}

// ========== 4. Прямоугольник  ==========
void drawRectangle(Canvas& canvas, int x1, int y1, int x2, int y2, const sf::Color& color) {
    saveToUndo(canvas);
    int left   = std::min(x1, x2);
    int right  = std::max(x1, x2);
    int top    = std::min(y1, y2);
    int bottom = std::max(y1, y2);
    for (int y = top; y <= bottom; ++y) {
        for (int x = left; x <= right; ++x) {
            drawPixel(canvas, x, y, color);
        }
    }
}

// ========== 5. Линия — алгоритм Брезенхема==========
void drawLine(Canvas& canvas, int x1, int y1, int x2, int y2, const sf::Color& color) {
    saveToUndo(canvas);
    int dx = std::abs(x2 - x1);
    int dy = std::abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    while (true) {
        drawPixel(canvas, x1, y1, color);
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x1 += sx; }
        if (e2 <  dx) { err += dx; y1 += sy; }
    }
}

// ========== 6. Undo  ==========
void undo(Canvas& canvas) {
    if (!undoStack.empty()) {
        canvas = undoStack.top();
        undoStack.pop();
    }
}

// ========== 7. Оттенки серого  ==========
void grayscale(Canvas& canvas) {
    saveToUndo(canvas);
    for (auto& row : canvas) {
        std::transform(row.begin(), row.end(), row.begin(), [](sf::Color c) {
            int gray = (c.r + c.g + c.b) / 3;
            return sf::Color(gray, gray, gray);
        });
    }
}

// ========== 8. Негатив ==========
void negative(Canvas& canvas) {
    saveToUndo(canvas);
    for (auto& row : canvas) {
        std::transform(row.begin(), row.end(), row.begin(), [](sf::Color c) {
            return sf::Color(255 - c.r, 255 - c.g, 255 - c.b);
        });
    }
}

// ========== 9. Размытие 3x3  ==========
void blur(Canvas& canvas) {
    saveToUndo(canvas);
    Canvas original = canvas;                
    for (int y = 1; y < HEIGHT - 1; ++y) {
        for (int x = 1; x < WIDTH - 1; ++x) {
            int r = 0, g = 0, b = 0;
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    sf::Color c = original[y + dy][x + dx];
                    r += c.r;
                    g += c.g;
                    b += c.b;
                }
            }
            canvas[y][x] = sf::Color(r / 9, g / 9, b / 9);
        }
    }
    
}

// ========== Отрисовка ==========
void renderToWindow(sf::RenderWindow& window, const Canvas& canvas) {
    static sf::Image image;
    static sf::Texture texture;
    static sf::Sprite sprite;

    image.create(WIDTH, HEIGHT);
    for (int y = 0; y < HEIGHT; ++y)
        for (int x = 0; x < WIDTH; ++x)
            image.setPixel(x, y, canvas[y][x]);

    texture.loadFromImage(image);
    texture.setSmooth(false);
    sprite.setTexture(texture);
    sprite.setScale(PIXEL_SIZE, PIXEL_SIZE);
    window.draw(sprite);
}