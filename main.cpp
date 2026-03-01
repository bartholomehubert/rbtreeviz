#include <charconv>
#include <queue>

#include "raylib.h"

#include "RBTree.hpp"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

enum class Command {
    INVALID_COMMAND,
    INVALID_ARGUMENT,
    INSERT,
    CREATE_RANDOM,
};

// operator that will be used to compute the nodes coordinate
Vector2 operator+(const Vector2 &a, const Vector2 &b) {
    return {a.x + b.x, a.y + b.y};
}

Vector2 operator*(const Vector2 &a, float b) {
    return {a.x * b, a.y * b};
}

class Visualizer {
public:
    static Font font;
    static constexpr int STATUS_BAR_HEIGHT = 36;
    static constexpr int FONT_SIZE = 20;
    static constexpr float FONT_SPACING = 2.f;

    static constexpr float KEY_DELAY = 0.2f;
    static constexpr float KEY_REPEAT = 0.02f;

    std::string command = "";
    std::string status = "type a command";
    Color statusColor = WHITE;

    float backspaceTimer = 0.f;

    static constexpr float CURSOR_TIME = 0.5f;
    float cursorTimer = 0.f;

    static constexpr float ANIMATION_TIME = 1;
    float animationTimer = 0.f;

    struct NodeMeta {
        Vector2 pos;
        Vector2 newPos;
        Vector2 oldPos;
        bool initialized = false;
    };

    RBTree<int, NodeMeta> tree = RBTree<int, NodeMeta>::createRandom(9);

    void handleInput() {

        if (IsKeyPressed(KEY_BACKSPACE)) {
            if (!command.empty())
                command.pop_back();

            backspaceTimer = 0.0f;
        }

        if (IsKeyDown(KEY_BACKSPACE)) {
            backspaceTimer += GetFrameTime();

            if (backspaceTimer > KEY_DELAY) {
                if (!command.empty())
                    command.pop_back();

                backspaceTimer -= KEY_REPEAT;
            }
        }

        int key = GetCharPressed();
        while (key > 0) {
            if ((key >= 32) && (key <= 125)) {
                command += (char)key;
            }
            key = GetCharPressed();
        }

        cursorTimer += GetFrameTime();
        if (cursorTimer >= 2 * CURSOR_TIME) {
            cursorTimer = 0.0f;
        }

        if (IsKeyPressed(KEY_ENTER)) {
            execute();
            command = "";
        }
    }

    std::pair<Command, int> parseCommand() {
        command.erase(std::remove_if(command.begin(), command.end(), isspace), command.end());
        char c = tolower(command[0]);
        Command com = Command::INVALID_COMMAND;

        switch (c) {
        case 'i':
            com = Command::INSERT;
            break;
        case 'r':
            com = Command::CREATE_RANDOM;
            break;
        default:
            return {com, 0};
        }

        int val;
        auto [ptr, ec] = std::from_chars(command.data() + 1, command.data() + command.size(), val);
        if (ec == std::errc() && ptr == command.data() + command.size()) {
            return {com, val};
        }

        return {Command::INVALID_ARGUMENT, 0};
    }

    void execute() {
        auto [c, val] = parseCommand();

        switch (c) {
        case Command::INVALID_COMMAND:
            status = "invalid command";
            statusColor = RED;
            break;
        case Command::INVALID_ARGUMENT:
            status = "invalid argument";
            statusColor = RED;
            break;
        case Command::INSERT:
            tree.insert(val, {});
            animationTimer = 0.f;
            status = "node " + std::to_string(val) + " added";
            statusColor = GREEN;
            break;
        case Command::CREATE_RANDOM:
            tree = RBTree<int, NodeMeta>::createRandom(val);
            animationTimer = 0.f;
            status = "random tree of " + std::to_string(val) + " nodes created";
            statusColor = GREEN;
            break;
        }
    }

    static void drawNode(RBTree<int, NodeMeta>::Node *node, float radius, float progress) {

        // linear interpolation of the two positions
        node->val.pos = node->val.oldPos * (1 - progress) + node->val.newPos * progress;

        const float borderOffset = sqrt(2) * radius / 2;
        if (node->parent) {
            // this is a hack to avoid drawing an edge that would overlap with the parent that has already been drawn
            DrawLine(node->val.pos.x, node->val.pos.y,
                     node->parent->val.pos.x + (node->isRightChild() ? 1 : -1) * borderOffset, node->parent->val.pos.y + borderOffset,
                     WHITE);
        }

        Color color = node->isRed ? RED : Color{128, 128, 128, 255};
        DrawCircle(node->val.pos.x, node->val.pos.y, radius, color);

        std::string keyStr = "k:" + std::to_string(node->key);
        std::string rankStr = "r:" + std::to_string(node->rank);
        Vector2 keyStrDim = MeasureTextEx(font, keyStr.c_str(), FONT_SIZE, FONT_SPACING);
        Vector2 rankStrDim = MeasureTextEx(font, rankStr.c_str(), FONT_SIZE, FONT_SPACING);

        float alignWidth = fmaxf(keyStrDim.x, rankStrDim.x);

        DrawTextEx(font, keyStr.c_str(), {node->val.pos.x - alignWidth / 2, node->val.pos.y - keyStrDim.y - 1}, FONT_SIZE, FONT_SPACING, WHITE);
        DrawTextEx(font, rankStr.c_str(), {node->val.pos.x - alignWidth / 2, node->val.pos.y + 1}, FONT_SIZE, FONT_SPACING, WHITE);
    }

