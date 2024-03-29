#include "graphics.h"

#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "lib/stb_truetype.h"

static Graphics graphics;

static GLchar *VERT_SRC_2D =
"#version 330 core\n"
"layout (location = 0) in vec2 position;\n"
"layout (location = 1) in vec4 color;\n"
"out vec2 v_position;\n"
"out vec4 v_color;\n"
"void main() {\n"
"    v_color = color;\n"
"    gl_Position = vec4(position, 0.0, 1.0);\n"
"}\n";

static GLchar *FRAG_SRC_2D =
"#version 330 core\n"
"in vec2 v_position\n"
"in vec4 v_color;\n"
"out vec4 f_color;\n"
"void main() {\n"
"    f_color = vec4(1.0);\n"
float t = 1.0 - smoothstep(5.0, 6.0, 0.5)
"            float dist = distance(v_position, vec2(-0.5 + col, -0.5 - row));\n"
"            float delta = fwidth(dist);\n"
"            float alpha = smoothstep(0.45 - delta, 0.45, dist);\n"
"            v_color = mix(v_color, v_color, alpha);\n"
"    gl_FragColor = v_color;\n"
"}\n";

static GLchar *VERT_SRC_TEXT =
"#version 330 core\n"
"layout (location = 0) in vec2 position;\n"
"layout (location = 1) in vec2 tex_coords;\n"
"layout (location = 2) in vec4 color;\n"
"out vec2 v_tex_coords;\n"
"out vec4 v_color;\n"
"void main() {\n"
"    gl_Position = vec4(position, 0.0, 1.0);\n"
"    v_tex_coords = tex_coords;\n"
"    v_color = color;\n"
"}\n";

static GLchar *FRAG_SRC_TEXT =
"#version 330 core\n"
"uniform sampler2D tex;\n"
"in vec2 v_tex_coords;\n"
"in vec4 v_color;\n"
"out vec4 f_color;\n"
"void main() {\n"
"    f_color = v_color * vec4(1.0, 1.0, 1.0, texture(tex, v_tex_coords).r);\n"
"}\n";

static GLchar *VERT_SRC_2D_TEXTURE =
"#version 330 core\n"
"layout (location = 0) in vec2 position;\n"
"layout (location = 1) in vec2 tex_coords;\n"
"out vec2 v_tex_coords;\n"
"void main() {\n"
"    gl_Position = vec4(position, 0.0, 1.0);\n"
"    v_tex_coords = tex_coords;\n"
"}\n";

static GLchar *FRAG_SRC_2D_TEXTURE =
"#version 330 core\n"
"uniform sampler2D tex;\n"
"in vec2 v_tex_coords;\n"
"out vec4 f_color;\n"
"void main() {\n"
"    f_color = texture(tex, v_tex_coords);\n"
"}\n";

static GLuint create_shader(GLenum type, const GLchar *src) {
    GLuint id = glCreateShader(type);
    glShaderSource(id, 1, &src, NULL);
    glCompileShader(id);
    GLint success;
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE) {
        int len = 0;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &len);
        char buffer[len]; // TODO do we have to zero this?
        glGetShaderInfoLog(id, len, NULL, buffer);
        printf("BAD: %s\n", buffer);
        glDeleteShader(id);
        return 0;
    }
    return id;
}

static GLuint create_program(const char *vert_src, const char *frag_src) {
    GLuint vert = create_shader(GL_VERTEX_SHADER, vert_src);
    GLuint frag = create_shader(GL_FRAGMENT_SHADER, frag_src);
    GLuint program = glCreateProgram();
    glAttachShader(program, vert);
    glAttachShader(program, frag);
    glLinkProgram(program);
    glDeleteShader(vert);
    glDeleteShader(frag);
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success == GL_FALSE) {
        int len = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
        char buffer[len]; // TODO do we have to zero this?
        glGetProgramInfoLog(program, len, NULL, buffer);
        printf("BAD PROGRAM: %s\n", buffer);
        glDeleteProgram(program);
        return 0;
    }
    return program;
}

