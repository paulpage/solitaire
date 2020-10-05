#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct {
    int r;
    int g;
    int b;
    int a;
} Color;

typedef struct {
    int x;
    int y;
    int w;
    int h;
} Rect;

typedef struct {
    SDL_Window *window;
    SDL_GLContext gl;
    int screen_width;
    int screen_height;
    GLuint program_2d;
    GLuint program_text;
    GLuint program_texture;
} Graphics;

typedef struct {
    GLuint id;
    int width;
    int height;
} Texture;

bool graphics_init(char *window_name, int width, int height);
void graphics_free();
void graphics_swap();

void draw_rect(Rect rect, Color color);
Texture load_texture(char *filename);
Texture load_font(char *filename);
Texture create_texture(int width, int height, unsigned char *data);
void free_texture(Texture texture);
void draw_texture(Texture texture, Rect rect);
void draw_text(Texture texture, Rect src_rect, Rect dest_rect);
void draw_partial_texture(Texture texture, Rect src_rect, Rect dest_rect);
void clear_screen(int r, int g, int b, int a);
int get_screen_width();
int get_screen_height();

#endif // GRAPHICS_H
