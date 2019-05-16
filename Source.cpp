#include <iostream>
#include <fstream>
#include <math.h>
#include <Windows.h>
#include <time.h>

using namespace std;

HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

#define DEBUG 0

#define MAX_WIDTH 100
#define MAX_HEIGHT 100
#define M_PI 3.14159265358979323846
#define VIEW_ANGLE (M_PI / 2)
#define MINIMAP_WIDTH 9
#define MINIMAP_HEIGHT 9

#define MAX_COLUMNS 300
#define MAX_ROWS 150

unsigned int mapWidth, mapHeight;
bool map[MAX_WIDTH][MAX_HEIGHT];
unsigned char image[MAX_COLUMNS][MAX_ROWS];

char playerOrientation = 0;
double playerAngle = 0;
double playerPositionX = 0.5;
double playerPositionY = 0.5;

enum Distances { Close = 2, Further = 4, Far = 6, VeryFar = 8, ExtremelyFar = 10 };
enum Distances distance;

void readMap()
{
    ifstream fin;
    fin.open("map.dat");

    if (!fin) {
        cerr << "Unable to open file map.dat";
        exit(1);   // call system to stop
    }

    fin >> mapWidth;
    fin >> mapHeight;
    for (unsigned int i = 0; i < mapWidth; ++i)
        for (unsigned int j = 0; j < mapWidth; ++j)
            fin >> map[i][j];

    fin.close();
}

void printMap()
{
    for (unsigned int i = 0; i < mapWidth; ++i)
    {
        for (unsigned int j = 0; j < mapWidth; ++j)
            cout << map[i][j];
        cout << '\n';
    }
}

bool isOccupiedSpace(double coordX, double coordY)
{
    if (map[(int)ceil(coordX)][(int)ceil(coordY)])
        return 1;

    if ((int)(coordX) == coordX && ceil(coordY) == coordY)
        return map[(int)coordX - 1][(int)coordY - 1] ||
        map[(int)coordX - 1][(int)coordY] ||
        map[(int)coordX][(int)coordY - 1] ||
        map[(int)coordX][(int)coordY];

    if ((int)(coordX) == coordX)
        return map[(int)coordX - 1][(int)ceil(coordY)] ||
        map[(int)coordX][(int)ceil(coordY)];

    if ((int)(coordY) == coordY)
        return map[(int)ceil(coordX)][(int)coordY - 1] ||
        map[(int)ceil(coordX)][(int)coordY];

    return 0;
}

bool collision(double coordX, double coordY)
{
    if (map[(int)ceil(coordX)][(int)ceil(coordY)])
        return 1;

    return 0;
}

double getDistance(double coordX, double coordY, double angle)
{
    double newCoordX = coordX,
        newCoordY = coordY;

    while (!isOccupiedSpace(newCoordX, newCoordY))
    {
        newCoordX += 0.005 * cos(angle);
        newCoordY += 0.005 * sin(angle);
    }

    if (DEBUG)
        cout << newCoordX << ' ' << newCoordY << ' ' << coordX << ' ' << coordY << ' ' << angle << ' ' << endl;

    double x1 = coordX - newCoordX,
        y1 = coordY - newCoordY;
    return sqrt(x1 * x1 + y1 * y1);
}

void drawLineOnImage(unsigned short column, unsigned short noRows, unsigned short lineLength, char asciiValue)
{
    unsigned short rowOffset = noRows / 2 - lineLength;
    if (rowOffset >= noRows / 2)
        rowOffset = 0;

    for (unsigned short row = rowOffset; row < noRows - rowOffset; ++row)
        image[column][row] = asciiValue;
}

void setCursorPosition(int x, int y)
{
    static const HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    std::cout.flush();
    COORD coord = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(hOut, coord);
}

