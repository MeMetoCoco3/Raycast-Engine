#include <iostream>
#include <raylib.h>
#include <raymath.h>
#include <vector>
#include <array>
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

struct s_Collision_Info {
	vec2 PointCollision;
	f32 UVXValue;
	f32 DistanceToCollision;
	i32 TextureIndex;
};


s_Collision_Info DDA(Vector2 RayStart, f32 AngleRadians);
void ProcessInput();
void DrawScreen();
void DrawGrid();
void DrawCorners();

static std::vector<i32> Grid;
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

enum e_Mode {
	LEVEL_EDIT = 0,
	RAYCAST
};

enum e_Texture : i32{
	T_NONE,
	T_BOOKSHELF,
	T_DIRT,
	T_LAVA,
	T_COUNT
};


struct s_Game {
	e_Mode Mode;
} Game;

struct s_Context  {
	std::vector<Image> Images;
	std::array<Color, T_COUNT> Colors{ { RAYWHITE, GREEN, BROWN, RED} };
} Context;

int main(void)
{
	InitWindow(SCR_WIDTH, SCR_HEIGHT, "DDA");

	Grid.resize(int(N_ROW * N_COLUMNS));
	
	Context.Images.push_back(LoadImage( RESOURCES_PATH "bookshelf.png"));
	Context.Images.push_back(LoadImage( RESOURCES_PATH "Dirt.png"));
	Context.Images.push_back(LoadImage( RESOURCES_PATH "LavaF1.png"));


	while(Running)
	{
		ProcessInput();
		TimeElapsed = GetFrameTime();

		BeginDrawing();
		
			ClearBackground(GRAY);
			switch (Game.Mode == LEVEL_EDIT) {
			case LEVEL_EDIT: {
				DrawGrid();
				DrawCorners();
			} break;
			case RAYCAST: {
				DrawScreen();
			} break;
			}
		EndDrawing();
	}
	CloseWindow();
	return 0;
}

s_Collision_Info DDA(Vector2 RayStart, f32 AngleRadians)
{
	s_Collision_Info Result;
	Result.DistanceToCollision = -1.0f;

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
			if (Grid[Index] != e_Texture::T_NONE)
			{
				TileFound = Index;
				Result.DistanceToCollision = (Distance * GRID_SIZE);
				Vector2 PointCollision =  RayStart + (RayDir * Result.DistanceToCollision);
				Result.PointCollision = { PointCollision.x, PointCollision.y };
				Result.TextureIndex = Grid[Index];
				if (((int)PointCollision.x % GRID_SIZE) == 0)
					Result.UVXValue = (int)PointCollision.y % GRID_SIZE;
				else 
				{
					Result.UVXValue = (int)PointCollision.x % GRID_SIZE;
				}
			}
		}
	}
	return Result;
}


void DrawScreen()
{
	Vector2 RayStart = KeyboardPos;

	f32 STEP_BETWEEN_ANGLES = TO_RADIAN(FOV) / SCR_WIDTH;

	f32 LeftMostAngle = PlayerDirRadians - (TO_RADIAN(FOV) * 0.5f);
	f32 PreComputedValue = GRID_SIZE * DISTANCE_TO_PROJ_PLANE;
	for (int i = 0; i < SCR_WIDTH; i++)
	{
		f32 CurrentAngle = LeftMostAngle + (STEP_BETWEEN_ANGLES * i);
		s_Collision_Info ColInfo = DDA(RayStart, CurrentAngle);

		if (ColInfo.DistanceToCollision < 0.0f) continue;
		f32 CorrectedDistance = ColInfo.DistanceToCollision * cos(CurrentAngle - PlayerDirRadians);
		f32 WallHeight = (GRID_SIZE / CorrectedDistance) * DISTANCE_TO_PROJ_PLANE;
	
		i32 TextureIndex = 1 + ColInfo.TextureIndex;
		f32 StartY = (SCR_HEIGHT - WallHeight) / 2;
		f32 TextureY = 0.0f;
		f32 TextureYStep = GRID_SIZE / WallHeight;
		int TextureWidth = Context.Images[TextureIndex].width;
		for (int y = StartY; y <= StartY + WallHeight; y++) 
		{
			int PixelIndex = (int)TextureY * TextureWidth + ColInfo.UVXValue;
			u8* C = ((u8*)Context.Images[TextureIndex].data) + PixelIndex * 4;
			Color PixelColor = { C[0], C[1], C[2], C[3]};
			DrawPixel(i, y, PixelColor);
			TextureY += TextureYStep;
		}
	}
}

void DrawCorners()
{
	DrawCircle(MousePos.x, MousePos.y, MOUSE_RADIUS, MOUSE_COLOR);
	DrawCircle(KeyboardPos.x, KeyboardPos.y, MOUSE_RADIUS, KEYBOARD_COLOR);
//	DrawLine(MousePos.x, MousePos.y, KeyboardPos.x, KeyboardPos.y, RED);
	
}

void DrawGrid()
{
	for (int i = 0; i < Grid.size(); i++)
	{
		if (Grid[i] != e_Texture::T_COUNT)
		{
			int Row = i / N_COLUMNS;
			int Column = i % N_COLUMNS;
			DrawRectangle(Column * GRID_SIZE, Row * GRID_SIZE, GRID_SIZE, GRID_SIZE, Context.Colors[Grid[i]]);
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
		Vector2 Forward = { cos(PlayerDirRadians), -sin(PlayerDirRadians)};
		KeyboardPos += Forward * PLAYER_SPEED * TimeElapsed;
	} else if (IsKeyDown(KEY_S)) {
		Vector2 Backwards = { -cos(PlayerDirRadians), sin(PlayerDirRadians)};
		KeyboardPos += Backwards * PLAYER_SPEED * TimeElapsed;
	}

	if (IsKeyDown(KEY_A)) {
		Vector2 Left = { cos(PlayerDirRadians-HALF_PI), -sin(PlayerDirRadians-HALF_PI)};
		KeyboardPos += Left * PLAYER_SPEED * TimeElapsed;
	} else if (IsKeyDown(KEY_D)) {
		Vector2 Right = { cos(PlayerDirRadians+HALF_PI), -sin(PlayerDirRadians+HALF_PI)};
		KeyboardPos += Right * PLAYER_SPEED * TimeElapsed;
	}

	if (IsKeyDown(KEY_L)) {
		PlayerDirRadians += 0.5f * TimeElapsed;
		if (PlayerDirRadians > 2.0f * PI) PlayerDirRadians -= 2.0f * PI;
	} else if (IsKeyDown(KEY_K)) {
		PlayerDirRadians -= 0.5f * TimeElapsed;
		if (PlayerDirRadians < 0.0f) PlayerDirRadians += +2.0f * PI;
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
			++Grid[Index];
			LastIndexMarked = Index;
			if (Grid[Index] >= e_Texture::T_COUNT) Grid[Index] = e_Texture::T_NONE;
		}
	}
	if (IsKeyReleased(KEY_P))
	{
		if (Game.Mode == LEVEL_EDIT) Game.Mode = RAYCAST;
		else if (Game.Mode == RAYCAST) Game.Mode = LEVEL_EDIT;
	}

	if (IsKeyReleased(KEY_C))
	{
		for (int i = 0; i < Grid.size(); i++) Grid[i] = e_Texture::T_NONE;
	}
	MousePos = GetMousePosition();
}