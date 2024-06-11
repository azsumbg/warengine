#include "pch.h"
#include "warengine.h"

class HERO :public dll::PERSON
{
	public:	

		HERO(float _x, float _y) :PERSON(creatures::hero, _x, _y)
		{
			lifes = 100;
		}

		void Release() override
		{
			delete this;
		}

		int Move(float _speed, float final_x, float final_y) override
		{
			float my_speed = speed + _speed;

			Path.start_x = x;
			Path.start_y = y;
			Path.dest_x = final_x;
			Path.dest_y = final_y;

			if (Path.dest_x - Path.start_x == 0)
			{
				Path.slope = 0;
				Path.intercept = 0;
				Path.speed_when_vertical = my_speed;

				if (Path.start_y < Path.dest_y)
				{
					dir = dirs::up;
					if (y - my_speed >= 50.0f)
					{
						y -= my_speed;
						SetEdges();
						return DLL_OK;
					}
					else return DLL_FAIL;
				}
				else if (Path.start_y > Path.dest_y)
				{
					dir = dirs::down;
					if (y + my_speed < scr_height - 100.0f)
					{
						y += my_speed;
						SetEdges();
						return DLL_OK;
					}
					else return DLL_FAIL;
				}
			}
			else
			{
				Path.slope = (final_y - Path.start_y) / (final_x - Path.start_x);
				Path.intercept = Path.start_y - Path.slope * Path.start_x;

				if (Path.start_y > Path.dest_y)
				{
					if (Path.start_x > Path.dest_x)
					{
						dir = dirs::u_l;
						if (x - my_speed >= 0) x -= my_speed;
						else return DLL_FAIL;

						if (Path.slope * x + Path.intercept >= 50.0f)
						{
							y = Path.slope * x + Path.intercept;
							SetEdges();
							return DLL_OK;
						}
						else return DLL_FAIL;
					}
					else if (Path.start_x < Path.dest_x)
					{
						dir = dirs::u_r;
						if (x + my_speed <= scr_width) x += my_speed;
						else return DLL_FAIL;

						if (Path.slope * x + Path.intercept >= 50.0f)
						{
							y = Path.slope * x + Path.intercept;
							SetEdges();
							return DLL_OK;
						}
						else return DLL_FAIL;
					}
				}
				else if (Path.start_y < Path.dest_y)
				{
					if (Path.start_x > Path.dest_x)
					{
						dir = dirs::d_l;
						if (x - my_speed >= 0) x -= my_speed;
						else return DLL_FAIL;

						if (Path.slope * x + Path.intercept <= scr_height - 100.0f)
						{
							y = Path.slope * x + Path.intercept;
							SetEdges();
							return DLL_OK;
						}
						else return DLL_FAIL;
					}
					else if (Path.start_x < Path.dest_x)
					{
						dir = dirs::d_r;
						if (x + my_speed <= scr_width) x += my_speed;
						else return DLL_FAIL;

						if (Path.slope * x + Path.intercept <= scr_height - 100.0f)
						{
							y = Path.slope * x + Path.intercept;
							SetEdges();
							return DLL_OK;
						}
						else return DLL_FAIL;
					}
				}
				else if (Path.start_y == Path.dest_y)
				{
					if (Path.start_x > Path.dest_x)
					{
						dir = dirs::left;
						if (x - my_speed >= 0)
						{
							x -= my_speed;
							SetEdges();
							return DLL_OK;
						}
						else return DLL_FAIL;
					}
					else if (Path.start_x < Path.dest_x)
					{
						dir = dirs::right;
						if (x + my_speed <= scr_width)
						{
							x += my_speed;
							SetEdges();
							return DLL_OK;
						}
						else return DLL_FAIL;
					}
				}
			}

			dir = dirs::stop;
			return DLL_FAIL;
		} 
		bool Shoot() override
		{
			shoot_delay--;
			if (shoot_delay < 0)
			{
				shoot_delay = 25;
				return true;
			}
			return false;
		}
		bool Chop() override
		{
			chop_delay--;
			if (chop_delay < 0)
			{
				chop_delay = 25;
				return true;
			}
			return false;
		}
		void AIManager(AI_INPUT Input) override
		{
			AIDataIN = Input;

			if (x >= AIDataIN.near_enemy_x - 50.0f && x <= AIDataIN.near_enemy_x + 50.0f
				&& y >= AIDataIN.near_enemy_y - 50.0f && y <= AIDataIN.near_enemy_y + 50.0f)
			{
				if (lifes < AIDataIN.near_enemy_lifes / 2)
				{
					AIDataOut.new_action = actions::move;
					AIDataOut.new_x = (float)(AIDataIN.shelter.left + 50);
					AIDataOut.new_y = (float)(AIDataIN.shelter.top + 50);
					return;
				}
				else
				{
					AIDataOut.new_action = actions::shoot;
					AIDataOut.new_x = AIDataIN.near_enemy_x;
					AIDataOut.new_y = AIDataIN.near_enemy_y;
					return;
				}
			}
			else if (AIDataIN.base_under_attack)
			{
				if (x >= (float)(AIDataIN.shelter.left) && x <= (float)(AIDataIN.shelter.right)
					&& y >= (float)(AIDataIN.shelter.top + 50) && y <= (float)(AIDataIN.shelter.bottom))
				{
					AIDataOut.new_action = actions::shoot;
					AIDataOut.new_x = AIDataIN.near_enemy_x;
					AIDataOut.new_y = AIDataIN.near_enemy_y;
					return;
				}
				else
				{
					AIDataOut.new_action = actions::move;
					AIDataOut.new_x = (float)(AIDataIN.shelter.left + 50);
					AIDataOut.new_y = (float)(AIDataIN.shelter.top + 50);
					return;
				}
			}
			else if (x >= AIDataIN.near_tree_x - 10.0f && x <= AIDataIN.near_tree_x + 10.0f 
				&& y >= AIDataIN.near_tree_y - 10.0f && y <= AIDataIN.near_tree_y + 10.0f)
	
			{
				AIDataOut.new_action = actions::chop;
				return;
			}
			else
			{
				AIDataOut.new_action = actions::move;
				if (Input.obst_down || Input.obst_up || Input.obst_left || Input.obst_right)
				{
					if (Input.obst_left)AIDataOut.new_x = scr_width;
					if (Input.obst_right)AIDataOut.new_x = 0;
					if (Input.obst_up)AIDataOut.new_y = scr_height - 100.0f;
					if (Input.obst_down)AIDataOut.new_y = 50.0f;

					return;
				}
				else
				{
					AIDataOut.new_x = Input.near_tree_x;
					AIDataOut.new_y = Input.near_tree_y;
				}
			}
		}
};

