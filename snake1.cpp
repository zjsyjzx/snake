#include <iostream>
#include <vector>
#include <deque>
#include <random>
#include <cstdlib>
#include <conio.h>
#include <windows.h>

// ============================================================
// 游戏配置常量 (constexpr 替代宏定义)
// ============================================================
namespace Config {
    constexpr int BOARD_W    = 30;   // 逻辑宽度(格)
    constexpr int BOARD_H    = 25;   // 逻辑高度(格)
    constexpr int CHAR_W     = BOARD_W;       // 字符宽度(每格1字符)
    constexpr int WIN_SCORE  = 10;
    constexpr int SPEED_MS   = 80;
}

// ============================================================
// 方向枚举 (enum class 替代裸整数 0,1,2,3)
// ============================================================
enum class Direction { Right, Left, Up, Down };

// ============================================================
// Point 类：二维坐标 (纯数据类，不作为基类)
// ============================================================
class Point {
private:
    int m_x;
    int m_y;
public:
    Point(int x = 0, int y = 0) : m_x(x), m_y(y) {}

    int x() const { return m_x; }
    int y() const { return m_y; }
    void setX(int x) { m_x = x; }
    void setY(int y) { m_y = y; }

    bool operator==(const Point& other) const {
        return m_x == other.m_x && m_y == other.m_y;
    }
};

// ============================================================
// GameObject 抽象基类
//   - 包含纯虚函数 update()，派生类必须重写
//   - 使用 protected 封装坐标，体现封装性
// ============================================================
class GameObject {
protected:
    Point m_pos;
public:
    GameObject(int x, int y) : m_pos(x, y) {}
    virtual ~GameObject() = default;

    virtual void update() = 0;       // 纯虚函数

    int x() const { return m_pos.x(); }
    int y() const { return m_pos.y(); }
    Point pos() const { return m_pos; }
    void setPos(const Point& p) { m_pos = p; }
};

// ============================================================
// Food 类：派生自 GameObject，表示食物
//   - 使用 <random> 替代 rand() (现代C++随机数)
// ============================================================
class Food : public GameObject {
private:
    std::mt19937                     m_rng;
    std::uniform_int_distribution<int> m_distX;
    std::uniform_int_distribution<int> m_distY;

public:
    Food()
        : GameObject(0, 0)
        , m_rng(std::random_device{}())
        , m_distX(1, Config::BOARD_W - 2)
        , m_distY(1, Config::BOARD_H - 2)
    {
        randomize();
    }

    void randomize() {
        setPos(Point(m_distX(m_rng), m_distY(m_rng)));
    }

    void update() override {}        // 食物无需每帧更新，重写为空

    char symbol() const { return '@'; }
};

// ============================================================
// Snake 类：派生自 GameObject，表示蛇
//   - 使用 std::deque<Point> 管理身体 (RAII: 构造初始化，析构自动释放)
//   - 封装方向切换规则
// ============================================================
class Snake : public GameObject {
private:
    std::deque<Point> m_body;        // 蛇身队列 (deque 支持高效头部插入)
    Direction         m_dir;

    static bool isOpposite(Direction a, Direction b) {
        return (a == Direction::Right && b == Direction::Left)  ||
               (a == Direction::Left  && b == Direction::Right) ||
               (a == Direction::Up    && b == Direction::Down)  ||
               (a == Direction::Down  && b == Direction::Up);
    }

public:
    Snake()
        : GameObject(Config::BOARD_W / 2, Config::BOARD_H / 2)
        , m_dir(Direction::Right)
    {
        m_body.emplace_back(x(), y());
        m_body.emplace_back(x() - 1, y());
        m_body.emplace_back(x() - 2, y());
    }

    // ---- 访问器 ----
    const std::deque<Point>& body() const { return m_body; }
    const Point&             head() const { return m_body.front(); }
    Direction                dir()  const { return m_dir; }

    void setDirection(Direction d) {
        if (!isOpposite(m_dir, d))
            m_dir = d;
    }

    // ---- 碰撞检测 ----
    bool hitWall() const {
        const auto& h = head();
        return h.x() <= 0 || h.x() >= Config::BOARD_W - 1 ||
               h.y() <= 0 || h.y() >= Config::BOARD_H - 1;
    }

    bool hitSelf() const {
        const auto& h = head();
        for (size_t i = 1; i < m_body.size(); ++i)
            if (m_body[i] == h) return true;
        return false;
    }

    bool isDead() const { return hitWall() || hitSelf(); }

    bool eats(const Food& f) const { return head() == f.pos(); }

    // ---- 移动 ----
    void move(bool grow) {
        Point next = head();
        switch (m_dir) {
            case Direction::Right: next.setX(next.x() + 1); break;
            case Direction::Left:  next.setX(next.x() - 1); break;
            case Direction::Up:    next.setY(next.y() - 1); break;
            case Direction::Down:  next.setY(next.y() + 1); break;
        }
        m_body.push_front(next);
        if (!grow) m_body.pop_back();
        m_pos = m_body.front();            // 同步基类坐标
    }
    void update() override {}              // 移动由 move() 处理