    void drawStatusBar() {
        DrawRectangle(0, GetScreenHeight() - STATUS_BAR_HEIGHT, GetScreenWidth(), STATUS_BAR_HEIGHT, BLACK);

        int y = GetScreenHeight() - STATUS_BAR_HEIGHT + FONT_SIZE / 2;
        Vector2 textDim = MeasureTextEx(font, status.c_str(), FONT_SIZE, FONT_SPACING);

        Vector2 commandPosition = {10, (float)y};
        Vector2 statusPosition = {GetScreenWidth() - textDim.x - 10, (float)y};
        bool showCursor = cursorTimer <= CURSOR_TIME;

        if (showCursor)
            command += '|';

        DrawTextEx(font, command.c_str(), commandPosition, FONT_SIZE, FONT_SPACING, WHITE);
        DrawTextEx(font, status.c_str(), statusPosition, FONT_SIZE, FONT_SPACING, statusColor);

        if (showCursor)
            command.pop_back();
    }

    void drawTree() {
        if (tree.root == nullptr)
            return;

        const size_t maxTreeWidth = tree.size;
        const size_t maxTreeHeight = 2 * tree.root->rank;
        const float radius = fminf(GetScreenWidth() / (2.f * maxTreeWidth) * 0.8, 32);

        const float margin = 2 * radius;
        const float drawableWidth = GetScreenWidth() - 2 * margin;
        const float drawableHeight = GetScreenHeight() - STATUS_BAR_HEIGHT - 2 * margin;
        const float cellHeight = drawableHeight / maxTreeHeight;

        std::queue<RBTree<int, NodeMeta>::Node *> q{};
        q.push(tree.root);

        int level = 0;

        while (!q.empty()) {

            int size = q.size(); // the number of nodes at this level

            // traverse all nodes of the tree which are on the same level
            for (int i = 0; i < size; i++) {

                auto node = q.front();
                q.pop();

                if (node->left)
                    q.push(node->left);
                if (node->right)
                    q.push(node->right);

                if (node == tree.root) {
                    node->val.newPos = {margin + drawableWidth / 2, margin + cellHeight / 2};
                } else {
                    float cellWidth = drawableWidth / pow(2, level);
                    if (node->isLeftChild()) {
                        node->val.newPos = {node->parent->val.newPos.x - cellWidth / 2, node->parent->val.newPos.y + cellHeight};
                    } else {
                        node->val.newPos = {node->parent->val.newPos.x + cellWidth / 2, node->parent->val.newPos.y + cellHeight};
                    }
                }

                if (!node->val.initialized) {
                    // generate random coordinates of the new node so that the user can notice the new node
                    node->val.oldPos = node->val.newPos + Vector2{-10.f + rand() % 20, -10.f + rand() % 20};
                    node->val.initialized = true;
                }

                float progress = fminf(animationTimer / ANIMATION_TIME, 1.f);
                if (progress >= 1.f) {
                    node->val.oldPos = node->val.newPos;
                }

                drawNode(node, radius, progress);
            }

            level++;
        }
    }

    void updateDrawFrame() {
        BeginDrawing();

        ClearBackground(Color{16, 16, 16, 255});
        handleInput();
        drawStatusBar();
        animationTimer += GetFrameTime();
        drawTree();
        EndDrawing();
    }
};

Font Visualizer::font;

int main(void) {
    srand(time(NULL));

    SetConfigFlags(FLAG_MSAA_4X_HINT);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1000, 800, "Red-Black Tree visualiser");

    Visualizer::font = LoadFontEx("./assets/IosevkaNerdFontMono-Regular.ttf", 20, NULL, 0);
    Visualizer viz{};

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(viz.(Visualizer::updateDrawFrame), 60, 1);
#else

    while (!WindowShouldClose())
    {
        viz.updateDrawFrame();
    }
#endif

    CloseWindow();

    return 0;
}