class EVILS :public dll::PERSON
{
	public:

		EVILS(creatures what, float _x, float _y) :PERSON(what, _x, _y)
		{
			AIDataIN.current_action = actions::move;
		}

		void Release() override
		{
			delete this;
		}
		int Move(float _speed, float final_x, float final_y) override
		{
			float my_speed = speed + _speed;

			Path.start_x = x;
			Path.start_y = y;
			Path.dest_x = final_x;
			Path.dest_y = final_y;

			if (Path.dest_x - Path.start_x == 0)
			{
				Path.slope = 0;
				Path.intercept = 0;
				Path.speed_when_vertical = my_speed;

				if (Path.start_y < Path.dest_y)
				{
					dir = dirs::up;
					if (y - my_speed >= 50.0f)
					{
						y -= my_speed;
						SetEdges();
						return DLL_OK;
					}
					else return DLL_FAIL;
				}
				else
				{
					dir = dirs::down;
					if (y + my_speed <= scr_height - 100.0f)
					{
						y += my_speed;
						SetEdges();
						return DLL_OK;
					}
					else return DLL_FAIL;
				}
			}
			else
			{
				Path.slope = (final_y - Path.start_y) / (final_x - Path.start_x);
				Path.intercept = Path.start_y - Path.slope * Path.start_x;

				if (Path.start_y > Path.dest_y)
				{
					if (Path.start_x > Path.dest_x)
					{
						dir = dirs::u_l;
						x -= my_speed;
						y = Path.slope * x + Path.intercept;
						SetEdges();
						return DLL_OK;
					}
					else
					{
						dir = dirs::u_r;
						x += my_speed;
						y = Path.slope * x + Path.intercept;
						SetEdges();
						return DLL_OK;
					}
				}
				else if (Path.start_y < Path.dest_y)
				{
					if (Path.start_x > Path.dest_x)
					{
						dir = dirs::d_l;
						x -= my_speed;
						y = Path.slope * x + Path.intercept;
						SetEdges();
						return DLL_OK;
					}
					else
					{
						dir = dirs::d_r;
						x += my_speed;
						y = Path.slope * x + Path.intercept;
						SetEdges();
						return DLL_OK;
					}
				}
				else if (Path.start_y == Path.dest_y)
				{
					if (Path.start_x > Path.dest_x)
					{
						dir = dirs::left;
						x -= my_speed;
						SetEdges();
						return DLL_OK;
					}
					else
					{
						dir = dirs::right;
						x += my_speed;
						SetEdges();
						return DLL_OK;
					}
				}
			}

			return DLL_FAIL;
		}
		bool Shoot() override
		{
			shoot_delay--;
			if (shoot_delay < 0)
			{
				shoot_delay = 25;
				return true;
			}
			return false;
		}
		bool Chop() override
		{
			return false;
		}
		void AIManager(AI_INPUT Input)
		{
			AIDataIN = Input;

			if (AIDataIN.obst_down)
			{
				AIDataOut.new_action = actions::move;
				AIDataOut.new_x = AIDataIN.near_enemy_x;
				AIDataOut.new_y = 50.0f;
				AIDataIN.obst_down = false;
				return;
			}
			else if (AIDataIN.obst_up)
			{
				AIDataOut.new_action = actions::move;
				AIDataOut.new_x = AIDataIN.near_enemy_x;
				AIDataOut.new_y = scr_height;
				AIDataIN.obst_up = false;
				return;
			}
			else if (AIDataIN.obst_left)
			{
				AIDataOut.new_action = actions::move;
				AIDataOut.new_x = scr_width;
				AIDataOut.new_y = AIDataIN.near_enemy_y;
				AIDataIN.obst_left = false;
				return;
			}
			else if (AIDataIN.obst_right)
			{
				AIDataOut.new_action = actions::move;
				AIDataOut.new_x = 0;
				AIDataOut.new_y = AIDataIN.near_enemy_y;
				AIDataIN.obst_right = false;
				return;
			}

			if (x >= (float)(AIDataIN.shelter.left) && x <= (float)(AIDataIN.shelter.right) &&
				y >= (float)(AIDataIN.shelter.top) && y <= (float)(AIDataIN.shelter.bottom))
			{
				AIDataOut.new_action = actions::shoot;
				AIDataOut.new_x = (float)(AIDataIN.shelter.left + 50);
				AIDataOut.new_y = (float)(AIDataIN.shelter.top);
				return;
			}
			else if (x >= AIDataIN.near_enemy_x - 50.0f && x <= AIDataIN.near_enemy_x + 50.0f &&
				y >= AIDataIN.near_enemy_y - 50.0f && y <= AIDataIN.near_enemy_y + 50.0f)
			{
				AIDataOut.new_action = actions::shoot;
				AIDataOut.new_x = AIDataIN.near_enemy_x;
				AIDataOut.new_y = AIDataIN.near_enemy_y;
				return;
			}

			if ((float)(abs(x - AIDataIN.near_enemy_x)) <= 100 && (float)(abs(y - AIDataIN.near_enemy_y)) <= 100)
			{
				AIDataOut.new_action = actions::move;
				AIDataOut.new_x = AIDataIN.near_enemy_x;
				AIDataOut.new_y = AIDataIN.near_enemy_y;
				return;
			}

			AIDataOut.new_action = actions::move;
			AIDataOut.new_x = AIDataIN.shelter.left + 50.0f;
			AIDataOut.new_y = AIDataIN.shelter.top + 50.0f;
		}
};

