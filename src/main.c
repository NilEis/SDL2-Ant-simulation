#if !defined(MP) && !defined(OPT) && !defined(__EMSCRIPTEN__)
#define MP 1
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <math.h>

#ifndef WIDTH
#define WIDTH (int)(320)
#endif

#ifndef HEIGHT
#define HEIGHT (int)(180)
#endif

#ifndef NUM_AGENTS
#define NUM_AGENTS (int)((WIDTH * HEIGHT) * 0.15)
#endif

#define PI2 1.570796326794896

#define SA (0.3926991 * 2.5)
#define RA (0.7853982 * 1)

#define XY_TO_I(y, x) (((y)*WIDTH + (x)) % (WIDTH * HEIGHT))

#define x_func cos

#define y_func sin

typedef struct
{
	float x;
	float y;
	float a;
} agent_t;

typedef double cell_t;

agent_t agents[NUM_AGENTS];
cell_t buffer_a[HEIGHT * WIDTH] = {0};
cell_t buffer_b[HEIGHT * WIDTH] = {0};

cell_t *map_a = buffer_a;
cell_t *map_b = buffer_b;

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Surface *surface;

void draw();
void update_agents();
void update_map();

double lerp(double a, double b, double t);

int main(int argc, char **argv)
{
	srand(clock());
	memset(agents, 0, NUM_AGENTS);
#if MP
#pragma omp parallel for shared(agents) num_threads(8)
#endif
	for (int i = 0; i < NUM_AGENTS; i++)
	{
		/* const static float R = HEIGHT / 2 - 50;
		const float r = R * sqrt((double)(rand() % 1000) / 1000.0);
		const float theta = ((float)(rand() % 10000) / 10000.0) * 4.0 * PI2;
		agents[i].x = (WIDTH / 2) + r * x_func(theta);
		agents[i].y = (HEIGHT / 2) + r * y_func(theta);
		agents[i].a = atan2(-y_func(theta), -x_func(theta)); */
		agents[i].x = WIDTH / 2;
		agents[i].y = HEIGHT / 2;
		agents[i].a = i;
	}
	SDL_Init(SDL_INIT_VIDEO);
	SDL_CreateWindowAndRenderer(WIDTH, HEIGHT, SDL_WINDOW_FULLSCREEN_DESKTOP, &window, &renderer);
	surface = SDL_CreateRGBSurface(0, WIDTH, HEIGHT, 32, 0, 0, 0, 0);
#if MP
	static const char title[] = "Game of life openMP: ON";
#else
	static const char title[] = "Game of life";
#endif
	SDL_SetWindowTitle(window, title);
#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop(draw, 0, 1);
#else
	while (1)
	{
		draw();
		SDL_Event event;
		if (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				break;
			}
		}
		SDL_Delay(30);
	}
#endif
	if (0)
	{
		FILE *log_f = fopen("log.txt", "w");
		for (int y = 0; y < HEIGHT; y++)
		{
			for (int x = 0; x < WIDTH; x++)
			{
				fprintf(log_f, "%f, ", map_a[XY_TO_I(y, x)]);
			}
			fputc('\n', log_f);
		}
	}
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void draw()
{
	for (int st = 0; st < 1; st++)
	{
		update_agents();
		update_map();
		cell_t *tmp = map_a;
		map_a = map_b;
		map_b = tmp;
	}
}

