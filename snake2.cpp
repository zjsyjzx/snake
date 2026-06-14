#include <iostream>
#include <string>
#include <deque>
#include <vector>
#include <random>
#include <cstdlib>
#include <conio.h>
#include <windows.h>
using namespace std;
const int width = 30;
const int height = 25;
HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
void gotoxy(int x, int y)
{
    COORD coord = {(SHORT)x, (SHORT)y};
    SetConsoleCursorPosition(console, coord);
}
void getkey(int &key)
{
    if (_kbhit())
        key = _getch();
    else
        return;
}
class point
{
protected:
    int x;
    int y;

public:
    point(int x = 0, int y = 0) : x(x), y(y) {}
    int getx() const { return x; }
    int gety() const { return y; }
    void setx(int a) { x = a; }
    void sety(int a) { y = a; }
    virtual void update() {}
    bool operator==(const point &other) const { return x == other.x && y == other.y; }
};
class snake
{
private:
    deque<point> body;
    int direction; // 0为右，1为左，2为上，3为下
public:
    snake()
    {
        direction = 0;
        body.push_back(point(width / 2, height / 2));
        body.push_back(point(width / 2 - 1, height / 2));
        body.push_back(point(width / 2 - 2, height / 2)); // 初始长度为3,方向朝右
    }
    deque<point> &getBody()
    {
        return body;
    }
    void changeDirection(int newDirection)
    {
        if ((direction == 0 && newDirection == 1) || (direction == 1 && newDirection == 0) || (direction == 2 && newDirection == 3) || (direction == 3 && newDirection == 2))
            return; // 180度反方向改变无效
        direction = newDirection;
    }
    bool checkWallCollision()
    {
        point head = body.front();

        // 检查是否超出边界（边框占用了0和width-1）
        if (head.getx() <= 0 || head.getx() >= 2 * width - 1 ||
            head.gety() <= 0 || head.gety() >= height - 1)
        {
            return true; // 撞墙了
        }
        return false;
    }
    bool checkSelfCollision()
    {
        point head = body.front();

        // 从第2节开始检查（第1节是头自己）
        for (size_t i = 1; i < body.size(); i++)
        {
            if (body[i].getx() == head.getx() &&
                body[i].gety() == head.gety())
            {
                return true; // 撞到自己了
            }
        }
        return false;
    }
    bool checkCollision()
    {
        return checkWallCollision() || checkSelfCollision();
    }
    void update(int &key, bool &eat, int &score)
    {

        point newHead = body.front();
        switch (key)
        {
        case 'a':
            changeDirection(1);
            break;
        case 'A':
            changeDirection(1);
            break;
        case 'd':
            changeDirection(0);
            break;
        case 'D':
            changeDirection(0);
            break;
        case 'w':
            changeDirection(2);
            break;
        case 'W':
            changeDirection(2);
            break;
        case 's':
            changeDirection(3);
            break;
        case 'S':
            changeDirection(3);
            break;
        }
        switch (direction)
        {
        case 0:
            newHead.setx(newHead.getx() + 1);
            break;
        case 1:
            newHead.setx(newHead.getx() - 1);
            break;
        case 2:
            newHead.sety(newHead.gety() - 1);
            break;
        case 3:
            newHead.sety(newHead.gety() + 1);
            break;
        }
        if (eat)
        {
            body.push_front(newHead);
            score++;
        }
        else
        {
            body.push_front(newHead);
            body.pop_back();
        }
    }
    void checkEat(bool &eat, const point &a) const
    {
        eat = (body.front() == a);
    }
    void reset()
    {
        body.clear();
        direction = 0;
        body.push_back(point(width / 2, height / 2));
        body.push_back(point(width / 2 - 1, height / 2));
        body.push_back(point(width / 2 - 2, height / 2));
    }
};
class apple : public point
{
public:
    apple()
    {
        int newx = rand() % (width - 2) + 1;
        int newy = rand() % (height - 2) + 1;
        setx(newx);
        sety(newy);
    }
    void update() override
    {
        int newx = rand() % (width - 2) + 1;
        int newy = rand() % (height - 2) + 1;
        setx(newx);
        sety(newy);
    }
    void reset()
    {
        update();
    }
};
class game
{
private:
    string back;
    bool pause;
    snake s;
    apple a;
    bool eat = true;
    bool gameover;
    int key = 'd';
    int score;

public:
    game()
    {
        back = "", pause = false;
        eat = false;
        gameover = false;
        key = 'd';
        score = 0;
    }
    void getkey(int &key)
    {
        if (_kbhit())
            key = _getch();
        else
            return;
    }
    void drawbackground()
    {
        back = "";
        for (int i = 0; i < width; i++)
            back = back + "##";
        back = back + "\n#";
        for (int i = 0; i < height - 2; i++)
        {
            for (int j = 0; j < width - 1; j++)
                back = back + "  ";
            back = back + "#\n#";
        }
        for (int i = 0; i < width - 1; i++)
            back = back + "##";
        back = back + "#\nscore: ";
    }
    void drawsnake(deque<point> &body)
    {
        for (int i = 0; i < body.size(); i++)
        {
            back[body[i].gety() * (width * 2 + 1) + body[i].getx()] = 'O';
        }
    }
    void drawapple(apple &a)
    {
        back[a.gety() * (width * 2 + 1) + a.getx()] = '@';
    }
    void draw(deque<point> &body, apple &a)
    {
        drawbackground();
        drawsnake(body);
        drawapple(a);
        gotoxy(0, 0);
        cout << back << score;
        if (score >= 50)
        {
            cout << "\nYou Win!";
            gameover = true;
        }
    }
    void resetgame()
    {
        s.reset();
        a.reset();
        eat = false;
        gameover = false;
        key = 'd';
        score = 0;
        pause = false;
    }
    void stop()
    {
        gotoxy(0, height);
        cout << "PAUSED - Press P to resume";
        while (pause)
        {
            if (_kbhit())
            {
                int temp = _getch();
                if (temp == 'p' || temp == 'P')
                {
                    pause = false;
                    gotoxy(0, height);
                    cout << "                           ";
                    key = '\0';
                }
                else if (temp == 'q' || temp == 'Q')
                {
                    exit(0);
                }
            }
            Sleep(100);
        }
    }
    void gamerun()
    {
        while (true)
        {
            system("cls");
            resetgame();

            while (!gameover)
            {
                draw(s.getBody(), a);
                getkey(key);

                if (key == 'q' || key == 'Q')
                    return;

                // 处理暂停
                if (key == 'p' || key == 'P')
                {
                    pause = !pause;
                    if (pause)
                    {
                        stop(); // stop 会等待恢复
                    }
                }

                s.checkEat(eat, a);
                s.update(key, eat, score);

                if (s.checkCollision())
                {
                    cout << "\nGame Over! Score: " << score;
                    cout << "\nPress R to restart or Q to quit";
                    gameover = true;

                    // 等待玩家选择
                    while (gameover)
                    {
                        if (_kbhit())
                        {
                            int choice = _getch();
                            if (choice == 'r' || choice == 'R')
                            {
                                break; // 重玩，跳出内层循环
                            }
                            else if (choice == 'q' || choice == 'Q')
                            {
                                return; // 退出程序
                            }
                        }
                        Sleep(100);
                    }
                }

                if (eat)
                {
                    a.update();
                    eat = false;
                }

                Sleep(50);
            }
        }
    }
};
int main()
{
    game b;
    b.gamerun();
    return 0;
}