    void reset() {
        m_body.clear();
        m_dir = Direction::Right;
        m_pos = Point(Config::BOARD_W / 2, Config::BOARD_H / 2);
        m_body.emplace_back(x(), y());
        m_body.emplace_back(x() - 1, y());
        m_body.emplace_back(x() - 2, y());
    }
};

// ============================================================
// Game 类：管理游戏主循环、渲染和输入
// ============================================================
class Game {
private:
    Snake  m_snake;
    Food   m_food;
    int    m_score;
    bool   m_gameover;
    bool   m_paused;
    HANDLE m_console;

    // ---- 控制台辅助 ----
    void gotoxy(int x, int y) const {
        COORD c = { static_cast<SHORT>(x), static_cast<SHORT>(y) };
        SetConsoleCursorPosition(m_console, c);
    }

    void hideCursor() const {
        CONSOLE_CURSOR_INFO info;
        GetConsoleCursorInfo(m_console, &info);
        info.bVisible = FALSE;
        SetConsoleCursorInfo(m_console, &info);
    }

    void showCursor() const {
        CONSOLE_CURSOR_INFO info;
        GetConsoleCursorInfo(m_console, &info);
        info.bVisible = TRUE;
        SetConsoleCursorInfo(m_console, &info);
    }

    // ---- 渲染 ----
    void render() const {
        // 构建字符网格 (vector<string> 替代原始字符串拼接)
        std::vector<std::string> grid(Config::BOARD_H,
                                       std::string(Config::CHAR_W, ' '));

        // 绘制墙壁
        for (int x = 0; x < Config::CHAR_W; ++x) {
            grid[0][x] = '#';
            grid[Config::BOARD_H - 1][x] = '#';
        }
        for (int y = 1; y < Config::BOARD_H - 1; ++y) {
            grid[y][0] = '#';
            grid[y][Config::CHAR_W - 1] = '#';
        }

        // 绘制食物
        grid[m_food.y()][m_food.x()] = m_food.symbol();

        // 绘制蛇
        for (const auto& seg : m_snake.body()) {
            grid[seg.y()][seg.x()] = 'O';
        }

        // 输出到控制台
        gotoxy(0, 0);
        for (const auto& row : grid)
            std::cout << row << '\n';
        std::cout << "Score: " << m_score << "   ";
    }

    // ---- 输入处理 ----
    int readKey() {
        if (!_kbhit()) return -1;
        return _getch();
    }

    void handleInput(int key) {
        switch (key) {
            case 'w': case 'W': m_snake.setDirection(Direction::Up);    break;
            case 's': case 'S': m_snake.setDirection(Direction::Down);  break;
            case 'a': case 'A': m_snake.setDirection(Direction::Left);  break;
            case 'd': case 'D': m_snake.setDirection(Direction::Right); break;
            case 'p': case 'P': m_paused = !m_paused;                   break;
        }
    }

    // ---- 暂停处理 ----
    void handlePause() {
        gotoxy(0, Config::BOARD_H + 1);
        std::cout << "PAUSED - Press P to resume, Q to quit";
        while (m_paused) {
            if (_kbhit()) {
                int key = _getch();
                if (key == 'p' || key == 'P') {
                    m_paused = false;
                    gotoxy(0, Config::BOARD_H + 1);
                    std::cout << "                                        ";
                    return;
                }
                if (key == 'q' || key == 'Q')
                    exit(0);
            }
            Sleep(50);
        }
    }

    // ---- 游戏结束处理 ----
    void showEndScreen() const {
        gotoxy(0, Config::BOARD_H + 1);
        if (m_score >= Config::WIN_SCORE)
            std::cout << "YOU WIN!  ";
        else
            std::cout << "GAME OVER! ";
        std::cout << "Press R to restart, Q to quit";
    }

    bool waitRestart() {
        while (true) {
            if (_kbhit()) {
                int key = _getch();
                if (key == 'r' || key == 'R') return true;
                if (key == 'q' || key == 'Q') return false;
            }
            Sleep(50);
        }
    }

    // ---- 重置 ----
    void reset() {
        m_snake.reset();
        m_food.randomize();
        m_score    = 0;
        m_gameover = false;
        m_paused   = false;
    }

public:
    Game()
        : m_score(0), m_gameover(false), m_paused(false)
        , m_console(GetStdHandle(STD_OUTPUT_HANDLE))
    {
        hideCursor();
    }

    ~Game() {
        showCursor();                    // RAII: 析构时恢复光标
    }

    void run() {
        while (true) {
            system("cls");
            reset();

            while (!m_gameover) {
                render();

                int key = readKey();
                if (key == 'q' || key == 'Q') return;
                if (key != -1) handleInput(key);

                if (m_paused) {
                    handlePause();
                    continue;
                }

                bool eat = m_snake.eats(m_food);
                m_snake.move(eat);

                if (eat) {
                    ++m_score;
                    m_food.randomize();
                }

                if (m_snake.isDead() || m_score >= Config::WIN_SCORE)
                    m_gameover = true;

                Sleep(Config::SPEED_MS);
            }

            render();
            showEndScreen();
            if (!waitRestart()) return;
        }
    }
};

// ============================================================
// 主函数
// ============================================================
int main() {
    Game game;
    game.run();
    return 0;
}
