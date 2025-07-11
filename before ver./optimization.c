/*
 * SDL3 文档链接：https://wiki.libsdl.org/SDL3/SDL_CreateWindow
 * 本程序实现一个迷宫游戏，包含玩家移动和胜利条件检测
 */

// 头文件引入
#include <SDL.h>     // SDL2 主库
#include <stdbool.h> // 布尔类型支持
#include <SDL_ttf.h> // SDL字体渲染库

// 窗口和游戏参数配置
#define WINDOW_WIDTH 1280                                       // 窗口像素宽度
#define WINDOW_HEIGHT 720                                       // 窗口像素高度
#define CELL_SIZE 40                                            // 迷宫单元格像素尺寸
#define MAZE_COLS 16                                            // 迷宫列数（水平方向单元格数）
#define MAZE_ROWS 11                                            // 迷宫行数（垂直方向单元格数）
#define MAZE_PIXEL_WIDTH (MAZE_COLS * CELL_SIZE)                // 迷宫总像素宽度
#define MAZE_PIXEL_HEIGHT (MAZE_ROWS * CELL_SIZE)               // 迷宫总像素高度
#define MAZE_OFFSET_X ((WINDOW_WIDTH - MAZE_PIXEL_WIDTH) / 2)   // 迷宫水平居中偏移量
#define MAZE_OFFSET_Y ((WINDOW_HEIGHT - MAZE_PIXEL_HEIGHT) / 2) // 迷宫垂直居中偏移量

// 玩家坐标结构体
typedef struct
{
    int x; // 玩家所在列（单元格坐标，0-based）
    int y; // 玩家所在行（单元格坐标，0-based）
} Player;

/* 迷宫地图定义
 * 单元格类型：
 * 0 - 可通过路径 (PATH)
 * 1 - 不可通过墙壁 (WALL) */
int maze[MAZE_ROWS][MAZE_COLS] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1},
    {1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1},
    {1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1},
    {1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1},
    {1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1},
    {1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1}};

/**
 * 绘制迷宫和游戏标题
 * @param renderer SDL渲染器指针
 */
void draw_maze(SDL_Renderer *renderer)
{
    // 设置默认绘制颜色为黑色
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    /* ------ 标题渲染部分 ------ */
    // 加载字体并创建标题纹理
    TTF_Font *font_title = TTF_OpenFont("./fonts/cyber.ttf", 64);
    SDL_Color color_title = {255, 255, 255, 255}; // 白色标题
    SDL_Surface *surface_title = TTF_RenderText_Blended(font_title, "Greenhats' MAZE", color_title);
    SDL_Texture *texture_title = SDL_CreateTextureFromSurface(renderer, surface_title);

    // 计算标题位置（水平居中，上边距30像素）
    SDL_Rect rect_title;
    SDL_QueryTexture(texture_title, NULL, NULL, &rect_title.w, &rect_title.h);
    rect_title.x = (WINDOW_WIDTH - rect_title.w) / 2;
    rect_title.y = 30;
    SDL_RenderCopy(renderer, texture_title, NULL, &rect_title);

    /* ------ 迷宫墙体渲染部分 ------ */
    SDL_SetRenderDrawColor(renderer, 25, 25, 255, 255); // 墙壁颜色（浅灰色）
    for (int y = 0; y < MAZE_ROWS; y++)
    {
        for (int x = 0; x < MAZE_COLS; x++)
        {
            if (maze[y][x] == 1)
            { // 仅绘制墙壁
                SDL_Rect wall = {
                    x * CELL_SIZE + MAZE_OFFSET_X,
                    y * CELL_SIZE + MAZE_OFFSET_Y,
                    CELL_SIZE,
                    CELL_SIZE};
                SDL_RenderFillRect(renderer, &wall);
            }
        }
    }

    // 清理标题资源
    TTF_CloseFont(font_title);
    SDL_DestroyTexture(texture_title);
    SDL_FreeSurface(surface_title);
}