void update_agents()
{
#if MP
#pragma omp parallel for num_threads(8)
#endif
	for (int i = 0; i < NUM_AGENTS; i++)
	{
		int x = agents[i].x;
		int y = agents[i].y;
		double dx = x_func(agents[i].a);
		double dy = y_func(agents[i].a);
		for (int j = 0; j < 1; j++)
		{
			if (x + dx < 2 || x + dx > WIDTH - 2 || y + dy < 2 || y + dy > HEIGHT - 2)
			{
				agents[i].a = atan2(-dx, -dy);
			}
			else
			{
				agents[i].x += x_func(agents[i].a);
				agents[i].y += y_func(agents[i].a);
			}
			x = agents[i].x;
			y = agents[i].y;
			map_a[XY_TO_I(y, x)] = 1;
		}
	}

#if MP
#pragma omp barrier
#endif
#if MP
#pragma omp parallel for num_threads(8)
#endif
	for (int i = 0; i < NUM_AGENTS; i++)
	{
		int x = agents[i].x;
		int y = agents[i].y;
		float ad_a = agents[i].a + SA;
		float su_a = agents[i].a - SA;
		float ad = 0;
		float su = 0;
		float f = 0;
		for (int d = 9; d > 8; d--)
		{
			ad += map_a[XY_TO_I((int)(y + d * y_func(ad_a)), (int)(x + d * x_func(ad_a)))];
			su += map_a[XY_TO_I((int)(y + d * y_func(su_a)), (int)(x + d * x_func(su_a)))];
			f += map_a[XY_TO_I((int)(y + d * y_func(agents[i].a)), (int)(x + d * x_func(agents[i].a)))];
		}
		if ((f > su && f > ad) || (f == su && f == ad))
		{
			continue;
		}
		else if (f < su && f < ad)
		{
			int r = rand() % 100;
			agents[i].a += (r < 50) * (((((rand() % 100) / 100.0)) + 0.5) * RA) - ((r >= 50) * (((((rand() % 100) / 100.0)) + 0.5) * RA));
		}
		else if (su < ad)
		{
			agents[i].a += ((((rand() % 100) / 100.0)) + 0.5) * RA;
		}
		else if (su > ad)
		{
			agents[i].a -= ((((rand() % 100) / 100.0)) + 0.5) * RA;
		}
	}
}

void update_map()
{
	const static float s = 0.014;
#if MP
#pragma omp parallel for collapse(2) num_threads(8)
#endif
	for (int y = 1; y < HEIGHT - 1; y++)
	{
		for (int x = 1; x < WIDTH - 1; x++)
		{
			map_a[XY_TO_I(y, x)] = ((map_a[XY_TO_I(y, x)] - s) >= 0 && (map_a[XY_TO_I(y, x)] - s) < 1) * (map_a[XY_TO_I(y, x)] - s);
		}
	}
	if (SDL_MUSTLOCK(surface))
		SDL_LockSurface(surface);

	uint8_t *pixels = surface->pixels;
#if MP
#pragma omp parallel for collapse(2) num_threads(8)
#endif
	for (int y = 1; y < HEIGHT - 1; y++)
	{
		for (int x = 1; x < WIDTH - 1; x++)
		{
			cell_t current =
				(1 * map_a[XY_TO_I(y + 1, x + 1)] +
				 1 * map_a[XY_TO_I(y + 1, x * 1)] +
				 1 * map_a[XY_TO_I(y + 1, x - 1)] +
				 1 * map_a[XY_TO_I(y * 1, x + 1)] +
				 1 * map_a[XY_TO_I(y * 1, x * 1)] +
				 1 * map_a[XY_TO_I(y * 1, x - 1)] +
				 1 * map_a[XY_TO_I(y - 1, x + 1)] +
				 1 * map_a[XY_TO_I(y - 1, x * 1)] +
				 1 * map_a[XY_TO_I(y - 1, x - 1)]) *
				(1.0 / 9.0);
			map_b[XY_TO_I(y, x)] = lerp(map_a[XY_TO_I(y * 1, x * 1)], current, 0.3);
			/*pixels[(WIDTH * 4 * y + x * 4)] = map_b[XY_TO_I(y, x)] * 255;
			pixels[(WIDTH * 4 * y + x * 4) + 1] = map_b[XY_TO_I(y, x)] * 255;
			pixels[(WIDTH * 4 * y + x * 4) + 2] = map_b[XY_TO_I(y, x)] * 255;
			pixels[(WIDTH * 4 * y + x * 4) + 3] = -1;*/
			uint8_t c = 255 * map_b[XY_TO_I(y, x)];
			uint32_t *p = (uint32_t *)((uint8_t *)pixels + y * surface->pitch + x * surface->format->BytesPerPixel);
			*p = SDL_MapRGBA(surface->format, c, c, c, 255);
		}
	}
	if (SDL_MUSTLOCK(surface))
		SDL_UnlockSurface(surface);

	SDL_Texture *screenTexture = SDL_CreateTextureFromSurface(renderer, surface);

	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, screenTexture, NULL, NULL);
	SDL_RenderPresent(renderer);

	SDL_DestroyTexture(screenTexture);
}

double lerp(double a, double b, double t)
{
	return a + t * (b - a);
}