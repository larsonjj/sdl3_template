#define SDL_MAIN_USE_CALLBACKS 1 /* use the callbacks instead of main() */
#include "SDL3_mixer/SDL_mixer.h"
// #include "SDL_init.h"
// #include "SDL_rect.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

static int audio_open = 0;
static Mix_Music *music = NULL;

struct AppContext
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    SDL_Texture *font_texture;
    Mix_Music *music;
    SDL_AppResult app_quit = SDL_APP_SUCCESS;
};

typedef enum
{
    TextRenderSolid,
    TextRenderShaded,
    TextRenderBlended
} TextRenderMethod;

typedef enum
{
    RENDER_LATIN1,
    RENDER_UTF8,
    RENDER_UNICODE
} RenderType;

SDL_AppResult SDL_Fail()
{
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Error %s", SDL_GetError());
    return SDL_APP_FAILURE;
}

extern "C" {
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    // Avoid compiler warnings
    (void)argv;
    (void)argc;

    // init the library, here we make a window so we only need the Video capabilities.
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK | SDL_INIT_GAMEPAD) != 0) {
        return SDL_Fail();
    }

    // create a window
    SDL_Window *window = SDL_CreateWindow("Window", 800, 450, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (!window) {
        return SDL_Fail();
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        return SDL_Fail();
    }
    // SDL_RendererInfo rendererInfo;
    const char *rendererName = SDL_GetRendererName(renderer);

    SDL_AudioSpec spec;
    int loops = -1; // Infinite
    spec.freq = MIX_DEFAULT_FREQUENCY;
    spec.format = MIX_DEFAULT_FORMAT;
    spec.channels = MIX_DEFAULT_CHANNELS;

    /* Open the audio device */
    if (!Mix_OpenAudio(0, &spec)) {
        SDL_Log("Couldn't open audio: %s\n", SDL_GetError());
        return SDL_Fail();
    } else {
        Mix_QuerySpec(&spec.freq, &spec.format, &spec.channels);
        SDL_Log("Opened audio at %d Hz %d bit%s %s", spec.freq,
                (spec.format & 0xFF),
                (SDL_AUDIO_ISFLOAT(spec.format) ? " (float)" : ""),
                (spec.channels > 2) ? "surround" : (spec.channels > 1) ? "stereo"
                                                                       : "mono");
        if (loops) {
            SDL_Log(" (looping)\n");
        }
    }
    audio_open = 1;

    /* Load the requested wave file */
    music = Mix_LoadMUS("./assets/background.mp3");
    if (music == NULL) {
        SDL_Log("Couldn't load %s: %s\n", "./assets/background.mp3", SDL_GetError());
        return SDL_Fail();
    }

    if (!TTF_Init()) {
        SDL_Log("Couldn't initialize TTF: %s\n", SDL_GetError());
        SDL_Fail();
    }

    SDL_Log("Renderer Name: %s", rendererName);
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

    SDL_Log("Application started successfully!");

    SDL_Surface *surface = NULL;
    SDL_Surface *temp = NULL;
    SDL_Texture *texture = NULL;
    float w, h;

    surface = IMG_Load("./assets/bunny.png");
    if (!surface) {
        SDL_Log("Couldn't load %s: %s\n", "./assets/bunny.png", SDL_GetError());
    }

    temp = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32);
    SDL_DestroySurface(surface);

    texture = SDL_CreateTextureFromSurface(renderer, temp);
    SDL_DestroySurface(temp);
    if (!texture) {
        SDL_Log("Couldn't create texture: %s\n", SDL_GetError());
    }

    SDL_GetTextureSize(texture, &w, &h);
    SDL_Log("W: %f | H: %f - %s\n", w, h, SDL_GetError());
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);

    TTF_Font *font;
    SDL_Surface *text = NULL;
    SDL_Color white = { 0xFF, 0xFF, 0xFF, 0 };
    SDL_Color black = { 0x00, 0x00, 0x00, 0 };
    SDL_Color *forecol;
    SDL_Color *backcol;
    int ptsize = 32;
    TextRenderMethod rendermethod = TextRenderSolid;
    int renderstyle = TTF_STYLE_NORMAL;
    int rendertype = RENDER_LATIN1;
    int outline = 0;
    int hinting = TTF_HINTING_MONO;
    int kerning = 1;
    /* Default is black and white */
    forecol = &black;
    backcol = &white;
    int wrap = 0;

    font = TTF_OpenFont("./assets/monogram.ttf", ptsize);
    if (font == NULL) {
        SDL_Log("Couldn't load %d pt font from %s: %s\n",
                ptsize, "./assets/monogram.ttf", SDL_GetError());
        return SDL_Fail();
    }

    TTF_SetFontStyle(font, renderstyle);
    TTF_SetFontOutline(font, outline);
    TTF_SetFontKerning(font, kerning);
    TTF_SetFontHinting(font, hinting);

    char message[] = "FPS: 60";
    char string[128];
    switch (rendermethod) {
    case TextRenderSolid:
        text = TTF_RenderText_Solid(font, string, sizeof(string), *forecol);
        break;
    case TextRenderShaded:
        text = TTF_RenderText_Shaded(font, string, sizeof(string), *forecol, *backcol);
        break;
    case TextRenderBlended:
        text = TTF_RenderText_Blended(font, string, sizeof(string), *forecol);
        break;
    }

    switch (rendertype) {
    case RENDER_LATIN1:
        switch (rendermethod) {
        case TextRenderSolid:
            if (wrap) {
                text = TTF_RenderText_Solid_Wrapped(font, message, sizeof(string), *forecol, 0);
            } else {
                text = TTF_RenderText_Solid(font, message, sizeof(string), *forecol);
            }
            break;
        case TextRenderShaded:
            if (wrap) {
                text = TTF_RenderText_Shaded_Wrapped(font, message, sizeof(string), *forecol, *backcol, 0);
            } else {
                text = TTF_RenderText_Shaded(font, message, sizeof(string), *forecol, *backcol);
            }
            break;
        case TextRenderBlended:
            if (wrap) {
                text = TTF_RenderText_Blended_Wrapped(font, message, sizeof(string), *forecol, 0);
            } else {
                text = TTF_RenderText_Blended(font, message, sizeof(string), *forecol);
            }
            break;
        }
        break;

    case RENDER_UTF8:
        switch (rendermethod) {
        case TextRenderSolid:
            if (wrap) {
                text = TTF_RenderText_Solid_Wrapped(font, message, sizeof(string), *forecol, 0);
            } else {
                text = TTF_RenderText_Solid(font, message, sizeof(string), *forecol);
            }
            break;
        case TextRenderShaded:
            if (wrap) {
                text = TTF_RenderText_Shaded_Wrapped(font, message, sizeof(string), *forecol, *backcol, 0);
            } else {
                text = TTF_RenderText_Shaded(font, message, sizeof(string), *forecol, *backcol);
            }
            break;
        case TextRenderBlended:
            if (wrap) {
                text = TTF_RenderText_Blended_Wrapped(font, message, sizeof(string), *forecol, 0);
            } else {
                text = TTF_RenderText_Blended(font, message, sizeof(string), *forecol);
            }
            break;
        }
        break;
    }

    if (text == NULL) {
        SDL_Log("Couldn't render text: %s\n", SDL_GetError());
        TTF_CloseFont(font);
        return SDL_Fail();
    }

    SDL_Texture *font_texture = SDL_CreateTextureFromSurface(renderer, text);
    SDL_Log("Font is generally %d big, and string is %d big\n",
            TTF_GetFontHeight(font), text->h);
    SDL_SetTextureScaleMode(font_texture, SDL_SCALEMODE_NEAREST);

    // set up the application data
    *appstate = new AppContext{
        window,
        renderer,
        texture,
        font_texture,
        music,
    };

    Mix_PlayMusic(music, loops);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    auto *app = static_cast<AppContext *>(appstate);

    if (event->type == SDL_EVENT_QUIT) {
        app->app_quit = SDL_APP_SUCCESS;
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    auto *app = static_cast<AppContext *>(appstate);

    SDL_SetRenderDrawColor(app->renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(app->renderer);
    // Update

    // Get Renderer height and width
    int rendererWidth, rendererHeight;
    SDL_GetCurrentRenderOutputSize(app->renderer, &rendererWidth, &rendererHeight);

    float width;
    float height;
    int margin = 16;
    SDL_GetTextureSize(app->font_texture, &width, &height);
    SDL_FRect bunny_srcrect = { .x = 0, .y = 0, .w = 26, .h = 37 };
    SDL_FRect bunny_dstrect = { .x = 0, .y = 0, .w = 26, .h = 37 };
    SDL_FRect font_srcrect = { .x = 0, .y = 0, .w = static_cast<float>(width), .h = static_cast<float>(height) };
    SDL_FRect font_dstrect = { .x = static_cast<float>(margin), .y = static_cast<float>(rendererHeight - height - margin), .w = static_cast<float>(width), .h = static_cast<float>(height) };
    SDL_RenderTexture(app->renderer, app->font_texture, &font_srcrect, &font_dstrect);
    SDL_RenderTexture(app->renderer, app->texture, &bunny_srcrect, &bunny_dstrect);
    SDL_RenderPresent(app->renderer);

    return app->app_quit;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    (void)result;
    if (appstate != NULL) {
        auto *app = static_cast<AppContext *>(appstate);
        if (app) {
            Mix_FreeMusic(app->music);
            SDL_DestroyTexture(app->texture);
            SDL_DestroyTexture(app->font_texture);
            SDL_DestroyRenderer(app->renderer);
            SDL_DestroyWindow(app->window);
            delete app;
        }
    }

    TTF_Quit();
    SDL_Quit();
    SDL_Log("Application quit successfully!");
}
}