bool graphics_init(char *window_name, int width, int height) {
    // TODO add error checking here
    /* SDL_Window *window = SDL_CreateWindow(window_name, 0, 0, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE); */
    SDL_Window *window = SDL_CreateWindow(window_name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL);

    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 3 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
    SDL_GLContext gl = SDL_GL_CreateContext(window);

    GLenum error = glewInit();
    if (error != GLEW_OK) {
        printf("Failed to initialize GLEW\n");
        return false;
    }
    GLuint program_2d = create_program(VERT_SRC_2D, FRAG_SRC_2D);
    GLuint program_text = create_program(VERT_SRC_TEXT, FRAG_SRC_TEXT);
    GLuint program_texture = create_program(VERT_SRC_2D_TEXTURE, FRAG_SRC_2D_TEXTURE);

    glViewport(0, 0, width, height);

    glDepthFunc(GL_LESS);
    glEnable(GL_BLEND);

    graphics = (Graphics) {
        window,
        gl,
        width,
        height,
        program_2d,
        program_text,
        program_texture,
    };
    return true;
}

void graphics_free() {
    SDL_GL_DeleteContext(graphics.gl);
}

void graphics_swap() {
    SDL_GL_SwapWindow(graphics.window);
}

// Screen to OpenGL normalized coordinates
float gl_x(float x) {
    return -1.0f + 2.0f * x / graphics.screen_width;
}

// Screen to OpenGL normalized coordinates
float gl_y(float y) {
    return 1.0f - 2.0f * y / graphics.screen_height;
}

void gl_draw_triangles(GLfloat vertex_data[], GLuint index_data[], int vertex_count, int triangle_count) {

    glEnable(GL_MULTISAMPLE); // TODO only need this once
    GLuint vao, vbo, ibo;

    GLint vertex_pos_location = -1, vertex_color_location = -1;
    vertex_pos_location = glGetAttribLocation(graphics.program_2d, "position");
    vertex_color_location = glGetAttribLocation(graphics.program_2d, "color");
    if (vertex_pos_location == -1 || vertex_color_location == -1) {
        return;
    }

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 6 * vertex_count * sizeof(GLfloat), vertex_data, GL_STATIC_DRAW);

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * triangle_count * sizeof(GLuint), index_data, GL_STATIC_DRAW);

    /* glBindBuffer(GL_ARRAY_BUFFER, vbo); */

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glEnableVertexAttribArray(vertex_pos_location);
    glVertexAttribPointer(vertex_pos_location, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), NULL);
    glEnableVertexAttribArray(vertex_color_location);
    glVertexAttribPointer(vertex_color_location, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(2*sizeof(GLfloat)));

    glUseProgram(graphics.program_2d);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glDrawElements(GL_TRIANGLES, 3 * triangle_count, GL_UNSIGNED_INT, NULL);
    glDisableVertexAttribArray(vertex_pos_location);
    glUseProgram(0);

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &ibo);
    glDeleteBuffers(1, &vbo);
}

void gl_draw_triangles_2(GLfloat vertex_data[], int vertex_count) {
    GLuint vao, vbo;

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertex_count * 6 * sizeof(GLfloat), vertex_data, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLsizei stride = 6 * sizeof(GLfloat);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, NULL);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(2 * sizeof(GLfloat)));

    glUseProgram(graphics.program_2d);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, vertex_count * 6);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
}

