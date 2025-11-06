#include <iostream>
#include <vector>
#include <conio.h>
#include <windows.h>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <string>

using namespace std;

// Game configuration
const int WIDTH = 60;
const int HEIGHT = 20;
int speed = 100; // milliseconds per move (lower = faster)

bool borderMode = true; // true = border kills, false = wrap around

// Game state
enum Direction
{
    STOP = 0,
    LEFT,
    RIGHT,
    UP,
    DOWN
};
Direction dir = STOP;

struct Position
{
    int x, y;
    Position(int x = 0, int y = 0) : x(x), y(y) {}
    bool operator==(const Position &other) const
    {
        return x == other.x && y == other.y;
    }
};

vector<Position> snake;
Position food;
int score = 0;
bool gameOver = false;

// Helper functions

// Move cursor to a specific coordinate
void gotoxy(int x, int y)
{
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// Hide the blinking cursor
void hideCursor() // called by setupConsole() 
{
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
}

// Set up the console window
void setupConsole()
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    const int totalWidth = WIDTH + 2;
    const int totalHeight = HEIGHT + 2 + 2;

    // Set console buffer and window size
    COORD bufferSize = {totalWidth, totalHeight};
    SetConsoleScreenBufferSize(hConsole, bufferSize);

    SMALL_RECT windowSize = {0, 0, totalWidth - 1, totalHeight - 1};
    SetConsoleWindowInfo(hConsole, TRUE, &windowSize);

    hideCursor();
}

// Initialize the game
void Setup()
{
    gameOver = false;
    dir = STOP;
    score = 0;

    // Start snake in the middle with initial length of 3
    snake.clear();
    int startX = WIDTH / 2;
    int startY = HEIGHT / 2;
    snake.push_back(Position(startX, startY));
    snake.push_back(Position(startX - 1, startY));
    snake.push_back(Position(startX - 2, startY));

    // Generate initial food
    srand((unsigned int)time(0));
    do
    {
        food.x = rand() % WIDTH;
        food.y = rand() % HEIGHT;
    } while (find(snake.begin(), snake.end(), food) != snake.end());
}

// Draw the game board using WriteConsoleOutput for flicker-free rendering
void Draw()
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    const int totalHeight = HEIGHT + 2 + 2; // Game area + borders + 2 lines for score/instructions
    const int totalWidth = WIDTH + 2;

    // Create character buffer
    CHAR_INFO *buffer = new CHAR_INFO[totalWidth * totalHeight];

    // Initialize buffer with spaces
    for (int i = 0; i < totalWidth * totalHeight; i++)
    {
        buffer[i].Char.AsciiChar = ' ';
        buffer[i].Attributes = 7; // White on black
    }

    int idx = 0;

    // Draw top border
    for (int i = 0; i < totalWidth; i++)
    {
        buffer[idx].Char.AsciiChar = '#';
        buffer[idx].Attributes = 7;
        idx++;
    }

    // Draw game area
    for (int i = 0; i < HEIGHT; i++)
    {
        // Left border
        buffer[idx].Char.AsciiChar = '#';
        buffer[idx].Attributes = 7;
        idx++;

        // Game cells
        for (int j = 0; j < WIDTH; j++)
        {
            bool drawn = false;

            // Draw snake head
            if (i == snake[0].y && j == snake[0].x)
            {
                buffer[idx].Char.AsciiChar = 'O';
                buffer[idx].Attributes = 10; // Green
                drawn = true;
            }
            // Draw snake body
            else
            {
                for (size_t k = 1; k < snake.size(); k++)
                {
                    if (snake[k].x == j && snake[k].y == i)
                    {
                        buffer[idx].Char.AsciiChar = 'o';
                        buffer[idx].Attributes = 10; // Green
                        drawn = true;
                        break;
                    }
                }
            }

            // Draw food
            if (!drawn && i == food.y && j == food.x)
            {
                buffer[idx].Char.AsciiChar = '*';
                buffer[idx].Attributes = 12; // Red
                drawn = true;
            }

            // Empty space (already initialized)
            if (!drawn)
            {
                buffer[idx].Char.AsciiChar = ' ';
                buffer[idx].Attributes = 7;
            }

            idx++;
        }

        // Right border
        buffer[idx].Char.AsciiChar = '#';
        buffer[idx].Attributes = 7;
        idx++;
    }

    // Draw bottom border
    for (int i = 0; i < totalWidth; i++)
    {
        buffer[idx].Char.AsciiChar = '#';
        buffer[idx].Attributes = 7;
        idx++;
    }

    // Draw score line
    string scoreStr = "Score: " + to_string(score);
    for (size_t i = 0; i < scoreStr.length() && i < totalWidth; i++)
    {
        buffer[idx].Char.AsciiChar = scoreStr[i];
        buffer[idx].Attributes = 7;
        idx++;
    }
    // Fill rest with spaces
    while (idx % totalWidth != 0)
    {
        buffer[idx].Char.AsciiChar = ' ';
        idx++;
    }

    // Draw instructions line
    string instStr = "Use W/A/S/D to move. Press 'X' to quit.";
    for (size_t i = 0; i < instStr.length() && i < totalWidth; i++)
    {
        buffer[idx].Char.AsciiChar = instStr[i];
        buffer[idx].Attributes = 7;
        idx++;
    }

    // Write to console
    COORD bufferSize = {totalWidth, totalHeight};
    COORD bufferCoord = {0, 0};
    SMALL_RECT writeRegion = {0, 0, totalWidth - 1, totalHeight - 1};

    WriteConsoleOutput(hConsole, buffer, bufferSize, bufferCoord, &writeRegion);

    delete[] buffer;
}

