#pragma once

#ifdef WARENGINE_EXPORTS
#define WARAPI _declspec(dllexport)
#else
#define WARAPI _declspec(dllimport)
#endif

#include <cmath>

constexpr float scr_width = 1000.0f;
constexpr float scr_height = 750.0f;

constexpr int DLL_OK = 22000;
constexpr int DLL_FAIL = 22001;
constexpr int DLL_NaN = 22002;

enum class dirs { up = 0, down = 1, left = 2, right = 3, u_r = 4, u_l = 5, d_r = 6, d_l = 7, stop = 8 };
enum class creatures { evil1 = 0, evil2 = 1, evil3 = 2, bear = 3, hero = 4 };
enum class buildings { snow_tile = 0, soil_tile = 1, home = 2, wall = 3, tree = 4, tower = 5, townhall = 6, fire = 7 };

enum class actions { move = 0, shoot = 1, chop = 2, stop = 3, shelter = 4 };

struct WARAPI PATH
{
	float start_x = 0;
	float start_y = 0;
	float dest_x = 0;
	float dest_y = 0;
	
	float slope = 0;
	float intercept = 0;
	float speed_when_vertical = 0;
};

struct WARAPI AI_INPUT
{
	actions current_action = actions::stop;

	RECT shelter = { 0 };
	
	float near_enemy_x = 0;
	float near_enemy_y = 0;
	int near_enemy_lifes = 0;

	float near_tree_x = 0;
	float near_tree_y = 0;

	bool obst_left = false;
	bool obst_right = false;
	bool obst_up = false;
	bool obst_down = false;

	bool tree_in_range = false;
	bool base_under_attack = false;
};

struct WARAPI AI_OUTPUT
{
	float new_x = 0;
	float new_y = 0;
	actions new_action = actions::stop;
};

namespace dll
{
	class WARAPI OBJECT
	{
		protected:
			float width = 0;
			float height = 0;

		public:
			float x = 0;
			float y = 0;
			float ex = 0;
			float ey = 0;

			OBJECT(float _x, float _y, float _width = 1.0f, float _height = 1.0f)
			{
				x = _x;
				y = _y;
				width = _width;
				height = _height;
				ex = x + width;
				ey = y + height;
			}
			virtual ~OBJECT() {};
			int ReleaseObj(OBJECT** anObject)
			{
				if ((*anObject))
				{
					delete (*anObject);
					anObject = nullptr;
					return DLL_OK;
				}
				return DLL_NaN;
			}

			float GetWidth() const
			{
				return width;
			}
			float GetHeight() const
			{
				return height;
			}

			void SetWidth(float new_width)
			{
				width = new_width;
				ex = x + width;
			}
			void SetHeight(float new_height)
			{
				height = new_height;
				ey = y + height;
			}
			void SetEdges()
			{
				ex = x + width;
				ey = y + height;
			}
			void NewDims(float new_width, float new_height)
			{
				width = new_width;
				height = new_height;
				ex = x + width;
				ey = y + height;
			}
	};

	class WARAPI PERSON :public OBJECT
	{
		protected:
			float speed = 1.0f;
			int shoot_delay = 25;
			int chop_delay = 25;
			PATH Path;
			
		public:
			int lifes = 0;
			creatures type = creatures::hero;
			dirs dir = dirs::right;
			AI_OUTPUT AIDataOut;
			AI_INPUT AIDataIN;

			PERSON(creatures who, float _x, float _y) :OBJECT(_x, _y)
			{
				x = _x;
				y = _y;
				type = who;

				switch (type)
				{
				case creatures::hero:
					NewDims(27.0f, 40.0f);
					lifes = 50;
					break;

				case creatures::evil1:
					NewDims(80.0f, 60.0f);
					lifes = 100;
					speed = 0.8f;
					break;

				case creatures::evil2:
					NewDims(80.0f, 80.0f);
					lifes = 120;
					speed = 0.6f;
					break;

				case creatures::evil3:
					NewDims(70.0f, 78.0f);
					lifes = 110;
					speed = 0.9f;
					break;

				case creatures::bear:
					NewDims(100.0f, 100.0f);
					lifes = 150;
					speed = 0.5f;
					break;

				}
			}
			virtual ~PERSON(){}