//FUNCTIONS **********************

void dll::SetPath(PATH& newPath)
{
	if (newPath.dest_x - newPath.start_x == 0)
	{
		newPath.intercept = 0;
		newPath.slope = 0;
		return;
	}

	newPath.slope = (newPath.dest_y - newPath.start_y) / (newPath.dest_x - newPath.start_x);
	newPath.intercept = newPath.start_y - newPath.slope * newPath.start_x;
}
float dll::GetNextYFromPath(float next_x, PATH PathData)
{
	if (PathData.slope == 0 && PathData.intercept == 0)
	{
		if (PathData.dest_y < PathData.start_y)return PathData.start_y - PathData.speed_when_vertical;
		else return PathData.start_y - PathData.speed_when_vertical;
	}
	return PathData.slope * next_x + PathData.intercept;
}
void dll::SetTargetY(float angle, dirs move_dir, PATH& PathData)
{
	float rad_angle = angle * (3.14f / 180.0f);
	float known_cathet = 0;
	float unknown_cathet = 0;

	if (move_dir == dirs::up)
	{
		PathData.dest_x = PathData.start_x;
		PathData.dest_y = 50.0f;
	}
	else if(move_dir == dirs::down)
	{
		PathData.dest_x = PathData.start_x;
		PathData.dest_y = scr_height - 50.0f;
	}
	else if (move_dir == dirs::left || move_dir == dirs::u_l || move_dir == dirs::d_l)
	{
		PathData.dest_x = 0;
		if (move_dir == dirs::left)PathData.dest_y = PathData.start_y;
		known_cathet = PathData.start_x;
	}
	else if(move_dir == dirs::right || move_dir == dirs::u_r || move_dir == dirs::d_r)
	{
		PathData.dest_x = scr_width;
		if (move_dir == dirs::left)PathData.dest_y = PathData.start_y;
		known_cathet = PathData.dest_x - PathData.start_x;
	}

	if (move_dir == dirs::u_l || move_dir == dirs::u_r)
	{
		unknown_cathet = static_cast<float>(tan(rad_angle) * known_cathet);
		PathData.dest_y = PathData.start_y - unknown_cathet;
	}
	else if (move_dir == dirs::d_l || move_dir == dirs::d_r)
	{
		unknown_cathet = static_cast<float>(tan(rad_angle) * known_cathet);
		PathData.dest_y = PathData.start_y + unknown_cathet;
	}

	if(PathData.dest_x - PathData.start_x == 0)
	{
		PathData.intercept = 0;
		PathData.slope = 0;
		return;
	}

	PathData.slope = (PathData.dest_y - PathData.start_y) / (PathData.dest_x - PathData.start_x);
	PathData.intercept = PathData.start_y - PathData.slope * PathData.start_x;
}

/////////////////////////////////////

//FACTORIES ***********************

dll::Tile dll::BUILDING::TileFactory(buildings what, float start_x, float start_y)
{
	Tile ret = nullptr;

	ret = new BUILDING(what, start_x, start_y);

	return ret;
}
dll::Creature dll::CreatureFactory(creatures who, float start_x, float start_y)
{
	Creature ret = nullptr;

	switch (who)
	{
	case creatures::hero:
		ret = new HERO(start_x, start_y);
		break;

	case creatures::evil1:
		ret = new EVILS(creatures::evil1, start_x, start_y);
		break;

	case creatures::evil2:
		ret = new EVILS(creatures::evil2, start_x, start_y);
		break;

	case creatures::evil3:
		ret = new EVILS(creatures::evil3, start_x, start_y);
		break;

	case creatures::bear:
		ret = new EVILS(creatures::bear, start_x, start_y);
		break;
	}
	return ret;
}