// Handle keyboard input (non-blocking)
void Input()
{
    if (_kbhit())
    {
        char key = _getch();
        key = toupper(key);

        switch (key)
        {
        case 'A':
            if (dir != RIGHT)
                dir = LEFT;
            break;
        case 'D':
            if (dir != LEFT)
                dir = RIGHT;
            break;
        case 'W':
            if (dir != DOWN)
                dir = UP;
            break;
        case 'S':
            if (dir != UP)
                dir = DOWN;
            break;
        case 'X':
            gameOver = true;
            break;
        }
    }
}

// Update game logic
void Logic()
{
    if (dir == STOP)
        return;

    // Calculate new head position
    Position newHead = snake[0];

    switch (dir)
    {
    case LEFT:
        newHead.x--;
        break;
    case RIGHT:
        newHead.x++;
        break;
    case UP:
        newHead.y--;
        break;
    case DOWN:
        newHead.y++;
        break;
    }

    // for handling border / without border
    // Handle wall collision or wrapping
    if (borderMode)
    {
        // Border mode: hitting wall = game over
        if (newHead.x < 0 || newHead.x >= WIDTH ||
            newHead.y < 0 || newHead.y >= HEIGHT)
        {
            gameOver = true;
            return;
        }
    }
    else
    {
        // Wrap-around mode
        if (newHead.x < 0)
            newHead.x = WIDTH - 1;
        else if (newHead.x >= WIDTH)
            newHead.x = 0;

        if (newHead.y < 0)
            newHead.y = HEIGHT - 1;
        else if (newHead.y >= HEIGHT)
            newHead.y = 0;
    } // border over!!

    // Check self collision (exclude tail since it will move forward)
    if (snake.size() > 1)
    {
        for (size_t i = 0; i < snake.size() - 1; i++)
        {
            if (snake[i] == newHead)
            {
                gameOver = true;
                return;
            }
        }
    }

    // Add new head
    snake.insert(snake.begin(), newHead);

    // Check if food eaten
    if (newHead.x == food.x && newHead.y == food.y)
    {
        score++;

        // Generate new food
        bool foodOnSnake;
        int attempts = 0;
        do
        {
            foodOnSnake = false;
            food.x = rand() % WIDTH;
            food.y = rand() % HEIGHT;

            for (size_t i = 0; i < snake.size(); i++)
            {
                if (snake[i] == food)
                {
                    foodOnSnake = true;
                    break;
                }
            }
            attempts++;
            if (attempts > 1000)
                break; // Safety check to prevent infinite loop
        } while (foodOnSnake);

        // Optional: Speed up slightly as score increases
        if (speed > 40)
            speed -= 2;
    }
    else
    {
        // Remove tail if food not eaten
        snake.pop_back();
    }
}

int main()
{
    // Set up console
    system("cls");
    setupConsole();

    cout << "=====================================\n";
    cout << "         WELCOME TO SNAKE GAME       \n";
    cout << "=====================================\n\n";

    // -------------------- GAME MODE SELECTION --------------------
    cout << "Choose Game Mode:\n";
    cout << "1. Border Mode (hit wall = game over)\n";
    cout << "2. No Borders (snake wraps around)\n";
    cout << "Enter choice (1 or 2): ";

    char modeChoice;
    cin >> modeChoice;
    borderMode = (modeChoice == '1');

    system("cls");
    setupConsole();

    // -------------------- DIFFICULTY SELECTION --------------------
    cout << "Select Difficulty Level:\n";
    cout << "1. Easy   (Slow snake)\n";
    cout << "2. Medium (Balanced speed)\n";
    cout << "3. Hard   (Fast snake)\n";
    cout << "Enter choice (1/2/3): ";

    char diffChoice;
    cin >> diffChoice;

    // set initial speed according to difficulty
    switch (diffChoice)
    {
    case '1':
        speed = 130; // slower = easier
        break;
    case '3':
        speed = 70; // faster = harder
        break;
    default:
        speed = 100; // medium default
        break;
    }

    cout << "\nGame Mode: " << (borderMode ? "Border" : "No Border") << endl;
    cout << "Difficulty: " << (diffChoice == '1' ? "Easy" : (diffChoice == '3' ? "Hard" : "Medium")) << endl;
    cout << "\nPress any key to start...";
    _getch();

    // Clear and initialize game
    system("cls");
    setupConsole();
    Setup();

    // -------------------- MAIN GAME LOOP --------------------
    while (!gameOver)
    {
        Draw();
        Input();
        Logic();
        Sleep(speed);
    }

    // -------------------- GAME OVER SCREEN --------------------
    system("cls");
    cout << "#########################\n";
    cout << "#      GAME OVER!       #\n";
    cout << "#########################\n";
    cout << "Final Score: " << score << "\n";
    cout << "Press any key to exit...\n";
    _getch();

    return 0;
}
