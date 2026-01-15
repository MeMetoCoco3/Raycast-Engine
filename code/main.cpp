#include <iostream>
#include <raylib.h>
#include <raymath.h>
#include <vector>
#include "vstd/vmath.h"
#include <iostream>
constexpr auto SCR_WIDTH = 600;
constexpr auto SCR_HEIGHT = 600;
constexpr auto FOV = 60;

constexpr auto GRID_SIZE = 64;
constexpr auto DISTANCE_TO_PROJ_PLANE = 277;
constexpr auto N_ROW = SCR_HEIGHT / GRID_SIZE;
constexpr auto N_COLUMNS = SCR_WIDTH / GRID_SIZE;
constexpr auto RECT_COLOR = RAYWHITE;
constexpr auto MAX_RAY_DISTANCE = 800.0f;
int LastIndexMarked = -1;

f32 DDA(Vector2 RayStart, f32 AngleRadians);
void ProcessInput();
void DrawScreen();
void DrawGrid();
void DrawCorners();

static std::vector<bool> Grid;
static bool Running = true;

constexpr auto MOUSE_COLOR = GREEN;
constexpr auto KEYBOARD_COLOR = BLUE;
constexpr auto MOUSE_RADIUS = 20;
constexpr auto PLAYER_SPEED = 200.0f;
constexpr auto RAY_LENGTH = 900.0f;
static Vector2 KeyboardPos = { SCR_WIDTH * 0.5f, SCR_HEIGHT * 0.5f };
static double TimeElapsed = 0;
static Vector2 MousePos = { 0, 0 };

static f32 PlayerDirRadians = 0.0;
static Vector2 RayDir = { 0, 0 };
static Vector2 CollisionPos = { 0, 0 };




int main(void)
{
	InitWindow(SCR_WIDTH, SCR_HEIGHT, "DDA");

	Grid.resize(int(N_ROW * N_COLUMNS), false);
	
	while(Running)
	{
		ProcessInput();
		TimeElapsed = GetFrameTime();

	//	DrawScreen();
		BeginDrawing();
			ClearBackground(GRAY);
			DrawGrid();
			DrawCorners();
			f32 DistanceToCollision = DDA(KeyboardPos, PlayerDirRadians);
			if (DistanceToCollision > 0)
			{
				vec2 Dir = { cos(PlayerDirRadians), -sin(PlayerDirRadians) };
				vec2 CollisionPos = { KeyboardPos.x + Dir.x * DistanceToCollision, KeyboardPos.y + Dir.y * DistanceToCollision };
				DrawCircle(CollisionPos.x, CollisionPos.y, 10, YELLOW);
			}
			/*if (TileFound != -1)
			{
				int Row = TileFound / N_COLUMNS;
				int Column = TileFound % N_COLUMNS;
				DrawRectangle(Column * GRID_SIZE, Row * GRID_SIZE, GRID_SIZE, GRID_SIZE, ORANGE);
				CollisionPos = { Intersection.x * GRID_SIZE, Intersection.y * GRID_SIZE };
				DrawCircle(	CollisionPos.x, CollisionPos.y, MOUSE_RADIUS / 2.0f, YELLOW);


			}*/
			//DrawScreen();
		EndDrawing();
	}
	CloseWindow();
	return 0;
}

f32 DDA(Vector2 RayStart, f32 AngleRadians)
{
	
	AngleRadians = AngleRadians <= 2.0 * PI ? AngleRadians : AngleRadians - 2.0 * PI;
	RayDir = { cos(AngleRadians), -sin(AngleRadians) };
	RayDir.x = RayDir.x == 0 ? 0.00001 : RayDir.x;
	RayDir.y = RayDir.y == 0 ? 0.00001 : RayDir.y;
	

	Vector2 RayUnitStepSize = {
		fabsf(1.0f / RayDir.x),
		fabsf(1.0f / RayDir.y)
	};


	Vector2 MapCheck = {
		(int)((RayStart.x) / GRID_SIZE),
		(int)((RayStart.y) / GRID_SIZE)
	};

	Vector2 RayLength1D;
	Vector2 Step;
	
	Vector2 RayStartGridCoordinates = { 
		((RayStart.x) / GRID_SIZE),
		((RayStart.y) / GRID_SIZE)
	};

	if (RayDir.x < 0)
	{
		Step.x = -1;
		RayLength1D.x = (RayStartGridCoordinates.x - (float)MapCheck.x) * RayUnitStepSize.x;
	}
	else
	{
		Step.x = 1;
		RayLength1D.x = (float(MapCheck.x + 1) - RayStartGridCoordinates.x) * RayUnitStepSize.x;
	}

	if (RayDir.y < 0)
	{
		Step.y = -1;
		RayLength1D.y = (RayStartGridCoordinates.y - (float)MapCheck.y) * RayUnitStepSize.y;
	}
	else
	{
		Step.y = 1;
		RayLength1D.y = (float(MapCheck.y + 1) - RayStartGridCoordinates.y) * RayUnitStepSize.y;
	}

	int TileFound = -1;
	float Distance = 0.0f;
	float MaxDistance = MAX_RAY_DISTANCE;
	while (TileFound == -1 && Distance < MaxDistance)
	{
		if (RayLength1D.x < RayLength1D.y)
		{
			MapCheck.x += Step.x;
			Distance = RayLength1D.x;
			RayLength1D.x += RayUnitStepSize.x;
		}
		else
		{
			MapCheck.y += Step.y;
			Distance = RayLength1D.y;
			RayLength1D.y += RayUnitStepSize.y;
		}

		if (MapCheck.x >= 0 && MapCheck.x < N_COLUMNS &&
			MapCheck.y >= 0 && MapCheck.y < N_ROW)
		{
			int Index = (int)MapCheck.y * N_COLUMNS + (int)MapCheck.x;
			std::cout << Index << std::endl;
			std::cout << MapCheck.x << MapCheck.y << std::endl;
			if (Grid[Index])
			{
				TileFound = Index;
			}
		}
	}
	return Distance != 0.0f ? (Distance * GRID_SIZE) : -1.0f;
}


