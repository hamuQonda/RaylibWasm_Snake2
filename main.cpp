#include "raylib.h"
#include "raymath.h"
#include <vector>
#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

// raylib color ���烉���_���ɐF���P�I�Ԋ֐�
static Color GetColorRaylib() {
    Color colors[] = {
    LIGHTGRAY, GRAY, DARKGRAY, YELLOW, GOLD, ORANGE, PINK, RED, MAROON, GREEN,
    LIME, DARKGREEN, SKYBLUE, BLUE, DARKBLUE, PURPLE, VIOLET, DARKPURPLE,
    BEIGE, BROWN, DARKBROWN, MAGENTA, RAYWHITE
    };
    int colorCount = sizeof(colors) / sizeof(colors[0]);

    return colors[GetRandomValue(0, colorCount - 1)];
}

constexpr int screenWidth = 800;
constexpr int screenHeight = 600;
constexpr int worldWidth = 2000;
constexpr int worldHeight = 2000;

const float startR = 12.0f;         // �{�[�����a�̏����l
const int startBallNum = 5;         // �{�[�����̏����l
const float nomalSpeed = 120.0f;    // �ʏ�X�s�[�h�i�s�N�Z�����b�j
const float highSpeed = 240.0f;     // �����X�s�[�h�i�}�E�X���{�^���������j
const float angleIncrement = 360.0f / 1.25f; // �Ȃ���p�x�̑���
const int numFood = 400; // �G�T�̐�
const int initialNumObstacles = 50; // �����̏�Q���̐�
Vector2 ballDirection = { 1, 0 }; // ��������: �E
struct Ball {
    Vector2 position;
    float radius;
    float angle; // ���݂̈ړ������̊p�x�i�x�j
    bool active;
};

struct Food {
    Vector2 position;
    float radius;
    bool active;
    Color color;
};

struct Obstacle {
    Vector2 position = { 0, 0 };
    float radius = 5.0f;
    bool active = true;

    Obstacle() = default;
};

// �����_���ɃG�T��z�u����֐�
static void InitFood(std::vector<Food>& food) {
    for (int i = 0; i < numFood; i++)
    {
        food.push_back(Food{});
    }
    for (auto& f : food) {
        f.position = { (float)GetRandomValue(0, worldWidth), (float)GetRandomValue(0, worldHeight) };
        f.radius = 3.6f;
        f.active = true;
        f.color = GetColorRaylib();
    }
}

// �����_���ɏ�Q����z�u����֐�
static void InitObstacles(std::vector<Obstacle>& obstacles) {
    for (int i = 0; i < initialNumObstacles; i++)
    {
        obstacles.push_back(Obstacle{});
    }
    for (auto& o : obstacles) {
        o.position = { (float)GetRandomValue(0, worldWidth), (float)GetRandomValue(0, worldHeight) };
        o.active = true;
    }
}

// ��Q����1�����_���ɔz�u����֐�
static void AddObstacle(std::vector<Obstacle>& obstacles) {
    Obstacle newObstacle;
    newObstacle.position = { (float)GetRandomValue(0, worldWidth), (float)GetRandomValue(0, worldHeight) };
    newObstacle.active = true;
    obstacles.push_back(newObstacle);
}

// �w�r�̑̃{�[���̏�����
static void InitSnake(std::vector<Ball>& balls) {
    for (int i = 0; i < startBallNum; i++)
    {
        balls.push_back(Ball{});
    }

    // �c��̃{�[����擪�̃{�[���̈ʒu������Ԋu�Ŕz�u
    for (int i = 0; i < balls.size(); ++i) {
        balls[i].radius = startR; // �w�r�̔��a��ݒ�
        balls[i].angle = 0.0f;
        balls[i].position = (i == 0) ? Vector2{ worldWidth / 2.0f, worldHeight / 2.0f } : // ���̏����ʒu
            Vector2{ balls[i - 1].position.x - startR * 2.0f, balls[i - 1].position.y };    // ��͑O�̈ʒu������Ԋu�Ŕz�u
        //balls[i].position = balls[i - 1].position;
        balls[i].active = true;
    }
}