/**
 * 绘制玩家角色
 * @param renderer SDL渲染器指针
 * @param player   玩家对象指针
 */
void draw_player(SDL_Renderer *renderer, Player *player)
{
    SDL_SetRenderDrawColor(renderer, 255, 100, 100, 255); // 玩家颜色（红色）

    // 计算玩家矩形（在单元格内居中，大小为单元格的50%）
    SDL_Rect player_rect = {
        player->x * CELL_SIZE + MAZE_OFFSET_X + CELL_SIZE / 4,
        player->y * CELL_SIZE + MAZE_OFFSET_Y + CELL_SIZE / 4,
        CELL_SIZE / 2,
        CELL_SIZE / 2};
    SDL_RenderFillRect(renderer, &player_rect);
}

/**
 * 验证移动是否合法
 * @param x 目标位置列坐标
 * @param y 目标位置行坐标
 * @return 是否允许移动到该位置
 */
bool is_valid_move(int x, int y)
{
    // 边界检查（确保坐标在迷宫范围内）
    if (x < 0 || x >= MAZE_COLS || y < 0 || y >= MAZE_ROWS)
        return false;

    // 可通行性检查（目标必须为路径）
    return maze[y][x] == 0;
}

int main(int argc, char *argv[])
{
    // 初始化SDL子系统
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    // 创建游戏窗口（居中位置，指定尺寸）
    SDL_Window *window = SDL_CreateWindow(
        "Greenhats' MAZE",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        0);

    // 创建硬件加速渲染器
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // 初始化游戏状态
    Player player = {1, 1}; // 初始位置（1,1）
    bool running = true;    // 主循环运行标志
    bool game_won = false;  // 胜利状态标志

    // 游戏主循环
    while (running)
    {
        SDL_Event event;

        // 事件处理循环
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT: // 窗口关闭事件
                running = false;
                break;

            case SDL_KEYDOWN: // 键盘按下事件
                if (!game_won)
                {
                    // 计算试探性移动位置
                    int new_x = player.x;
                    int new_y = player.y;

                    // 根据按键调整坐标
                    switch (event.key.keysym.sym)
                    {
                    case SDLK_UP:
                        new_y--;
                        break;
                    case SDLK_DOWN:
                        new_y++;
                        break;
                    case SDLK_LEFT:
                        new_x--;
                        break;
                    case SDLK_RIGHT:
                        new_x++;
                        break;
                    }

                    // 验证并应用合法移动
                    if (is_valid_move(new_x, new_y))
                    {
                        player.x = new_x;
                        player.y = new_y;
                    }
                }
                break;
            }
        }

        /* 胜利条件检测：
         * 1. 到达终点（右下角倒数第二个单元格）
         * 2. 被困（四个方向均无法移动） */
        bool at_destination = (player.x == MAZE_COLS - 2 && player.y == MAZE_ROWS - 2);
        bool trapped = !is_valid_move(player.x + 1, player.y) &&
                       !is_valid_move(player.x - 1, player.y) &&
                       !is_valid_move(player.x, player.y + 1) &&
                       !is_valid_move(player.x, player.y - 1);
        game_won = at_destination || trapped;

        // 渲染帧
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // 背景色（黑色）
        SDL_RenderClear(renderer);                      // 清空帧缓冲区

        draw_maze(renderer);            // 绘制迷宫和标题
        draw_player(renderer, &player); // 绘制玩家角色

        SDL_RenderPresent(renderer); // 提交渲染到窗口
        SDL_Delay(16);               // 控制帧率（~60FPS）

        // 游戏结束处理
        if (game_won)
        {
            const char *message = at_destination ? "你已经成功通关游戏！" : "你已经无路可走，游戏失败！";
            SDL_ShowSimpleMessageBox(
                SDL_MESSAGEBOX_INFORMATION,
                at_destination ? "恭喜通关" : "Game Over",
                message,
                window);
            running = false;
        }
    }

    // 资源清理
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}