void printImage(char prevImage[MAX_COLUMNS][MAX_ROWS], unsigned short columns, unsigned short rows)
{
    //SetConsoleTextAttribute(hConsole, 20);
    for (unsigned int j = 0; j < rows; ++j)
        for (unsigned int i = 0; i < columns; ++i)
        {
            if (image[i][j] != prevImage[i][j])
            {
                setCursorPosition(i, j);
                printf("%c", image[i][j]);
            }
        }
}


char getMapBlock(short i, short j)
{
    if (0 <= i < mapWidth && 0 <= j < mapHeight)
        if (!map[i][j])
            return 255;

    return 219;
}

char getPlayerCharacter()
{
    switch (playerOrientation)
    {
    case 0: return 195;
    case 1: return 193;
    case 2: return 180;
    case 3: return 194;
    }
}

void drawMinimapBorder(unsigned short columns, unsigned short rows)
{
    for (short i = columns - MINIMAP_WIDTH * 2; i < columns; ++i)
        image[i][rows - MINIMAP_HEIGHT - 1] = 205;
    for (short j = rows - MINIMAP_HEIGHT; j < rows; ++j)
        image[columns - MINIMAP_WIDTH * 2 - 1][j] = 186;
    image[columns - MINIMAP_WIDTH * 2 - 1][rows - MINIMAP_HEIGHT - 1] = 201;
}

void drawMinimap(unsigned short columns, unsigned short rows)
{
    for (short i = -MINIMAP_WIDTH; i < 0; ++i)
        for (short j = -MINIMAP_HEIGHT; j < 0; ++j)
            image[columns + i * 2][rows + j] =
            image[columns + i * 2 + 1][rows + j] =
            getMapBlock(MINIMAP_WIDTH / 2 + i + 2 + (unsigned short)playerPositionX, MINIMAP_HEIGHT / 2 + j + 2 + (unsigned short)playerPositionY);

    image[columns - MINIMAP_WIDTH][rows - MINIMAP_HEIGHT / 2 - 1] = getPlayerCharacter();
    image[columns - MINIMAP_WIDTH - 1][rows - MINIMAP_HEIGHT / 2 - 1] = getPlayerCharacter();

    drawMinimapBorder(columns, rows);
}


void buildImage(unsigned short columns, unsigned short rows)
{
    //clear image
    for (unsigned short i = 0; i < columns; ++i)
        for (unsigned short j = 0; j < columns; ++j)
            image[i][j] = ' ';

    //draw lines
    for (double angle = playerAngle - VIEW_ANGLE / 2, column = 0; angle <= playerAngle + VIEW_ANGLE / 2; angle += VIEW_ANGLE / columns, ++column)
    {
        double dist = getDistance(playerPositionX, playerPositionY, angle);
        unsigned short lineLength = 1 / dist * (rows / 2);
        if (DEBUG)
            cout << dist << '\n';

        char pixelCode = 219;

        if (dist < Close)
            pixelCode = 219;
        else if (dist < Further)
            pixelCode = 178;
        else if (dist < Far)
            pixelCode = 177;
        else if (dist < VeryFar)
            pixelCode = 176;
        else
            pixelCode = 255;

        drawLineOnImage(column, rows, lineLength, pixelCode);
    }

    //drawMinimap(columns, rows);
}


void ShowConsoleCursor(bool showFlag)
{
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);

    CONSOLE_CURSOR_INFO     cursorInfo;

    GetConsoleCursorInfo(out, &cursorInfo);
    cursorInfo.bVisible = showFlag; // set the cursor visibility
    SetConsoleCursorInfo(out, &cursorInfo);
}

void copyImage(char prevImage[MAX_COLUMNS][MAX_ROWS], unsigned short columns, unsigned short rows)
{
    for (unsigned short i = 0; i < columns; ++i)
        memcpy((char*)prevImage[i], (char const*)image[i], rows);
}

