#include <iostream>
#include <string>
#include <deque>
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
enum class direction : int
{
    right = 0,
    left = 1,
    up = 2,
    down = 3
};
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
    virtual void update(int &key, bool &eat, int &score, bool &gameover) {}
    bool operator==(const point &other) const { return x == other.x && y == other.y; }
};
class snake : public point
{
private:
    deque<snake *> body;
    direction d;

public:
    snake(int x, int y) : point(x, y)
    {
        d = direction::right;
    }
    snake() : point(width / 2, height / 2)
    {
        d = direction::right;
        body.push_back(this);
        body.push_back(new snake(width / 2 - 1, height / 2));
        body.push_back(new snake(width / 2 - 2, height / 2));
    }
    ~snake()
    {
        for (auto &x : body)
        {
            if (x != this)
                delete x;
        }
    }
    deque<snake *> &getBody()
    {
        return body;
    }
    void changeDirection(direction newDirection)
    {
        if ((d == direction::right && newDirection == direction::left) ||
            (d == direction::left && newDirection == direction::right) ||
            (d == direction::up && newDirection == direction::down) ||
            (d == direction::down && newDirection == direction::up))
            return;
        d = newDirection;
    }
    bool checkWallCollision()
    {
        if (x <= 0 || x >= 2 * width - 1 ||
            y <= 0 || y >= height - 1)
        {
            return true;
        }
        return false;
    }
    bool checkSelfCollision()
    {
        for (size_t i = 1; i < body.size(); i++)
        {
            if (body[i]->getx() == x && body[i]->gety() == y)
            {
                return true;
            }
        }
        return false;
    }
    bool checkCollision()
    {
        return checkWallCollision() || checkSelfCollision();
    }
    void update(int &key, bool &eat, int &score, bool &gameover) override
    {
        switch (key)
        {
        case 'a':
        case 'A':
            changeDirection(direction::left);
            break;
        case 'd':
        case 'D':
            changeDirection(direction::right);
            break;
        case 'w':
        case 'W':
            changeDirection(direction::up);
            break;
        case 's':
        case 'S':
            changeDirection(direction::down);
            break;
        }
        int newX = x;
        int newY = y;
        switch (d)
        {
        case direction::right:
            newX += 1;
            break;
        case direction::left:
            newX -= 1;
            break;
        case direction::up:
            newY -= 1;
            break;
        case direction::down:
            newY += 1;
            break;
        }
        int oldX = x;
        int oldY = y;
        x = newX;
        y = newY;
        if (checkCollision())
        {
            gameover = true;
            x = oldX;
            y = oldY;
            return;
        }
        x = oldX;
        y = oldY;
        int oldTailX = body.back()->getx();
        int oldTailY = body.back()->gety();
        for (int i = body.size() - 1; i > 0; i--)
        {
            body[i]->setx(body[i - 1]->getx());
            body[i]->sety(body[i - 1]->gety());
        }
        body[0]->setx(newX);
        body[0]->sety(newY);
        x = newX;
        y = newY;
        if (eat)
        {
            snake *newSegment = new snake(oldTailX, oldTailY);
            body.push_back(newSegment);
            score++;
        }
    }
    void checkEat(bool &eat, const point &a) const
    {
        eat = (x == a.getx() && y == a.gety());
    }
    void reset()
    {
        for (auto &seg : body)
        {
            if (seg != this)
                delete seg;
        }
        body.clear();
        d = direction::right;
        x = width / 2;
        y = height / 2;
        body.push_back(this);
        body.push_back(new snake(width / 2 - 1, height / 2));
        body.push_back(new snake(width / 2 - 2, height / 2));
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
    void update(int &key, bool &eat, int &score, bool &gameover) override
    {
        if (eat)
        {
            int newx = rand() % (width - 2) + 1;
            int newy = rand() % (height - 2) + 1;
            setx(newx);
            sety(newy);
        }
    }
    void reset()
    {
        int newx = rand() % (width - 2) + 1;
        int newy = rand() % (height - 2) + 1;
        setx(newx);
        sety(newy);
    }
};
class game
{
private:
    string back;
    bool pause;
    snake s;
    apple a;
    bool eat;
    bool gameover;
    int key;
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
    void drawsnake(deque<snake *> &body)
    {
        for (const auto &seg : body)
        {
            int idx = seg->gety() * (width * 2 + 1) + seg->getx();
            if (idx >= 0 && idx < (int)back.length())
                back[idx] = 'O';
        }
    }
    void drawapple(apple &a)
    {
        back[a.gety() * (width * 2 + 1) + a.getx()] = '@';
    }
    void draw(deque<snake *> &body, apple &a)
    {
        drawbackground();
        drawsnake(body);
        drawapple(a);
        gotoxy(0, 0);
        cout << back << score;
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
                        stop();
                    }
                }

                if (pause)
                {
                    Sleep(50);
                    continue;
                }

                s.checkEat(eat, a);
                if (score >= 10)
                {
                    gameover = true;
                }
                s.update(key, eat, score, gameover); // 传入 gameover

                if (gameover)
                {
                    if (score >= 10)
                        cout << "\nYou Win!";
                    else
                        cout << "\nGame over!";
                    cout << "\nPress R to restart or Q to quit";

                    while (gameover)
                    {
                        if (_kbhit())
                        {
                            int choice = _getch();
                            if (choice == 'r' || choice == 'R')
                            {
                                break;
                            }
                            else if (choice == 'q' || choice == 'Q')
                            {
                                return;
                            }
                        }
                        Sleep(100);
                    }
                }

                if (eat)
                {
                    a.update(key, eat, score, gameover);
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