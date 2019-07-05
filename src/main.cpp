// https://www.youtube.com/watch?v=xW8skO7MFYw

#include <Windows.h>
#include <iostream>
#include <string>
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
	// Create screen buffer
	char *screen = new char[nScreenWidth * nScreenHeight];			// wchar_t *screen = new wchar_t[nScreenWidth * nScreenHeight];			// LPCSTR *screen = new LPCSTR[nScreenWidth * nScreenHeight];
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	//HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	//SMALL_RECT windowSize = { 0, 0, 40, 40 };
	//SetConsoleWindowInfo(hConsole, TRUE, &windowSize);
	//SetConsoleTitle((LPCSTR)"Console 3D engine");

	std::wstring map;
	map += L"################";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
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


		// Controls
		// Handle CCW Rotation
		if (GetAsyncKeyState((unsigned short)'A') & 0x8000)
			fPlayerA -= (0.8f) * fElapsedTime;

		if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
			fPlayerA += (0.8f) * fElapsedTime;

		if (GetAsyncKeyState((unsigned short) 'W') & 0x8000)
		{
			fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;
			fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;
		}

		if (GetAsyncKeyState((unsigned short) 'S') & 0x8000)
		{
			fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime;
			fPlayerY -= cosf(fPlayerA) * 5.0f * fElapsedTime;
		}

		// Ray tracing computations. Compute distances till wall within the field of view
		for (int x = 0; x < nScreenWidth; x++) 
		{
			// For each column, calculate the projected ray angle into world space
			float fRayAngle = (fPlayerA - fFOV / 2.0f) + (fFOV / (float)nScreenWidth) * (float)x;

			float fDistanceToWall = 0;
			bool bHitWall = false;

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
					}
				}
			}

			// Calculate distance to ceiling and floor
			int nCeiling = (float)(nScreenHeight / 2.0) - nScreenHeight / ((float)fDistanceToWall);
			int nFloor = nScreenHeight - nCeiling;

			// Paint the screen
			short nShade = ' ';

			if (fDistanceToWall <= fDepth / 4.0f)		nShade = 0x2588;	// Very close
			else if (fDistanceToWall < fDepth / 3.0f)	nShade = 0x2593;
			else if (fDistanceToWall < fDepth / 2.0f)	nShade = 0x2592;
			else if (fDistanceToWall < fDepth)			nShade = 0x2591;
			else										nShade = ' ';		// Too far away

			for (int y = 0; y < nScreenHeight; y++)
			{
				if (y < nCeiling)
					screen[y * nScreenWidth + x] = ' ';
				else if (y > nCeiling && y <= nFloor)
					screen[y * nScreenWidth + x] = nShade;
				else
					screen[y * nScreenWidth + x] = ' ';
			}
		}

		screen[nScreenWidth * nScreenHeight - 1] = '\0';
		LPCSTR screenNow = screen;										// LPCSTR: const char*	  <<<<<<<<<<<<<<<<< PROBLEM: We are forced to use LPCSTR (const char^), so we have to create this every time
		WriteConsoleOutputCharacter(hConsole,							// A handle to console screen buffer
									screenNow,							// Characters to be written to the console screen buffer
									nScreenWidth * nScreenHeight,		// Number of characters to be written
									{ 0,0 },							// COORD structure that specifies the character coordinates of the first cell in the buffer to which characters will be written
									&dwBytesWritten);					// Pointer to a variable that receives the number of characters actually written
	}

}