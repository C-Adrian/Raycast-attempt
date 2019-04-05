#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <math.h>
#include <Windows.h>
#include <time.h>

using namespace std;

#define MAX_WIDTH 100
#define MAX_HEIGHT 100
#define M_PI 3.14159265358979323846
#define VIEW_ANGLE M_PI / 3

#define MAX_COLUMNS 300
#define MAX_ROWS 150

unsigned int mapWidth, mapHeight;
bool map[MAX_WIDTH][MAX_HEIGHT];
char image[MAX_COLUMNS][MAX_ROWS];

double playerAngle;
double playerPositionX = 6;
double playerPositionY = 6;

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

double getDistance(double coordX,  double coordY, double angle)
{
	double newCoordX = coordX,
		   newCoordY = coordY;

	while (!isOccupiedSpace(newCoordX, newCoordY))
	{
		newCoordX += 0.1 * cos(angle);
		newCoordY += 0.1 * sin(angle);
	}

	return hypot(coordX - newCoordY, coordY - newCoordY);
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

void printImage(unsigned short columns, unsigned short rows)
{
	for (unsigned int j = 0; j < rows; ++j)
		for (unsigned int i = 0; i < columns; ++i)
			printf("%c", image[i][j]);
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
		unsigned short lineLength = 1/getDistance(playerPositionX, playerPositionY, angle) * 10;
		cout << getDistance(playerPositionX, playerPositionY, angle) << '\n';
		drawLineOnImage(column, rows, lineLength, 219);
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

int main()
{
	ShowConsoleCursor(false);
	readMap();

	CONSOLE_SCREEN_BUFFER_INFO csbi;
	unsigned short columns, rows;

	while (1)
	{
		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
		columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
		rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

		buildImage(columns, rows);
		//system("CLS");
		//printImage(columns, rows);
		Sleep(100000);

		playerAngle += M_PI / 90;
		//playerPositionX += 0.5;
	}
}

