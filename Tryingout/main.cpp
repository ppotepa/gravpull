#include "pch.h"
#include <stdio.h>
#include <iostream>
#include <algorithm>   
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_primitives.h>
#include <vector>
#include <omp.h>

using namespace std;

struct v2d
{
	float x, y;
};
ALLEGRO_COLOR red = al_map_rgb(255, 0, 0);

class ball
{
private:

public:

	v2d pos;
	v2d mov;
	ALLEGRO_COLOR color;
	char cr, cg, cb;
	float r;
	bool sun;

	void beSun()
	{
		sun = true;
	}

	ball(float x, float y, float vx, float vy, float r, ALLEGRO_COLOR color)
	{
		this->pos.x = x;
		this->pos.y = y;
		this->mov.x = vx;
		this->mov.y = vy;
		//this->color = red;
		this->r = r;
		this->sun = false;
		//print();
	}

	void move()
	{
		//if (!this->sun)
		{
			this->pos.x += this->mov.x;
			this->pos.y += this->mov.y;
		}
	}

	void print()
	{
		std::cout << "x:" << this->pos.x << endl;
		std::cout << "y:" << this->pos.y << endl;
		std::cout << "vx:" << this->mov.x << endl;
		std::cout << "vy:" << this->mov.y << endl;
		std::cout << "col:" << this->color.r << endl;
	}
};

vector <ball> balls;
float scale = 10000;

bool compare(ball i, ball j) { return (i.r < j.r); };


int calc = 0;

int pull(ball b, int id)

{
	int i = 0;
	float nx, ny=0, d;		
	{		
		#pragma omp for
		for (i = 0; i < balls.size(); i++)		
		{
			if (id != i)
			{
				bool erase = false;
				
				{
					nx = balls[i].pos.x - b.pos.x;
					ny = balls[i].pos.y - b.pos.y;

					d = (nx*nx) + (ny*ny);
					d = sqrt(d);
					d = abs(d);

					nx /= 10000000000;
					ny /= 10000000000;

					nx *= (b.r*b.r)*(100 / d);
					ny *= (b.r*b.r)*(100 / d);

					balls[i].mov.x += -nx / 10;
					balls[i].mov.y += -ny / 10;
					
				}
				if (d < 1000)
				{
					if (b.r > balls[i].r)
					{

					}
					else
					{
						balls[i].r += b.r;
						balls.erase(balls.begin() + id);
					}
				}
			}
		}
	}
	return calc;
}

void genballs(int count)
{
	ball * tmp;

	for (int i = 0; i < count; i++)
	{
		tmp = new ball((rand() % 1920)*scale, (rand() % 1080)* scale, 500 - rand() % 1000, 500 - rand() % 1000, 500+ rand() % 50000, red);
		balls.push_back(*tmp);
	}

	tmp = new ball((1920*scale)/2, (1080 * scale) / 2, 0, 0, 500000, al_map_rgb(255, 0, 0));
	tmp->beSun();
	//balls.push_back(*tmp);

	delete tmp;
}

using namespace std;
const float FPS = 60;

