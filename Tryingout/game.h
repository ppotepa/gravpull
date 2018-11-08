#pragma once
#include "pch.h"
#include <stdio.h>
#include <iostream>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <vector>

using namespace std;
 class object
{
	
	private:			
	protected:
	public:

		virtual int remove()
		{
				return 1;
		}
	
};

class circle: public object
{

private:
protected:
public:
	float x, y, r;
	ALLEGRO_COLOR color;
	circle(float x, float y, float r, ALLEGRO_COLOR color)
	{
	}

};

class game
{
	private:
	protected:
	public:

	vector <object*> objects;

	game()
	{
		
	}

	void DoLogic()
	{
		
	}

	void AddBall(float x, float y, float r, ALLEGRO_COLOR color)
	{
		circle c(x, y, r, color);
		objects.push_back(&c);		
	}	
	
	
};