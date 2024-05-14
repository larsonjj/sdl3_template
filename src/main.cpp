#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <cmath>

struct AppContext
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_bool app_quit = SDL_FALSE;
};

int SDL_Fail()
{
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Error %s", SDL_GetError());
    return -1;
}

int SDL_AppInit(void **appstate, int argc, char *argv[])
{
    // init the library, here we make a window so we only need the Video capabilities.
    if (SDL_Init(SDL_INIT_VIDEO)) {
        return SDL_Fail();
    }

    // create a window
    SDL_Window *window = SDL_CreateWindow("Window", 352, 430, SDL_WINDOW_RESIZABLE);
    if (!window) {
        return SDL_Fail();
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL, 0);
    if (!renderer) {
        return SDL_Fail();
    }
    SDL_RendererInfo rendererInfo;
    SDL_GetRendererInfo(renderer, &rendererInfo);
    SDL_Log("Renderer Name: %s", rendererInfo.name);
    SDL_Log("Video Driver: %s", SDL_GetCurrentVideoDriver());

    // print some information about the window
    SDL_ShowWindow(window);
    {
        int width, height, bbwidth, bbheight;
        SDL_GetWindowSize(window, &width, &height);
        SDL_GetWindowSizeInPixels(window, &bbwidth, &bbheight);
        SDL_Log("Window size: %ix%i", width, height);
        SDL_Log("Backbuffer size: %ix%i", bbwidth, bbheight);
        if (width != bbwidth) {
            SDL_Log("This is a highdpi environment.");
        }
    }

    // set up the application data
    *appstate = new AppContext{
        window,
        renderer,
    };

    SDL_Log("Application started successfully!");

    SDL_Surface *surface = NULL;
    SDL_Surface *temp = NULL;
    SDL_Texture *texture = NULL;
    int w, h;

    surface = IMG_Load("./assets/bunny.png");
    if (!surface) {
        SDL_Log("Couldn't load %s: %s\n", "./assets/bunny.png", SDL_GetError());
    }

    temp = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32);
    SDL_DestroySurface(surface);

    texture = SDL_CreateTextureFromSurface(renderer, temp);
    SDL_DestroySurface(temp);
    if (!texture) {
        SDL_Log("Couldn't create texture: %s\n", SDL_GetError());
    }

    SDL_QueryTexture(texture, NULL, NULL, &w, &h);
    SDL_Log("W: %d | H: %d - %s\n", w, h, SDL_GetError());

    return 0;
}

int SDL_AppEvent(void *appstate, const SDL_Event *event)
{
    auto *app = static_cast<AppContext *>(appstate);

    if (event->type == SDL_EVENT_QUIT) {
        app->app_quit = SDL_TRUE;
    }

    return 0;
}

int SDL_AppIterate(void *appstate)
{
    auto *app = static_cast<AppContext *>(appstate);

    // draw a color
    auto time = SDL_GetTicks() / 1000.f;
    auto red = (std::sin(time) + 1) / 2.0 * 255;
    auto green = (std::sin(time / 2) + 1) / 2.0 * 255;
    auto blue = (std::sin(time) * 2 + 1) / 2.0 * 255;

    SDL_SetRenderDrawColor(app->renderer, red, green, blue, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(app->renderer);
    SDL_RenderPresent(app->renderer);

    return app->app_quit;
}

void SDL_AppQuit(void *appstate)
{
    auto *app = static_cast<AppContext *>(appstate);
    if (app) {
        SDL_DestroyRenderer(app->renderer);
        SDL_DestroyWindow(app->window);
        delete app;
    }

    SDL_Quit();
    SDL_Log("Application quit successfully!");
}
