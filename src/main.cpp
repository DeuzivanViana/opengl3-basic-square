#include <iostream>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

extern "C"
{
#include <GL/glew.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_opengl.h>
}

#define WINDOW_POS SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED

static bool has_error = false;
std::uint32_t window_width = 720;
std::uint32_t window_height = 480;

// -- Shaders
const char* vShaderCode =
    "#version 460 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec3 aColor;\n"
    "uniform mat4 transform;\n"
    "out vec3 bColor;\n"
    "void main() {\n"
    "    gl_Position = transform * vec4(aPos, 1.0);\n"
    "    bColor = aColor;\n"
    "}\n";

const char* fShaderCode =
    "#version 460 core\n"
    "out vec4 FragColor;\n"
    "in vec3 bColor;\n"
    "void main() {\n"
    "    FragColor = vec4(bColor, 1.0);\n"
    "}\n";

void init()
{
    if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
    {
        std::cerr << "[ERR] - " << SDL_GetError() << '\n';
        has_error = true;
    }
}

void init_glew()
{
    GLenum glew_status = glewInit();
    if (glew_status != GLEW_OK)
    {
        std::cerr << "[ERR] GLEW - " << glewGetErrorString(glew_status) << '\n';
        has_error = true;
    }
}

void quit()
{
    SDL_Quit();
}

void opengl_setup()
{
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
}

int main(int, char*[])
{
    init();
    opengl_setup();

    SDL_WindowFlags window_flags = static_cast<SDL_WindowFlags>(SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI);

    std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> window(
        SDL_CreateWindow("2D Square Rotation", WINDOW_POS, window_width, window_height, window_flags),
        &SDL_DestroyWindow
    );

    SDL_Event event;

    SDL_GLContext gl_context = SDL_GL_CreateContext(window.get());
    SDL_GL_MakeCurrent(window.get(), gl_context);

    init_glew();

    unsigned int fShader;
    fShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fShader, 1, &fShaderCode, NULL);
    glCompileShader(fShader);

    unsigned int vShader;
    vShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vShader, 1, &vShaderCode, NULL);
    glCompileShader(vShader);

    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, fShader);
    glAttachShader(shaderProgram, vShader);
    glLinkProgram(shaderProgram);

    // Definindo os vértices do quadrado
    float vertices[] = {
        // Posições           // Cores
        -50.0f,  50.0f, 0.0f,  1.0f, 0.0f, 0.0f,  // Topo esquerdo (vermelho)
         50.0f,  50.0f, 0.0f,  0.0f, 1.0f, 0.0f,  // Topo direito (verde)
         50.0f, -50.0f, 0.0f,  0.0f, 0.0f, 1.0f,  // Base direita (azul)
        -50.0f, -50.0f, 0.0f,  1.0f, 1.0f, 0.0f   // Base esquerda (amarelo)
    };

    unsigned int indices[] = {
        0, 1, 2,   // Primeiro triângulo
        2, 3, 0    // Segundo triângulo
    };

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    float yPosition = 0.0f;
    float fallSpeed = 100.0f;

    while (window.get())
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                window.~unique_ptr();
            }

            // Detecta redimensionamento de janela
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED)
            {
                window_width = event.window.data1;
                window_height = event.window.data2;
                glViewport(0, 0, window_width, window_height);
            }
        }

        glViewport(0, 0, window_width, window_height);
        glClearColor(0.f, 0.f, 0.f, 1.f);
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        float angle = (float)SDL_GetTicks() / 1000.0f; // tempo em segundos
        yPosition += fallSpeed * (1.0f / 60.0f); // velocidade de queda

        if (yPosition > window_height) // Reseta a posição se o quadrado cair abaixo da tela
        {
            yPosition = 0.0f;
        }

        // Ajuste da projeção ortográfica com base no tamanho da janela
        glm::mat4 transform = glm::mat4(1.0f);
        transform = glm::ortho(0.0f, (float)window_width, (float)window_height, 0.0f, -1.0f, 1.0f);
        transform = glm::translate(transform, glm::vec3(window_width / 2, yPosition, 0.0f));
        transform = glm::rotate(transform, angle, glm::vec3(0.0f, 0.0f, 1.0f));

        unsigned int transformLoc = glGetUniformLocation(shaderProgram, "transform");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        SDL_GL_SwapWindow(window.get());
    }

    SDL_GL_DeleteContext(gl_context);
    quit();

    return EXIT_SUCCESS;
}

