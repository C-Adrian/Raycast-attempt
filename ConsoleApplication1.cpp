#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <math.h>
#include <Windows.h>
#include <time.h>

using namespace std;

#define DEBUG 0

#define MAX_WIDTH 100
#define MAX_HEIGHT 100
#define M_PI 3.14159265358979323846
#define VIEW_ANGLE (M_PI / 4)

#define MAX_COLUMNS 300
#define MAX_ROWS 150

unsigned int mapWidth, mapHeight;
bool map[MAX_WIDTH][MAX_HEIGHT];
char image[MAX_COLUMNS][MAX_ROWS];

double playerAngle = 0;
double playerPositionX = 1.5;
double playerPositionY = 1.5;

enum Distances {Close = 2, Further = 4, Far = 6, VeryFar = 8, ExtremelyFar = 10};
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
		return map[(int)coordX-1][(int)coordY-1] ||
			   map[(int)coordX-1][(int)coordY  ] ||
			   map[(int)coordX  ][(int)coordY-1] ||
			   map[(int)coordX  ][(int)coordY  ] ;

	if ((int)(coordX) == coordX)
		return map[(int)coordX-1][(int)ceil(coordY)] ||
			   map[(int)coordX  ][(int)ceil(coordY)] ;

	if ((int)(coordY) == coordY)
		return map[(int)ceil(coordX)][(int)coordY-1] ||
			   map[(int)ceil(coordX)][(int)coordY  ] ;

	return 0;
}

bool collision(double coordX, double coordY)
{
	if (map[(int)ceil(coordX)][(int)ceil(coordY)])
		return 1;

	return 0;
}

double getDistance(double coordX,  double coordY, double angle)
{
	double newCoordX = coordX,
		   newCoordY = coordY;

	while (!isOccupiedSpace(newCoordX, newCoordY))
	{
		newCoordX += 0.01 * cos(angle);
		newCoordY += 0.01 * sin(angle);
	}

	if (DEBUG)
		cout << newCoordX << ' ' << newCoordY << ' ' << coordX << ' ' << coordY << ' ' << angle << ' ' << endl;

	double x1 = coordX - newCoordX,
		   y1 = coordY - newCoordY;
	return sqrt(x1 * x1 + y1 * y1);
}

// IMAGE RELATED FUNCTIONS

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

void buildImage(unsigned short columns, unsigned short rows)
{
	//clear image
	for (unsigned short i = 0; i < columns; ++i)
		for (unsigned short j = 0; j < columns; ++j)
			image[i][j] = ' ';

	//draw lines
	//for (double angle = 0; angle <= 2 * M_PI; angle += M_PI/30)
	for (double angle = playerAngle - VIEW_ANGLE / 2, column = 0; angle <= playerAngle + VIEW_ANGLE / 2; angle += VIEW_ANGLE / columns, ++column)
	{
		double dist = getDistance(playerPositionX, playerPositionY, angle);
		unsigned short lineLength = 1/dist * 10;
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
}


void ShowConsoleCursor(bool showFlag)
{
	HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);

	CONSOLE_CURSOR_INFO     cursorInfo;

	GetConsoleCursorInfo(out, &cursorInfo);
	cursorInfo.bVisible = showFlag; // set the cursor visibility
	SetConsoleCursorInfo(out, &cursorInfo);
}


void cls() // black box lol
{
	// Get the Win32 handle representing standard output.
	// This generally only has to be done once, so we make it static.
	static const HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

	CONSOLE_SCREEN_BUFFER_INFO csbi;
	COORD topLeft = { 0, 0 };

	// std::cout uses a buffer to batch writes to the underlying console.
	// We need to flush that to the console because we're circumventing
	// std::cout entirely; after we clear the console, we don't want
	// stale buffered text to randomly be written out.
	std::cout.flush();

	// Figure out the current width and height of the console window
	if (!GetConsoleScreenBufferInfo(hOut, &csbi)) {
		// TODO: Handle failure!
		abort();
	}
	DWORD length = csbi.dwSize.X * csbi.dwSize.Y;

	DWORD written;

	// Flood-fill the console with spaces to clear it
	FillConsoleOutputCharacter(hOut, TEXT(' '), length, topLeft, &written);

	// Reset the attributes of every character to the default.
	// This clears all background colour formatting, if any.
	FillConsoleOutputAttribute(hOut, csbi.wAttributes, length, topLeft, &written);

	// Move the cursor back to the top left for the next sequence of writes
	SetConsoleCursorPosition(hOut, topLeft);
}

void clearScreen()
{

}

void copyImage(char prevImage[MAX_COLUMNS][MAX_ROWS], unsigned short columns, unsigned short rows)
{
	for (unsigned short i = 0; i < columns; ++i)
		memcpy((char*)prevImage[i], (char const*)image[i], rows);
}

void applyMovement()
{
	if (GetKeyState('W') & 0x8000)
	{
		if (!collision(playerPositionX + cos(playerAngle), playerPositionY + sin(playerAngle) ) )
		{
			playerPositionX += cos(playerAngle);
			playerPositionY += sin(playerAngle);
		}
	}
	if (GetKeyState('S') & 0x8000)
	{
		if (!collision(playerPositionX - cos(playerAngle), playerPositionY - sin(playerAngle)))
		{
			playerPositionX -= cos(playerAngle);
			playerPositionY -= sin(playerAngle);
		}
	}
	if (GetKeyState('A') & 0x8000)
	{
		playerAngle -= M_PI / 2;
	}
	if (GetKeyState('D') & 0x8000)
	{
		playerAngle += M_PI / 2;
	}
}

int main()
{
	ShowConsoleCursor(false);
	readMap();
	cout << playerPositionX << playerPositionY;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	unsigned short columns, rows;
	char prevImage[MAX_COLUMNS][MAX_ROWS];

	while (1)
	{
		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
		columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
		rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

		//copyImage(prevImage, columns, rows);
		buildImage(columns, rows);
		
		if (!DEBUG)
		{
			cls();
			printImage(prevImage, columns, rows);
			Sleep(200);
		}
		else
			Sleep(100000);

		applyMovement();
		//playerAngle += M_PI / 90;
		//playerPositionX += 0.5;
	}
}