void draw_rect(Rect rect, Color color) {
    float x = (float)rect.x * 2.0f / (float)graphics.screen_width - 1.0f;
    float y = 1.0f - (float)rect.y * 2.0f / (float)graphics.screen_height;
    float width = (float)rect.w * 2.0f / (float)graphics.screen_width;
    float height = -1.0f * (float)rect.h * 2.0f / (float)graphics.screen_height;
    float c[4] = {
        (float)color.r / 255.0f,
        (float)color.g / 255.0f,
        (float)color.b / 255.0f,
        (float)color.a / 255.0f,
    };
    GLfloat vertices[36] = {
        x, y, c[0], c[1], c[2], c[3],
        x + width, y, c[0], c[1], c[2], c[3],
        x + width, y + height, c[0], c[1], c[2], c[3],
        x, y, c[0], c[1], c[2], c[3],
        x + width, y + height, c[0], c[1], c[2], c[3],
        x, y + height, c[0], c[1], c[2], c[3], 
    };

    gl_draw_triangles_2(vertices, 6);
}

void gl_draw_rect(float x, float y, float w, float h, Color color) {
    float x1 = gl_x(x);
    float y1 = gl_y(y);
    float x2 = gl_x(x + w);
    float y2 = gl_y(y + h);

    float r = (float)color.r / 255.0f;
    float g = (float)color.g / 255.0f;
    float b = (float)color.b / 255.0f;
    float a = (float)color.a / 255.0f;

    GLfloat vertex_data[24] = {
        x1, y1, r, g, b, a,
        x2, y1, r, g, b, a,
        x2, y2, r, g, b, a,
        x1, y2, r, g, b, a,
    };
    GLuint index_data[] = { 0, 1, 2, 3, 0, 2 };

    gl_draw_triangles(vertex_data, index_data, 4, 2);
}

void gl_draw_circle_arc(float x, float y, float cr, float theta_start, float theta_arc, Color color) {
    int n = (int)(40.0f * theta_arc / (2*M_PI));
    GLfloat vertex_data[(n + 2) * 6];
    GLuint index_data[n * 3];
    int p = 0, i = 0;

    float r = (float)color.r / 255.0f;
    float g = (float)color.g / 255.0f;
    float b = (float)color.b / 255.0f;
    float a = (float)color.a / 255.0f;

    vertex_data[p] = gl_x(x); p++;
    vertex_data[p] = gl_y(y); p++;
    vertex_data[p] = r; p++;
    vertex_data[p] = g; p++;
    vertex_data[p] = b; p++;
    vertex_data[p] = a; p++;
    vertex_data[p] = gl_x(x + cr * cosf(theta_start)); p++;
    vertex_data[p] = gl_y(y - cr * sinf(theta_start)); p++;
    vertex_data[p] = r; p++;
    vertex_data[p] = g; p++;
    vertex_data[p] = b; p++;
    vertex_data[p] = a; p++;
    for (int t = 0; t < n; t++) {
        float theta = theta_start + (float)(t + 1) * theta_arc / (float)n;
        vertex_data[p] = gl_x(x + cr * cosf(theta)); p++;
        vertex_data[p] = gl_y(y - cr * sinf(theta)); p++;
        vertex_data[p] = r; p++;
        vertex_data[p] = g; p++;
        vertex_data[p] = b; p++;
        vertex_data[p] = a; p++;

        index_data[i] = 0; i++;
        index_data[i] = t + 1; i++;
        index_data[i] = t + 2; i++;
    }

    gl_draw_triangles(vertex_data, index_data, n + 2, n);
}

void gl_draw_rounded_rect(float x, float y, float w, float h, float cr, Color color) {
    gl_draw_rect(x + cr, y, w - 2*cr, h, color);
    gl_draw_rect(x, y + cr, w, h - 2*cr, color);

    gl_draw_circle_arc(x + w - cr, y + cr, cr, 0.0f, M_PI_2, color);
    gl_draw_circle_arc(x + cr, y + cr, cr, M_PI_2, M_PI_2, color);
    gl_draw_circle_arc(x + cr, y + h - cr, cr, M_PI, M_PI_2, color);
    gl_draw_circle_arc(x + w - cr, y + h - cr, cr, M_PI + M_PI_2, M_PI_2, color);
}

void draw_rounded_rect(Rect rect, float cr, Color color) {
    gl_draw_rounded_rect(
        (float)rect.x,
        (float)rect.y,
        (float)rect.w,
        (float)rect.h,
        cr, color);
}

