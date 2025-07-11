/* SDL文档链接:https://wiki.libsdl.org/SDL3/SDL_CreateWindow */
// 这里引入sdl2和bool
#include <SDL.h>
#include <stdbool.h>
#include <SDL_ttf.h>

// 定义一下窗口尺寸和迷宫设置
#define WINDOW_WIDTH 1280 // 窗口宽度
#define WINDOW_HEIGHT 720 // 窗口高度
#define CELL_SIZE 40      // 每个迷宫单元格的像素尺寸
#define MAZE_COLS 16
#define MAZE_ROWS 11
#define MAZE_WIDTH MAZE_COLS
#define MAZE_HEIGHT MAZE_ROWS
#define MAZE_PIXEL_WIDTH (MAZE_COLS * CELL_SIZE)
#define MAZE_PIXEL_HEIGHT (MAZE_ROWS * CELL_SIZE)
#define MAZE_OFFSET_X ((WINDOW_WIDTH - MAZE_PIXEL_WIDTH) / 2)
#define MAZE_OFFSET_Y ((WINDOW_HEIGHT - MAZE_PIXEL_HEIGHT) / 2)

// 玩家位置结构体
typedef struct
{
    int x; // 玩家所在的列（水平位置）
    int y; // 玩家所在的行（垂直位置）
} Player;

/* 迷宫地图定义（二维数组）
   0 = 可通过的空地
   1 = 不可通过的墙壁 */
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

/* 绘制迷宫函数 */
void draw_maze(SDL_Renderer *renderer)
{
    // 这里设置renderer的画笔颜色
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    // 这里设置窗口内部的标题
    TTF_Font *font_tittle = TTF_OpenFont("./fonts/cyber.ttf", 64);
    SDL_Color color_tittle = {255, 255, 255, 255};
    SDL_Surface *surface_tittle = TTF_RenderText_Blended(font_tittle, "Greenhats' MAZE", color_tittle);
    SDL_Texture *texture_tittle = SDL_CreateTextureFromSurface(renderer, surface_tittle);

    // 创建一个显示标题文字的矩形
    SDL_Rect rect_tittle;
    SDL_QueryTexture(texture_tittle, NULL, NULL, &rect_tittle.w, &rect_tittle.h);
    // 标题位置
    rect_tittle.x = (WINDOW_WIDTH - rect_tittle.w) / 2;
    rect_tittle.y = 30; // 更靠上，避免和迷宫重叠
    SDL_RenderCopy(renderer, texture_tittle, NULL, &rect_tittle);

    // 绘制迷宫墙体
    SDL_SetRenderDrawColor(renderer, 25, 25, 255, 255); // 设置墙壁为浅灰色
    for (int y = 0; y < MAZE_HEIGHT; y++)
    {
        for (int x = 0; x < MAZE_WIDTH; x++)
        {
            if (maze[y][x] == 1)
            {
                SDL_Rect wall = {
                    x * CELL_SIZE + MAZE_OFFSET_X,
                    y * CELL_SIZE + MAZE_OFFSET_Y,
                    CELL_SIZE,
                    CELL_SIZE};
                SDL_RenderFillRect(renderer, &wall);
            }
        }
    }
    TTF_CloseFont(font_tittle);
    SDL_DestroyTexture(texture_tittle);
    SDL_FreeSurface(surface_tittle);
}

/* 绘制玩家函数 */
void draw_player(SDL_Renderer *renderer, Player *player)
{
    SDL_SetRenderDrawColor(renderer, 255, 100, 100, 255);

    // 计算玩家矩形的位置和大小（比单元格稍小）
    SDL_Rect playerRect = {
        player->x * CELL_SIZE + MAZE_OFFSET_X + CELL_SIZE / 4,
        player->y * CELL_SIZE + MAZE_OFFSET_Y + CELL_SIZE / 4,
        CELL_SIZE / 2,
        CELL_SIZE / 2};
    SDL_RenderFillRect(renderer, &playerRect); // 绘制玩家矩形
}

/* 移动有效性检测函数 */
bool is_valid_move(int x, int y)
{
    // 检查是否超出迷宫边界
    if (x < 0 || x >= MAZE_WIDTH || y < 0 || y >= MAZE_HEIGHT)
        return false;

    // 检查目标位置是否是可通行的空地
    return maze[y][x] == 0;
}

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO); // 目前只初始化视频子系统(初始化视频时,已经同时初始化了事件子系统)
    TTF_Init();               // 初始化sdl字体

    // 这里创建一个窗口(具体参数看官方的文档)
    SDL_Window *window = SDL_CreateWindow(
        "Greenhats' MAZE",      // window title
        SDL_WINDOWPOS_CENTERED, // initial x position
        SDL_WINDOWPOS_CENTERED, // initial y position
        WINDOW_WIDTH,           // width, in pixels
        WINDOW_HEIGHT,          // height, in pixels
        0                       // flags - see below
    );

    // 这里创建一个renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0); // flags=0，那么将选择SDL_RENDERER_ACCELERATED(硬件加速渲染器)

    // 初始化游戏状态
    Player player = {1, 1}; // 初始化玩家位置（左上角起点）
    bool running = true;    // 游戏运行标志
    bool game_won = false;  // 游戏胜利标志

    /* 游戏主循环 */
    while (running)
    {
        SDL_Event event;

        /* 事件处理循环 */
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            { // 处理退出事件
                running = false;
            }
            else if (event.type == SDL_KEYDOWN && !game_won)
            {                         // 处理键盘输入
                int new_x = player.x; // 临时存储新位置
                int new_y = player.y;

                // 根据按键修改坐标
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

                // 如果移动有效则更新玩家位置
                if (is_valid_move(new_x, new_y))
                {
                    player.x = new_x;
                    player.y = new_y;
                }
            }
        }

        /* 游戏胜利或被困判定（到达右下角或被困） */
        if (
            (player.x == MAZE_WIDTH - 2 && player.y == MAZE_HEIGHT - 2) || // 到达终点
            (
                !is_valid_move(player.x + 1, player.y) &&
                !is_valid_move(player.x - 1, player.y) &&
                !is_valid_move(player.x, player.y + 1) &&
                !is_valid_move(player.x, player.y - 1)) // 四周都不能走
        )
        {
            game_won = true;
        }

        /* 渲染流程 */
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        draw_maze(renderer);            // 绘制迷宫和标题
        draw_player(renderer, &player); // 绘制玩家

        SDL_RenderPresent(renderer); // 只在这里Present一次
        SDL_Delay(1);                // 控制帧率（约60FPS）

        // 游戏胜利或被困都弹窗并退出
        if (game_won)
        {
            // 你可以根据需要自定义弹窗内容
            if (player.x == MAZE_WIDTH - 2 && player.y == MAZE_HEIGHT - 2)
                SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "恭喜通关", "你已经成功通关游戏！", window);
            else
                SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Game Over", "你已经无路可走，游戏失败！", window);
            running = false;
        }
    }

    /* 下面是各种内存释放 */
    // 下面是ttf的销毁组建
    TTF_Quit();
    // 下面是销毁三件套
    SDL_DestroyRenderer(renderer); // 销毁渲染器
    SDL_DestroyWindow(window);     // 销毁窗口
    SDL_Quit();                    // 退出SDL系统
    return 0;
}