// �S�̓_���܂ލŏ��̋�`���擾����֐�
static Rectangle GetRectangleFromPoints(const std::vector<Vector2>& points) {
    float minX = std::min_element(points.begin(), points.end(), [](Vector2 a, Vector2 b) { return a.x < b.x; })->x;
    float minY = std::min_element(points.begin(), points.end(), [](Vector2 a, Vector2 b) { return a.y < b.y; })->y;
    float maxX = std::max_element(points.begin(), points.end(), [](Vector2 a, Vector2 b) { return a.x < b.x; })->x;
    float maxY = std::max_element(points.begin(), points.end(), [](Vector2 a, Vector2 b) { return a.y < b.y; })->y;
    return { minX, minY, maxX - minX, maxY - minY };
}

// �x�N�g�����w��p�x�ŉ�]
static Vector2 RotateVector(Vector2 v, float degrees) {
    float radians = degrees * DEG2RAD;
    float cosTheta = cos(radians);
    float sinTheta = sin(radians);
    return { v.x * cosTheta - v.y * sinTheta, v.x * sinTheta + v.y * cosTheta };
}

static struct Game {
    Vector2 cameraOffset;
    Camera2D camera;

    std::vector<Ball> balls{};
    std::vector<Food> food;
    std::vector<Obstacle> obstacles{};

    float stopTimer;
    int numFoodEaten;
    int lifeNum;
    int score;
    bool gameOver;

    void InitGm()
    {
        cameraOffset = { screenWidth / 2.0f, screenHeight / 2.0f };
        camera = { {0, 0}, {0, 0}, 0.0f, 1.0f };
        balls.clear();
        InitSnake(balls);
        food.clear();
        InitFood(food);
        obstacles.clear();
        InitObstacles(obstacles);

        stopTimer = 0.0f;
        numFoodEaten = 0;
        score = 0;
        lifeNum = 5;
        gameOver = false;
    }

    Game()
    {
        InitGm();
    }

}gm;

void UpdateDrawFrame();

int main(void)
{
    InitWindow(screenWidth, screenHeight, "Raylib Wasm Snake2");
    //Game gm;

    //std::vector<Ball> balls{};
    //InitSnake(balls);

    //Vector2 cameraOffset = { screenWidth / 2.0f, screenHeight / 2.0f };
    //Camera2D camera = { {0, 0}, {0, 0}, 0.0f, 1.0f };

    //std::vector<Food> food(numFood);
    //InitFood(food);

    //std::vector<Obstacle> obstacles(initialNumObstacles);
    //InitObstacles(obstacles);

    //float stopTimer = 0.0f;
    //int numFoodEaten = 0;
    //int lifeNum = 5;
    //int score = 0;
    //bool gameOver = false;

    gm.InitGm();

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
    SetTargetFPS(60);
    while (!WindowShouldClose())
    {
        UpdateDrawFrame();
    }
#endif

    CloseWindow();

    return 0;
}