int yp = 0;
int main(int argc, char *argv[])
{
	omp_set_num_threads(100);
	int c = 1000;
	srand(time(NULL));
	genballs(c);
	int cb = 0;	
	{		
		for (int i = 0; i < 1; i++)
		{
			std::sort(balls.begin(), balls.end(), compare);			
			for (int i = 0; i < balls.size(); i++) {
				balls[i].move();
				
				{
					pull(balls[i], i);
				}
			}
			int cx = balls[balls.size() - 1].pos.x;
			int cy = balls[balls.size() - 1].pos.y;

			float mainx = balls[balls.size() - 1].pos.x;
			float mainy = balls[balls.size() - 1].pos.y;
		}
	}
	ALLEGRO_DISPLAY *display = NULL;
	ALLEGRO_EVENT_QUEUE *event_queue = NULL;
	ALLEGRO_TIMER *timer = NULL;
	ALLEGRO_BITMAP * buffer;

	bool running = true;
	bool redraw = true;


	// Initialize allegro
	if (!al_init()) {
		fprintf(stderr, "Failed to initialize allegro.\n");
		return 1;
	}

	// Initialize the timer
	timer = al_create_timer(1.0 / FPS);
	if (!timer) {
		fprintf(stderr, "Failed to create timer.\n");
		return 1;
	}


	// Create the display
	display = al_create_display(1920, 1080);
	if (!display) {
		fprintf(stderr, "Failed to create display.\n");
		return 1;
	}

	// Create the event queue
	event_queue = al_create_event_queue();
	if (!event_queue) {
		fprintf(stderr, "Failed to create event queue.");
		return 1;
	}

	// Register event sources
	al_register_event_source(event_queue, al_get_display_event_source(display));
	al_register_event_source(event_queue, al_get_timer_event_source(timer));

	// Display a black screen

	al_init_primitives_addon();
	buffer = al_create_bitmap(1920, 1080);

	//al_set_target_bitmap(buffer);
	al_clear_to_color(al_map_rgb(0, 0, 0));

	al_flip_display();

	// Start the timer
	al_start_timer(timer);

	

	// Game loop

	while (running) {
		ALLEGRO_EVENT event;
		ALLEGRO_TIMEOUT timeout;

		// Initialize timeout
		al_init_timeout(&timeout, 0.06);

		// Fetch the event (if one exists)
		bool get_event = al_wait_for_event_until(event_queue, &event, &timeout);

		// Handle the event
		if (get_event) {
			switch (event.type) {
			case ALLEGRO_EVENT_TIMER:
				redraw = true;
				break;
			case ALLEGRO_EVENT_DISPLAY_CLOSE:
				running = false;
				break;
			default:
				fprintf(stderr, "Unsupported event received: %d\n", event.type);
				break;
			}
		}
		
			std::sort(balls.begin(), balls.end(), compare);
			for (int i = 0; i < balls.size(); i++) {
				balls[i].move();
				pull(balls[i], i);
			}
			int cx = balls[balls.size() - 1].pos.x;
			int cy = balls[balls.size() - 1].pos.y;

			float mainx = balls[balls.size() - 1].pos.x;
			float mainy = balls[balls.size() - 1].pos.y;
		
		
		if (redraw && al_is_event_queue_empty(event_queue)) {
			int primes = 0;
			al_clear_to_color(al_map_rgb(0, 0, 0));
			
			for (int i = 0; i < balls.size(); i++) {
				primes = 0;
				//if (i < balls.size()-1)al_draw_line(balls[i].pos.x, balls[i].pos.y, balls[i + 1].pos.x, balls[i + 1].pos.y, al_map_rgb(127, 127, 127), 12);
				
				if (balls[i].sun == false)al_draw_filled_circle(((balls[i].pos.x) / scale), balls[i].pos.y / scale, balls[i].r / scale, al_map_rgb(255, 255, 255));
				
				else
				{
					al_draw_filled_circle(((balls[i].pos.x) / scale), (balls[i].pos.y / scale), balls[i].r / scale, al_map_rgb(0, 0, 255));
				}
				al_set_target_bitmap(buffer);
				al_draw_pixel(balls[i].pos.x / scale, balls[i].pos.y, al_map_rgb(255, 255, 255));
				al_set_target_bitmap(al_get_backbuffer(display));
			
			}
		


			al_draw_filled_circle(balls[balls.size() - 1].pos.x / scale, balls[balls.size() - 1].pos.y / scale, balls[balls.size() - 1].r / scale, al_map_rgb(255, 0, 0));
			al_draw_bitmap(buffer, 0, 0, 0);
			al_flip_display();
			redraw = false;
		}
	}

	// Clean up
	al_destroy_display(display);
	al_destroy_event_queue(event_queue);

	return 0;
}