Texture load_texture(char *filename) {
    int x, y, n;
    unsigned char *data = stbi_load(filename, &x, &y, &n, 4);
    Texture texture = create_texture(x, y, data);
    stbi_image_free(data);
    return texture;
}

Font load_font(char *filename) {

    unsigned char ttf_buffer[1<<20];
    unsigned char temp_bitmap[256*256];
    stbtt_bakedchar cdata[96];
    GLuint tex;

    fread(ttf_buffer, 1, 1<<20, fopen(filename, "rb"));
    stbtt_BakeFontBitmap(ttf_buffer, 0, 20.0f, temp_bitmap, 256, 256, 32, 96, cdata);
    glGenTextures(1, &tex);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 256, 256, 0, GL_RED, GL_UNSIGNED_BYTE, temp_bitmap);

    Font font = { tex, cdata };
    return font;
}

Texture create_texture(int width, int height, unsigned char *data) {
    GLuint id = 0;
    glGenTextures(1, &id);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    Texture texture = { id, width, height };
    return texture;
}

void free_texture(Texture texture) {
    glDeleteTextures(1, &texture.id);
}

void draw_texture(Texture texture, Rect rect) {
    Rect src_rect = {0, 0, texture.width, texture.height};
    draw_partial_texture(texture, src_rect, rect);
}

void draw_text(Font font, int x, int y, Color color, char *text) {

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);

    float fx = (float)x;
    float fy = (float)y;

    while (*text) {
        if (*text >= 32 && *text < 128) {
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(font.cdata, 256, 256, *text - 32, &fx, &fy, &q, 1);

            GLfloat x0 = (float)q.x0 * 2.0f / (float)graphics.screen_width - 1.0f;
            GLfloat y0 = -1.0f * (((float)q.y0 + 16.0f) * 2.0f / (float)graphics.screen_height - 1.0f);
            GLfloat x1 = (float)q.x1 * 2.0f / (float)graphics.screen_width - 1.0f;
            GLfloat y1 = -1.0f * (((float)q.y1 + 16.0f) * 2.0f / (float)graphics.screen_height - 1.0f);

            float c[4] = {
                (float)color.r / 255.0f,
                (float)color.g / 255.0f,
                (float)color.b / 255.0f,
                (float)color.a / 255.0f,
            };
            GLfloat vertices[48] = {
                x0, y0, q.s0, q.t0, c[0], c[1], c[2], c[3],
                x1, y0, q.s1, q.t0, c[0], c[1], c[2], c[3],
                x1, y1, q.s1, q.t1, c[0], c[1], c[2], c[3],
                x0, y0, q.s0, q.t0, c[0], c[1], c[2], c[3],
                x1, y1, q.s1, q.t1, c[0], c[1], c[2], c[3],
                x0, y1, q.s0, q.t1, c[0], c[1], c[2], c[3],
            };

            GLuint vao = 0;
            GLuint vbo = 0;
            GLint uniform = glGetUniformLocation(graphics.program_text, "tex");
            glGenVertexArrays(1, &vao);
            glGenBuffers(1, &vbo);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, 48 * sizeof(GLfloat), &vertices, GL_STATIC_DRAW);
            glBindVertexArray(vao);
            GLsizei stride = 8 * sizeof(GLfloat);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, font.tex);
            glUniform1i(uniform, 0);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, NULL);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(2 * sizeof(GLfloat)));
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(4 * sizeof(GLfloat))); // TODO is this void pointer correct?

            glUseProgram(graphics.program_text);
            glDrawArrays(GL_TRIANGLES, 0, 48);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);

            glDeleteBuffers(1, &vbo);
            glDeleteVertexArrays(1, &vao);
        }

        text++;
    }
}

