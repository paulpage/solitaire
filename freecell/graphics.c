#include "graphics.h"

#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb_image.h"

static Graphics graphics;

static GLchar *VERT_SRC_2D =
"#version 330 core\n"
"layout (location = 0) in vec2 position;\n"
"layout (location = 1) in vec4 color;\n"
"out vec4 v_color;\n"
"void main() {\n"
"    v_color = color;\n"
"    gl_Position = vec4(position, 0.0, 1.0);\n"
"}\n";

static GLchar *FRAG_SRC_2D =
"#version 330 core\n"
"in vec4 v_color;\n"
"void main() {\n"
"    gl_FragColor = v_color;\n"
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

static GLuint create_program(char *vert_src, char *frag_src) {
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
    SDL_Window *window = SDL_CreateWindow(window_name, 0, 0, width, height, SDL_WINDOW_OPENGL);

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
    GLuint program_texture = create_program(VERT_SRC_2D_TEXTURE, FRAG_SRC_2D_TEXTURE);

    glViewport(0, 0, width, height);

    graphics = (Graphics) {
        window,
        gl,
        width,
        height,
        program_2d,
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
    GLuint vao;
    GLuint vbo;
    GLfloat vertices[36] = {
        x, y, c[0], c[1], c[2], c[3],
        x + width, y, c[0], c[1], c[2], c[3],
        x + width, y + height, c[0], c[1], c[2], c[3],
        x, y, c[0], c[1], c[2], c[3],
        x + width, y + height, c[0], c[1], c[2], c[3],
        x, y + height, c[0], c[1], c[2], c[3], 
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(
            GL_ARRAY_BUFFER,
            36 * sizeof(GLfloat),
            &vertices,
            GL_STATIC_DRAW);
    glBindVertexArray(vao);
    GLsizei stride = 6 * sizeof(GLfloat);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, NULL);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(2 * sizeof(GLfloat)));

    glUseProgram(graphics.program_2d);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
}

Texture load_texture(char *filename) {
    int x, y, n;
    unsigned char *data = stbi_load(filename, &x, &y, &n, 4);
    Texture texture = create_texture(x, y, data);
    stbi_image_free(data);
    return texture;
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

    /* RectF src = normalize(src_rect, texture.width, texture.height, -1.0f, 1.0f); */
    /* RectF dest = normalize(dest_rect, graphics.screen_width, graphics.screen_height, 0.0f, 1.0f); */
    /* float dest_x = (float)dest_rect.x * 2.0f / (float)graphics.screen_width - 1.0f; */
    /* float dest_y = 1.0f - (float)dest_rect.y * 2.0f / (float)graphics.screen_height; */
    /* float dest_width = (float)dest_rect.w * 2.0f / (float)graphics.screen_width; */
    /* float dest_height = -1.0f * (float)rect.h * 2.0f / (float)graphics.screen_height; */

    /* GLfloat vertices[24] = { */
    /*     dest.x, dest.y, 0.0f, 1.0f, */
    /*     dest.x + dest.width, dest.y, 1.0f, 1.0f, */
    /*     dest.x + dest.width, dest.y + dest.height, 1.0f, 0.0f, */
    /*     dest.x, dest.y, 0.0f, 1.0f, */
    /*     dest.x + dest.width, dest.y + dest.height, 1.0f, 0.0f, */
    /*     dest.x, dest.y + dest.height, 0.0f, 0.0f, */
    /* }; */

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