void DrawScreen()
{
	Vector2 RayStart = KeyboardPos;
	RayStart.x = RayStart.x / GRID_SIZE;
	RayStart.y = RayStart.y / GRID_SIZE;
	f32 STEP_BETWEEN_ANGLES = SCR_WIDTH / (f32)FOV;


	f32 LeftMostAngle = PlayerDirRadians - ((FOV / 2.0f) * STEP_BETWEEN_ANGLES);
	f32 PreComputedValue = GRID_SIZE * DISTANCE_TO_PROJ_PLANE;
	for (int i = 0; i < SCR_WIDTH; i++)
	{
		f32 DistanceToWall = DDA(RayStart, LeftMostAngle + (STEP_BETWEEN_ANGLES * i));
		if (DistanceToWall < 0.0f) continue;
		f32 ProjectedColumnHeight = PreComputedValue / DistanceToWall;
		f32 StartY = (SCR_HEIGHT - ProjectedColumnHeight) / 2;
		DrawLine(i, StartY, i, StartY + ProjectedColumnHeight, RED);
	}
}
void DrawCorners()
{
	DrawCircle(MousePos.x, MousePos.y, MOUSE_RADIUS, MOUSE_COLOR);
	DrawCircle(KeyboardPos.x, KeyboardPos.y, MOUSE_RADIUS, KEYBOARD_COLOR);
//	DrawLine(MousePos.x, MousePos.y, KeyboardPos.x, KeyboardPos.y, RED);
	
	vec2 RayEndPos = { KeyboardPos.x + cosf(PlayerDirRadians) * MAX_RAY_DISTANCE, KeyboardPos.y + -sinf(PlayerDirRadians) * MAX_RAY_DISTANCE };
	DrawLine(KeyboardPos.x, KeyboardPos.y, RayEndPos.x, RayEndPos.y, RED);
	DrawLine(KeyboardPos.x, KeyboardPos.y, CollisionPos.x, CollisionPos.y, RED);
}

void DrawGrid()
{
	for (int i = 0; i < Grid.size(); i++)
	{
		if (Grid[i])
		{
			int Row = i / N_COLUMNS;
			int Column = i % N_COLUMNS;
			DrawRectangle(Column * GRID_SIZE, Row * GRID_SIZE, GRID_SIZE, GRID_SIZE, RECT_COLOR);
		}
	}

	for (int i = 0; i < SCR_WIDTH; i+=GRID_SIZE)
	{
		DrawLine(i, 0, i, SCR_HEIGHT, RED);
		for (int j = 0; j < SCR_HEIGHT; j+=GRID_SIZE)
		{
			DrawLine(0, j, SCR_WIDTH, j, RED);
		}
	}
}

void ProcessInput()
{
	if (IsKeyReleased(KEY_ESCAPE) || WindowShouldClose()) Running = false;
	
	if (IsKeyDown(KEY_W)) {
		KeyboardPos.y -= PLAYER_SPEED * TimeElapsed;
	} else if (IsKeyDown(KEY_S)) {
		KeyboardPos.y += PLAYER_SPEED * TimeElapsed;
	}

	if (IsKeyDown(KEY_A)) {
		KeyboardPos.x -= PLAYER_SPEED * TimeElapsed;
	} else if (IsKeyDown(KEY_D)) {
		KeyboardPos.x += PLAYER_SPEED * TimeElapsed;
	}

	if (IsKeyDown(KEY_K)) {
		PlayerDirRadians += 0.5f * TimeElapsed;
	} else if (IsKeyDown(KEY_L)) {
		PlayerDirRadians -= 0.5f * TimeElapsed;
	}

	
	KeyboardPos.x = Clampf32(KeyboardPos.x, 0, SCR_WIDTH);
	KeyboardPos.y = Clampf32(KeyboardPos.y, 0, SCR_HEIGHT);

	if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) 
	{
		int Row = MousePos.y / GRID_SIZE;
		int Column = MousePos.x / GRID_SIZE;
		int Index = Row * N_COLUMNS + Column;
		if (Index != LastIndexMarked)
		{
			Grid[Index] = !Grid[Index];
			LastIndexMarked = Index;
		}
	}
	if (IsKeyReleased(KEY_C))
	{
		for (int i = 0; i < Grid.size(); i++) Grid[i] = false;
	}
	MousePos = GetMousePosition();
}