void gl_draw_textures(Texture texture, Rect src_rects[], Rect dest_rects[], int count) {
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);

    GLfloat *vertices = malloc(24 * count * sizeof(GLfloat));

    for (int i = 0; i < count; i++) {

        Rect src_rect = src_rects[i];
        Rect dest_rect = dest_rects[i];

        float x0 = (float)dest_rect.x * 2.0f / (float)graphics.screen_width - 1.0f;
        float x1 = (float)(dest_rect.x + dest_rect.w) * 2.0f / (float)graphics.screen_width - 1.0f;
        float y0 = -1.0f * ((float)dest_rect.y * 2.0f / (float)graphics.screen_height - 1.0f);
        float y1 = -1.0f * ((float)(dest_rect.y + dest_rect.h) * 2.0f / (float)graphics.screen_height - 1.0f);
        float tx0 = (float)src_rect.x / (float)texture.width;
        float tx1 = (float)(src_rect.x + src_rect.w) / (float)texture.width;
        float ty0 = (float)src_rect.y / (float)texture.height;
        float ty1 = (float)(src_rect.y + src_rect.h) / (float)texture.height;

        GLfloat rect_vertices[24] = {
            x0, y0, tx0, ty0,
            x1, y0, tx1, ty0,
            x1, y1, tx1, ty1,
            x0, y0, tx0, ty0,
            x1, y1, tx1, ty1,
            x0, y1, tx0, ty1,
        };

        for (int j = 0; j < 24; j++) {
            vertices[i * 24 + j] = rect_vertices[j];
        }
    }

    GLuint vao = 0;
    GLuint vbo = 0;
    GLint uniform = glGetUniformLocation(graphics.program_texture, "tex");
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 24 * count * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
    glBindVertexArray(vao);
    GLsizei stride = 4 * sizeof(GLfloat);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture.id);
    glUniform1i(uniform, 0);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, NULL);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(2 * sizeof(GLfloat)));

    glUseProgram(graphics.program_texture);
    glDrawArrays(GL_TRIANGLES, 0, 24 * count);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);

    free(vertices);
}

void draw_partial_texture(Texture texture, Rect src_rect, Rect dest_rect) {

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);

    float x0 = (float)dest_rect.x * 2.0f / (float)graphics.screen_width - 1.0f;
    float x1 = (float)(dest_rect.x + dest_rect.w) * 2.0f / (float)graphics.screen_width - 1.0f;
    float y0 = -1.0f * ((float)dest_rect.y * 2.0f / (float)graphics.screen_height - 1.0f);
    float y1 = -1.0f * ((float)(dest_rect.y + dest_rect.h) * 2.0f / (float)graphics.screen_height - 1.0f);
    float tx0 = (float)src_rect.x / (float)texture.width;
    float tx1 = (float)(src_rect.x + src_rect.w) / (float)texture.width;
    float ty0 = (float)src_rect.y / (float)texture.height;
    float ty1 = (float)(src_rect.y + src_rect.h) / (float)texture.height;

    GLfloat vertices[24] = {
        x0, y0, tx0, ty0,
        x1, y0, tx1, ty0,
        x1, y1, tx1, ty1,
        x0, y0, tx0, ty0,
        x1, y1, tx1, ty1,
        x0, y1, tx0, ty1,
    };

    GLuint vao = 0;
    GLuint vbo = 0;
    GLint uniform = glGetUniformLocation(graphics.program_texture, "tex");
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(GLfloat), &vertices, GL_STATIC_DRAW);
    glBindVertexArray(vao);
    GLsizei stride = 4 * sizeof(GLfloat);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture.id);
    glUniform1i(uniform, 0);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, NULL);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(2 * sizeof(GLfloat)));

    glUseProgram(graphics.program_texture);
    glDrawArrays(GL_TRIANGLES, 0, 24);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}

void clear_screen(int r, int g, int b, int a) {
    glClearColor(
            (float)r / 255.0f,
            (float)g / 255.0f,
            (float)b / 255.0f,
            (float)a / 255.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

int get_screen_width() {
    return graphics.screen_width;
}

int get_screen_height() {
    return graphics.screen_height;
}
