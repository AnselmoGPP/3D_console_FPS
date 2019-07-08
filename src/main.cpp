// https://www.youtube.com/watch?v=xW8skO7MFYw
// TODO: Bigger screen, zoom out, frames per second resolution.

#include <Windows.h>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <utility>
#include <iomanip>
#include <wchar.h>
#include <string>
#include <vector>
#include <chrono>

int nScreenWidth = 120;
int nScreenHeight = 40;

float fPlayerX = 8.0f;		// Player's X position
float fPlayerY = 8.0f;		// Player's Y position
float fPlayerA = 0.0f;		// Angle player is looking at

int nMapHeight = 16;
int nMapWidth = 16;

float fFOV = 3.14159 / 4.0;
float fDepth = 16.0f;

int main()
{
	//for (int i = 0; i < 256; i++) std::cout << i << ':' << (char)i << ' ';
	//system("pause");
	
	// Create screen buffer
	char *screen = new char[nScreenWidth * nScreenHeight];			// wchar_t *screen = new wchar_t[nScreenWidth * nScreenHeight];			// LPCSTR *screen = new LPCSTR[nScreenWidth * nScreenHeight];
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	// Configure window
	//HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	_COORD screenBufSize = { nScreenWidth, nScreenHeight };
	bool WINAPI screen_buff = SetConsoleScreenBufferSize(hConsole, screenBufSize);
	SMALL_RECT windowSize = { 0, 0, nScreenWidth, nScreenHeight };
	bool WINAPI resiz = SetConsoleWindowInfo(hConsole, TRUE, &windowSize);
	SetConsoleTitle((LPCSTR)"Hans Console 3D engine");

	std::wstring map;
	map += L"################";
	map += L"#..............#";
	map += L"#..####..#######";
	map += L"#..#...........#";
	map += L"#..#...........#";
	map += L"#..#...........#";
	map += L"#..#...........#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..#...........#";
	map += L"#..#...........#";
	map += L"#..#...........#";
	map += L"#..#...........#";
	map += L"#..####..#######";
	map += L"#...............";
	map += L"################";

	auto tp1 = std::chrono::system_clock::now();
	auto tp2 = std::chrono::system_clock::now();

	// Game loop
	while (1) 
	{
		tp2 = std::chrono::system_clock::now();
		std::chrono::duration<float> elapsedTime = tp2 - tp1;
		tp1 = tp2;
		float fElapsedTime = elapsedTime.count();

		// Controls -----------------------------------------------------------------------------------------------
		// Handle CCW Rotation
		if (GetAsyncKeyState((unsigned short)'A') & 0x8000)
			fPlayerA -= (0.8f) * fElapsedTime;

		if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
			fPlayerA += (0.8f) * fElapsedTime;

		if (GetAsyncKeyState((unsigned short) 'W') & 0x8000)
		{
			fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;
			fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;

			if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#') 
			{
				fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime;
				fPlayerY -= cosf(fPlayerA) * 5.0f * fElapsedTime;
			}
		}

		if (GetAsyncKeyState((unsigned short) 'S') & 0x8000)
		{
			fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime;
			fPlayerY -= cosf(fPlayerA) * 5.0f * fElapsedTime;

			if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#')
			{
				fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;
				fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;
			}
		}

		if (GetAsyncKeyState((unsigned short) 'Q') & 0x8000)
		{
			fPlayerX -= sinf(fPlayerA + 3.14159 / 2) * 5.0f * fElapsedTime;
			fPlayerY -= cosf(fPlayerA + 3.14159 / 2) * 5.0f * fElapsedTime;

			if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#')
			{
				fPlayerX += sinf(fPlayerA + 3.14159 / 2) * 5.0f * fElapsedTime;
				fPlayerY += cosf(fPlayerA + 3.14159 / 2) * 5.0f * fElapsedTime;
			}
		}

		if (GetAsyncKeyState((unsigned short) 'E') & 0x8000)
		{
			fPlayerX -= sinf(fPlayerA - 3.14159/2) * 5.0f * fElapsedTime;
			fPlayerY -= cosf(fPlayerA - 3.14159 / 2) * 5.0f * fElapsedTime;

			if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#')
			{
				fPlayerX += sinf(fPlayerA - 3.14159 / 2) * 5.0f * fElapsedTime;
				fPlayerY += cosf(fPlayerA - 3.14159 / 2) * 5.0f * fElapsedTime;
			}
		}

		// Ray tracing computations. Compute distances when wall is within the field of view ----------------------
		for (int x = 0; x < nScreenWidth; x++) 
		{
			// For each column, calculate the projected ray angle into world space
			float fRayAngle = (fPlayerA - fFOV / 2.0f) + (fFOV / (float)nScreenWidth) * (float)x;

			float fDistanceToWall = 0;
			bool bHitWall = false;
			bool bBoundary = false;			// For detecting the edge of a cell

			float fEyeX = sinf(fRayAngle);	// Unit vector for ray in player space
			float fEyeY = cosf(fRayAngle);
			
			while (!bHitWall && fDistanceToWall < fDepth) 
			{
				fDistanceToWall += 0.1f;

				int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
				int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);

				// Test if ray is out of bounds
				if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight) 
				{
					bHitWall = true;		// Just set distance to maximum depth
					fDistanceToWall = fDepth;
				}
				else
				{
					// Ray is inbounds to test to see if the ray cell is a wall block
					if (map[nTestY * nMapWidth + nTestX] == '#') 
					{
						bHitWall = true;

						std::vector<std::pair<float, float>> p;		// distance, dot

						for (int tx = 0; tx < 2; tx++)
							for(int ty = 0; ty < 2; ty++)
							{
								float vy = (float)nTestY + ty - fPlayerY;
								float vx = (float)nTestX + tx - fPlayerX;
								float d = sqrt(vx*vx + vy * vy);
								float dot = (fEyeX * vx / d) + (fEyeY * vy / d);
								p.push_back(std::make_pair(d, dot));
							}

						// Sort Pairs from closest to farthest
						std::sort(p.begin(), p.end(), [](const std::pair<float, float> &left, const std::pair<float, float> &right) { return left.first < right.first; });

						float fBound = 0.01;
						if (acos(p.at(0).second) < fBound) bBoundary = true;
						if (acos(p.at(1).second) < fBound) bBoundary = true;
						//if (acos(p.at(2).second) < fBound) bBoundary = true;
					}
				}
			}

			// Drawing --------------------------------------------------------------------------------------------
			// Calculate distance to ceiling and floor
			int nCeiling = (float)(nScreenHeight / 2.0) - nScreenHeight / ((float)fDistanceToWall);
			int nFloor = nScreenHeight - nCeiling;

			char nShade = ' ';

			if (fDistanceToWall <= fDepth / 4.0f)		nShade = (char)219; //0x2588;	// Very close
			else if (fDistanceToWall < fDepth / 3.0f)	nShade = (char)178; //0x2593;
			else if (fDistanceToWall < fDepth / 2.0f)	nShade = (char)177; //0x2592;
			else if (fDistanceToWall < fDepth)			nShade = (char)176; //0x2591;
			else										nShade = ' ';					// Too far away

			if (bBoundary)								nShade = ' ';

			for (int y = 0; y < nScreenHeight; y++)
			{
				if (y <= nCeiling)							// Sky
					screen[y * nScreenWidth + x] = ' ';
				else if (y > nCeiling && y <= nFloor)		// Wall
					screen[y * nScreenWidth + x] = nShade;
				else										// Floor
				{
					// Shade floor based on distance
					float b = 1.0f - (((float)y - nScreenHeight / 2.0f) / ((float)nScreenHeight / 2.0f));
					if (b < 0.25)					nShade = '#';
					else if (b < 0.5)				nShade = 'x';
					else if (b < 0.75)				nShade = '.';
					else if (b < 0.9)				nShade = '-';
					else							nShade = ' ';
					screen[y * nScreenWidth + x] = nShade;
				}
			}
		}

		// Display stats
		//swprintf_s(screen, 40, L"X=%3.2f, Y=%3.2f, A=%3.2f, FPS=%3.2f", fPlayerX, fPlayerY, fPlayerA, 1.0f / fElapsedTime);
		sprintf(screen, "X=%3.2f, Y=%3.2f, A=%3.2f, FPS=%3.0f", fPlayerX, fPlayerY, fPlayerA, 1.0f / fElapsedTime);
		
		// Display map
		for (int nx = 0; nx < nMapWidth; nx++)
			for (int ny = 0; ny < nMapWidth; ny++)
				screen[(ny + 1)*nScreenWidth + nx] = map[ny * nMapWidth + nx];

		screen[((int)fPlayerY + 1) * nScreenWidth + (int)fPlayerX] = 'o';

		screen[nScreenWidth * nScreenHeight - 1] = '\0';
		LPCSTR screenNow = screen;										// LPCSTR: const char*	  <<<<<<<<<<<<<<<<< PROBLEM: We are forced here to use LPCSTR (const char^), so we have to create this every time.
		WriteConsoleOutputCharacter(hConsole,							// A handle to console screen buffer
									screenNow,							// Characters to be written to the console screen buffer
									nScreenWidth * nScreenHeight,		// Number of characters to be written
									{ 0,0 },							// COORD structure that specifies the character coordinates of the first cell in the buffer to which characters will be written
									&dwBytesWritten);					// Pointer to a variable that receives the number of characters actually written
	}

}