void UpdateDrawFrame()
{
    int hitBallNum = 0;
    std::vector<Vector2> vtx = { {},{},{},{} };
    if (!gm.gameOver) {
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // ����
        // Vector2 mousePosition = GetMousePosition();
        Vector2 mousePosition = GetScreenToWorld2D(GetMousePosition(), gm.camera);
        float bSpeed = (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) ? highSpeed : nomalSpeed;
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // �X�V
        float deltaTime = GetFrameTime();

        if (gm.stopTimer <= 0.0f)
        {
            float targetAngle = atan2f(mousePosition.y - gm.balls[0].position.y, mousePosition.x - gm.balls[0].position.x) * (180.0f / PI);
            // �p�x�̍����v�Z���A�K�؂ȕ����Ƀ{�[������]������
            float angleDifference = targetAngle - gm.balls[0].angle;
            if (angleDifference > 180) angleDifference -= 360;
            if (angleDifference < -180) angleDifference += 360;
            float turnAngle = angleIncrement * deltaTime;
            if (fabs(angleDifference) > turnAngle) {
                if (angleDifference > 0) {
                    gm.balls[0].angle += turnAngle;
                }
                else {
                    gm.balls[0].angle -= turnAngle;
                }
            }
            else {
                gm.balls[0].angle = targetAngle;
            }
            ballDirection = Vector2Normalize(Vector2Subtract(mousePosition, gm.balls[0].position));

            Ball bkupBall = { gm.balls[0].position, gm.balls[0].radius, gm.balls[0].angle };
            // �擪�̃{�[���̈ʒu���X�V
            gm.balls[0].position.x += bSpeed * cos((gm.balls[0].angle * PI / 180.0f)) * deltaTime;
            gm.balls[0].position.y += bSpeed * sin((gm.balls[0].angle * PI / 180.0f)) * deltaTime;

            // �c��̃{�[�����擪�̃{�[����ǐ�����悤�Ɉʒu���X�V
            for (int i = 1; i < gm.balls.size(); ++i) {
                Vector2 bkPos = { gm.balls[i].position.x, gm.balls[i].position.y };
                float am = (bSpeed == highSpeed) ? 0.38f : 0.19f;
                //balls[i].position = Vector2Lerp(balls[i].position, balls[i - 1].position, am);
                gm.balls[i].position = Vector2Lerp(gm.balls[i].position, bkupBall.position, am);
                bkupBall = { bkPos, bkupBall.radius, bkupBall.angle };
            }
            //// �c��̃{�[�����擪�̃{�[����ǐ�����悤�Ɉʒu���X�V
            //for (int i = 1; i < balls.size(); ++i) {
            //    float am = bSpeed == highSpeed ? 0.27f : 0.19f;
            //    balls[i].position = Vector2Lerp(balls[i].position, balls[i - 1].position, am);
            //}

            // �G�T�Ƃ̏Փ˔���
            for (auto& f : gm.food) {
                Rectangle recBall0 = {
                    gm.balls[0].position.x - gm.balls[0].radius, gm.balls[0].position.y - gm.balls[0].radius ,
                    gm.balls[0].radius * 2, gm.balls[0].radius * 2
                };
                Rectangle recFood = {
                    f.position.x - f.radius, f.position.y - f.radius ,f.radius * 2, f.radius * 2
                };

                if (f.active && CheckCollisionRecs(recBall0, recFood)) {
                    //if (f.active && CheckCollisionCircles(balls[0].position, balls[0].radius, f.position, f.radius)) {
                    f.active = false;
                    gm.numFoodEaten++;
                    gm.score = gm.numFoodEaten * 10;
                    f.position = { (float)GetRandomValue(0, worldWidth), (float)GetRandomValue(0, worldHeight) };
                    f.active = true;

                    if ((gm.numFoodEaten % 2 == 0)) AddObstacle(gm.obstacles); // �G�T���������H�ׂ邲�Ƃɏ�Q����1�ǉ�

                    // �w�r�̐g�̂̃{�[�����P����
                    Ball addBall = { gm.balls[0].position, startR, gm.balls[0].angle, true };
                    gm.balls.insert(gm.balls.begin(), addBall);
                    //balls.push_back(addBall);
                    //balls.insert(balls.end(), addBall);
                }
            }

            // ���Ƒ�ball[15-19]�Ƃ̏Փ˔���
            if (gm.balls.size() >= 15) {
                for (int i = 15; i < gm.balls.size(); i++)
                {
                    if (i > 19) break;
                    if (CheckCollisionCircles(gm.balls[0].position, gm.balls[0].radius, gm.balls[i].position, gm.balls[i].radius)) {
                        hitBallNum = i;
                        break;
                    }
                }
            }

            // �w�r�ň͂������̏�Q���͏���
            if (hitBallNum > 0) {
                vtx[0] = gm.balls[hitBallNum].position;
                vtx[1] = gm.balls[4].position;
                vtx[2] = gm.balls[9].position;
                vtx[3] = gm.balls[14].position;

                for (int i = 0; i < gm.obstacles.size(); i++)
                {
                    Rectangle rect = GetRectangleFromPoints(vtx);
                    if (CheckCollisionPointRec(gm.obstacles[i].position, rect)) {
                        //obstacles[i].active = false;
                        gm.obstacles.erase(gm.obstacles.begin() + i);
                        gm.score += 50;
                    }
                }
            }

            // ��Q���Ƃ̏Փ˔���
            for (auto& o : gm.obstacles) {
                Rectangle recBall0 = {
                    gm.balls[0].position.x - gm.balls[0].radius * 0.7f,gm.balls[0].position.y - gm.balls[0].radius * 0.7f,
                    gm.balls[0].radius * 2 * 0.7f, gm.balls[0].radius * 2 * 0.7f
                };
                Rectangle recObst = {
                    o.position.x - o.radius * 0.7f, o.position.y - o.radius * 0.7f,
                    o.radius * 2 * 0.7f, o.radius * 2 * 0.7f
                };
                if (o.active && CheckCollisionRecs(recBall0, recObst)) {
                    if (--gm.lifeNum > 0) {
                        gm.balls[0].active = false;
                        o.active = false;
                        gm.stopTimer = 1.2f;
                    }
                    else {
                        o.active = false;
                        gm.gameOver = true;
                    }
                }
                else if (o.active == false) {
                    o.position = { (float)GetRandomValue(0, worldWidth), (float)GetRandomValue(0, worldHeight) };
                    o.active = true;
                }
            }
        }
        else {
            gm.stopTimer -= deltaTime;
            if (gm.stopTimer < 0) {
                gm.stopTimer = 0.0f;
                gm.balls[0].active = true;
            }
        }

    }

    gm.camera.target = gm.balls[0].position;
    gm.camera.offset = gm.cameraOffset;

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // �`��
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    BeginDrawing();
    ClearBackground({ 55,15,55,255 }); // �o�b�N�O���E���h�̐F
    //DrawRectangle(80, 60, 640, 480, { 255,255,5,195 });
    //DrawRectangleGradientEx({ 80, 60, 640, 480 },GOLD,PINK, GOLD, PINK);
    //DrawRectangleGradientEx({ 0, 0, 800, 600 }, BROWN, BLUE, GOLD, RED);

    BeginMode2D(gm.camera);
    DrawRectangle(0, 0, worldWidth, worldHeight, Color{0,0,15,255});

    // �͂��͈̓e�X�g
    {
        if (hitBallNum > 0) {
            vtx[0] = gm.balls[hitBallNum].position;
            vtx[1] = gm.balls[4].position;
            vtx[2] = gm.balls[9].position;
            vtx[3] = gm.balls[14].position;
        }
        DrawLineV(vtx[0], vtx[1], YELLOW);
        DrawLineV(vtx[1], vtx[2], YELLOW);
        DrawLineV(vtx[2], vtx[3], YELLOW);
        DrawLineV(vtx[3], vtx[0], YELLOW);
    }
    /*
    */

    // �ւ�(player)----------------------------------------------------------------------------------------------
    Color color = gm.balls[0].active ? Color{ 15, 255, 15, 255 } : GRAY;
    for (const auto& ball : gm.balls) {
        //DrawCircleLinesV(ball.position, ball.radius, color);
        //DrawCircle(ball.position.x, ball.position.y, ball.radius, color);
        //DrawCircleV(ball.position, ball.radius, color);
        DrawCircleGradient(ball.position.x, ball.position.y, ball.radius, color, { 255,255,255,0 });
    }
    //DrawCircleV(balls[0].position, balls[0].radius / 2.5f, { 55,255,55,205 }); // ��
        // ��
    Vector2 lEyeDir = RotateVector(ballDirection, -40.0f);
    Vector2 rEyeDir = RotateVector(ballDirection, 40.0f);
    float eyeRadius = gm.balls[0].radius / 3.2f;
    Color eyesColor = gm.balls[0].active ? PINK : RAYWHITE;//Color{ 225,225,225,255 };
    DrawCircleV({ gm.balls[0].position.x + lEyeDir.x * 4.0f, gm.balls[0].position.y + lEyeDir.y * 4.0f }, eyeRadius, eyesColor);
    DrawCircleV({ gm.balls[0].position.x + rEyeDir.x * 4.0f, gm.balls[0].position.y + rEyeDir.y * 4.0f }, eyeRadius, eyesColor);
    if (gm.balls[0].active) {
        DrawCircleV({ gm.balls[0].position.x + lEyeDir.x * 4.0f, gm.balls[0].position.y + lEyeDir.y * 4.0f }, eyeRadius / 2.2f, BLACK);
        DrawCircleV({ gm.balls[0].position.x + rEyeDir.x * 4.0f, gm.balls[0].position.y + rEyeDir.y * 4.0f }, eyeRadius / 2.2f, BLACK);
    }

    // 5�{�[�����Ƃ̐�
    for (size_t i = 0; i < gm.balls.size(); i++)
    {
        //if ((i + 1) % 10 == 0) {
        //    DrawCircleV(gm.balls[i].position, gm.balls[i].radius / 3.2f, GREEN); // 5
        //}
        //else
        if ((i + 1) % 5 == 0) {
            DrawCircleV(gm.balls[i].position, gm.balls[i].radius / 4.8f, GREEN); // 5
        }
    }
    /*
    */
    // �ւ�(player)----------------------------------------------------------------------------------------------

    // �G�T
    for (const auto& f : gm.food) {
        if (f.active) {
            //DrawStar(f, YELLOW);
            //DrawCircleV(f.position, f.radius, RED); // �G�T�̐F��ԂɕύX
            DrawCircleGradient(f.position.x, f.position.y, f.radius * 2, f.color, BLANK);
        }
    }

    // ��Q��
    for (const auto& o : gm.obstacles) {
        //if (o.active) {
            //DrawCircleV(o.position, o.radius, DARKGRAY);
        DrawCircleGradient(o.position.x, o.position.y, o.radius * 1.5, RED, { 255,255,255,128 });
        //}
    }

    EndMode2D();
    // ���b�Z�[�W�\��
    // �X�R�A
    DrawText(TextFormat("Score: %05d", gm.score), 4, 4, 20, RAYWHITE);
    DrawText(TextFormat("Life : %d", gm.lifeNum), 180, 4, 20, RAYWHITE);

    if (gm.stopTimer > 0.0f) {
        DrawText(TextFormat("Life : %d", gm.lifeNum), screenWidth / 2 - MeasureText("Life : 0", 50) / 2, screenHeight / 2 - 200, 50, YELLOW);
    }

    if (gm.gameOver) {
        DrawText(TextFormat("Life : %d", gm.lifeNum), screenWidth / 2 - MeasureText("Life : 0", 50) / 2, screenHeight / 2 - 200, 50, YELLOW);
        DrawText("Game  Over!", screenWidth / 2 - MeasureText("Game  Over!", 80) / 2, screenHeight / 2 - 20, 80, YELLOW);
        DrawText(TextFormat("Score: %05d", gm.score), screenWidth / 2 - MeasureText("Score: 00000", 60) / 2, screenHeight / 2 - 140, 60, YELLOW);
        DrawText("Press R to Retry", screenWidth / 2 - MeasureText("Press R to Retry", 40) / 2, screenHeight / 2 + 80, 40, YELLOW);

        if (IsKeyPressed(KEY_R)) {
            ballDirection = { 1, 0 };
            // �Q�[�������Z�b�g
            gm.balls.clear();
            InitSnake(gm.balls);
            gm.food.clear();
            InitFood(gm.food);
            gm.obstacles.clear();
            InitObstacles(gm.obstacles);

            gm.stopTimer = 0.0f;
            gm.numFoodEaten = 0;
            gm.score = 0;
            gm.lifeNum = 5;
            gm.gameOver = false;
        }
    }

    EndDrawing();

}