			void GetPath(PATH& MyPath)const
			{
				MyPath = Path;
			}

			virtual void Release() = 0;
			virtual int Move(float _speed, float final_x, float final_y) = 0;
			virtual bool Shoot() = 0;
			virtual bool Chop() = 0;
			virtual void AIManager(AI_INPUT Input) = 0;
	};

	class WARAPI BUILDING :public OBJECT
	{
		private:
			int frame = -1;
		
			BUILDING(buildings what, float _x, float _y) :OBJECT(_x, _y)
			{
				type = what;
				x = _x;
				y = _y;

				switch (type)
				{
				case buildings::snow_tile:
					NewDims(50.0f, 50.0f);
					break;

				case buildings::soil_tile:
					NewDims(50.0f, 50.0f);
					break;

				case buildings::home:
					NewDims(50.0f, 32.0f);
					lifes = 200;
					break;

				case buildings::tower:
					NewDims(25.0f, 50.0f);
					lifes = 80;
					break;

				case buildings::townhall:
					NewDims(150.0f, 140.0f);
					lifes = 500;
					break;

				case buildings::wall:
					NewDims(50.0f, 50.0f);
					lifes = 100;
					break;

				case buildings::tree:
					NewDims(47.0f, 50.0f);
					lifes = 100;
					break;
				}
			}

		public:
			int lifes = 0;
			buildings type;

			static BUILDING* TileFactory(buildings what, float start_x, float start_y);

			virtual ~BUILDING(){}

			int GetFrame()
			{
				frame++;
				if (frame > 15)return DLL_FAIL;
				return frame;
			}

			void Release()
			{
				delete this;
			}
			int Transform(buildings what)
			{
				switch (what)
				{
				case buildings::snow_tile:
					if (type != buildings::fire && type != buildings::soil_tile && type != buildings::tree)
						return DLL_FAIL;
					else type = what;
					break;

				case buildings::soil_tile:
					if (type != buildings::fire && type != buildings::snow_tile && type != buildings::tree)
						return DLL_FAIL;
					else type = what;
					break;

				case buildings::home:
					if (type != buildings::soil_tile)
						return DLL_FAIL;
					else type = what;
					break;

				case buildings::townhall:
					if (type != buildings::snow_tile && type != buildings::soil_tile)
						return DLL_FAIL;
					else type = what;
					break;

				case buildings::tower:
					if (type != buildings::snow_tile && type != buildings::soil_tile)
						return DLL_FAIL;
					else type = what;
					break;

				case buildings::wall:
					if (type != buildings::snow_tile && type != buildings::soil_tile)
						return DLL_FAIL;
					else type = what;
					break;

				case buildings::fire:
					if (type != buildings::home && type != buildings::townhall && type != buildings::tower)
						return DLL_FAIL;
					else type = what;
					break;

				case buildings::tree:
					if (type != buildings::snow_tile && type != buildings::soil_tile)
						return DLL_FAIL;
					else type = what;
					break;
				}
				return DLL_OK;
			}
	};

	typedef PERSON* Creature;
	typedef BUILDING* Tile;

	//FUNCTIONS **********************

	extern WARAPI void SetPath(PATH& newPath);
	extern WARAPI float GetNextYFromPath(float next_x, PATH PathData);

	//Set dest_x and dest_y using tangens of input angle, then set slope and intercept. 
	// !!! start_x and start_y must be set !!! 
	
	extern WARAPI void SetTargetY(float angle, dirs move_dir, PATH& PathData);

	extern WARAPI Creature CreatureFactory(creatures who, float start_x, float start_y);
}