int main()
{
    ShowConsoleCursor(false);
    readMap();
    //cout << playerPositionX << playerPositionY;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    HANDLE hOutput;
    HWND hWindow = GetConsoleWindow();
    unsigned short columns, rows;
    char prevImage[MAX_COLUMNS][MAX_ROWS];
    unsigned int frames = 0;
    char titleBuffer[128] = { 0 };

    while (1)
    {
        hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
        GetConsoleScreenBufferInfo(hOutput, &csbi);
        columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

        copyImage(prevImage, columns, rows);
        buildImage(columns, rows);

        if (!DEBUG)
        {
            // cls();
            //printImage(prevImage, columns, rows);

            COORD dwBufferSize = { columns, rows };
            COORD dwBufferCoord = { 0, 0 };
            SMALL_RECT rcRegion = { 0, 0, columns - 1, rows - 1 };

            CHAR_INFO *buffer = new CHAR_INFO[rows * columns];

            for (int i = 0; i < rows; i++)
            {
                for (int j = 0; j < columns; j++)
                {
                    (buffer + i * columns + j)->Char.AsciiChar = image[j][i];
                    
                    if (i < rows / 2)
                    {
                        (buffer + i * columns + j)->Attributes = 0xB7;
                    }
                    else if (image[j][i] != 32)
                    {
                        (buffer + i * columns + j)->Attributes = 0x34;
                    }
                    else
                    {
                        (buffer + i * columns + j)->Attributes = 0x88;
                    }

                }
            }

            WriteConsoleOutput(hOutput, (CHAR_INFO *)buffer, dwBufferSize, dwBufferCoord, &rcRegion);

            sprintf_s(titleBuffer, "rows: %d columns: %d frames: %d extra1: %uc", rows, columns, frames++, image[columns-1][rows-1]);
            SetWindowText(hWindow, titleBuffer);
            Sleep(16);
        }
        else
            Sleep(100000);

        //MOVEMENT
        if (GetKeyState('W') & 0x8000)
        {
            double sn = sin(playerAngle),
                cs = cos(playerAngle);
            if (!collision(playerPositionX + cs, playerPositionY + sn))
            {
                //for (char i = 0; i < 5; i++)
                //{
                playerPositionX += cs / 32;
                playerPositionY += sn / 32;
                copyImage(prevImage, columns, rows);
                buildImage(columns, rows);
                //printImage(prevImage, columns, rows);
                //Sleep(30);
                //}
                playerPositionX += cs / 32;
                playerPositionY += sn / 32;
            }
        }
        if (GetKeyState('S') & 0x8000)
        {
            double sn = sin(playerAngle),
                cs = cos(playerAngle);
            if (!collision(playerPositionX - cs, playerPositionY - sn))
            {
                //for (char i = 0; i < 5; i++)
                //{
                playerPositionX -= cs / 32;
                playerPositionY -= sn / 32;
                copyImage(prevImage, columns, rows);
                buildImage(columns, rows);
                //printImage(prevImage, columns, rows);
                //Sleep(30);
                //}
                playerPositionX -= cs / 32;
                playerPositionY -= sn / 32;
            }
        }
        if (GetKeyState('A') & 0x8000)
        {
            //for (char i = 0; i < 5; i++)
            //{
            playerAngle -= M_PI / 64;
            copyImage(prevImage, columns, rows);
            buildImage(columns, rows);
            //printImage(prevImage, columns, rows);
            //Sleep(30);
            //}
            playerAngle -= M_PI / 64;
            playerOrientation = (playerOrientation + 1) % 4;
        }
        if (GetKeyState('D') & 0x8000)
        {
            //for (char i = 0; i < 5; i++)
            //{
            playerAngle += M_PI / 64;
            copyImage(prevImage, columns, rows);
            buildImage(columns, rows);
            //printImage(prevImage, columns, rows);
            //Sleep(30);
            //}
            playerAngle += M_PI / 64;
            playerOrientation = (playerOrientation + 3) % 4;
        }
    }
}
