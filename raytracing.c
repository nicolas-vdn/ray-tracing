#include <stdio.h>
#include <SDL2/SDL.h>
#include <math.h>

#define WIDTH 1200
#define HEIGHT 900
#define COLOR_WHITE 0xffffffff
#define COLOR_BLACK 0x00000000
#define COLOR_RAY 0xffd43b
#define RAYS_NUMBER 2000
#ifndef M_PI
#define M_PI atan(1) * 4
#endif
#define RAY_THICKNESS 3

struct Circle
{
    double x;
    double y;
    double r;
};

struct Ray
{
    double x_start, y_start;
    double angle;
};

void FillCircle(SDL_Surface *surface, struct Circle circle, Uint32 color)
{
    double radius_squared = pow(circle.r, 2);

    for (double x = circle.x - circle.r; x <= circle.x + circle.r; x++)
    {
        double x_squared = pow(x - circle.x, 2);

        for (double y = circle.y - circle.r; y <= circle.y + circle.r; y++)
        {
            double distance_squared = x_squared + pow(y - circle.y, 2);
            if (distance_squared < radius_squared)
            {
                SDL_Rect pixel = (SDL_Rect){x, y, 1, 1};
                SDL_FillRect(surface, &pixel, color);
            }
        }
    }
}

void generate_rays(struct Circle circle, struct Ray rays[RAYS_NUMBER])
{
    for (int i = 0; i < RAYS_NUMBER; i++)
    {
        double angle = ((double)i / RAYS_NUMBER) * 2 * M_PI;

        struct Ray ray = {circle.x + circle.r * cos(angle), circle.y + circle.r * sin(angle), angle};

        rays[i] = ray;
    }
}

void FillRays(SDL_Surface *surface, struct Ray rays[RAYS_NUMBER], struct Circle object)
{
    double radius_squared = pow(object.r, 2);

    for (int i = 0; i < RAYS_NUMBER; i++)
    {
        struct Ray ray = rays[i];

        int end_of_screen = 0;
        int object_hit = 0;

        double step = 1;
        double x_draw = ray.x_start;
        double y_draw = ray.y_start;

        while (!end_of_screen && !object_hit)
        {
            x_draw += step * cos(ray.angle);
            y_draw += step * sin(ray.angle);

            double distance_squared = pow(x_draw - object.x, 2) + pow(y_draw - object.y, 2);

            if (distance_squared < radius_squared)
            {
                object_hit = 1;
                continue;
            }

            if (x_draw < 0 || x_draw > WIDTH || y_draw < 0 || y_draw > HEIGHT)
            {
                end_of_screen = 1;
                continue;
            }

            SDL_Rect ray_point = (SDL_Rect){x_draw, y_draw, RAY_THICKNESS, RAY_THICKNESS};

            SDL_FillRect(surface, &ray_point, COLOR_RAY);
        }
    }
}

int main()
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("Raytracing", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);

    SDL_Surface *surface = SDL_GetWindowSurface(window);

    struct Circle circle = {200, 200, 80};
    struct Circle shadow_circle = {650, 300, 140};
    SDL_Rect erase_rect = (SDL_Rect){0, 0, WIDTH, HEIGHT};

    struct Ray rays[RAYS_NUMBER];
    generate_rays(circle, rays);

    double obstacle_speed_x = 1;
    double obstacle_speed_y = 1;
    int simulation_running = 1;
    SDL_Event event;

    double offset_mouse_x;
    double offset_mouse_y;

    while (simulation_running)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                simulation_running = 0;
            }

            if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                offset_mouse_x = event.motion.x - circle.x;
                offset_mouse_y = event.motion.y - circle.y;
            }

            if (event.type == SDL_MOUSEMOTION && event.motion.state != 0)
            {
                circle.x = event.motion.x - offset_mouse_x;
                circle.y = event.motion.y - offset_mouse_y;

                generate_rays(circle, rays);
            }
        }

        SDL_FillRect(surface, &erase_rect, COLOR_BLACK);
        FillCircle(surface, circle, COLOR_WHITE);

        FillCircle(surface, shadow_circle, COLOR_WHITE);
        FillRays(surface, rays, shadow_circle);

        shadow_circle.y += obstacle_speed_y;
        if (shadow_circle.y - shadow_circle.r < 0 || shadow_circle.y + shadow_circle.r > HEIGHT)
            obstacle_speed_y = -obstacle_speed_y;

        shadow_circle.x += obstacle_speed_x;
        if (shadow_circle.x - shadow_circle.r < 0 || shadow_circle.x + shadow_circle.r > WIDTH)
            obstacle_speed_x = -obstacle_speed_x;

        SDL_UpdateWindowSurface(window);
        SDL_Delay(10);
    }
}