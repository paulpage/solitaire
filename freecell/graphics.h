#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#include <stdio.h>
#include <stdbool.h>

#include "lib/stb_truetype.h"

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

typedef struct {
    GLuint tex;
    stbtt_bakedchar *cdata;
} Font;

bool graphics_init(char *window_name, int width, int height);
void graphics_free();
void graphics_swap();

void draw_rect(Rect rect, Color color);
Texture load_texture(char *filename);
Font load_font(char *filename);
Texture create_texture(int width, int height, unsigned char *data);
void free_texture(Texture texture);
void draw_texture(Texture texture, Rect rect);
void gl_draw_textures(Texture texture, Rect src_rects[], Rect dest_rects[], int count);
void draw_text(Font font, int x, int y, Color color, char *text);
void draw_partial_texture(Texture texture, Rect src_rect, Rect dest_rect);
void draw_rounded_rect(Rect rect, float cr, Color color);
void gl_draw_rounded_rect(float x, float y, float w, float h, float cr, Color color);
void clear_screen(int r, int g, int b, int a);
int get_screen_width();
int get_screen_height();

#endif // GRAPHICS_H
