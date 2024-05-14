#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <cmath>

struct AppContext
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    SDL_Texture *font_texture;
    SDL_bool app_quit = SDL_FALSE;
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

    int flags = IMG_INIT_JPG | IMG_INIT_PNG;
    int initted = IMG_Init(flags);
    if ((initted & flags) != flags) {
        printf("IMG_Init: Failed to init required jpg and png support!\n");
        printf("IMG_Init: %s\n", IMG_GetError());
        // handle error
    }

    if (TTF_Init() < 0) {
        SDL_Log("Couldn't initialize TTF: %s\n", SDL_GetError());
        SDL_Fail();
    }

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

    TTF_Font *font;
    SDL_Surface *text = NULL;
    SDL_Color white = { 0xFF, 0xFF, 0xFF, 0 };
    SDL_Color black = { 0x00, 0x00, 0x00, 0 };
    SDL_Color *forecol;
    SDL_Color *backcol;
    int ptsize = 8;
    TextRenderMethod rendermethod = TextRenderShaded;
    int renderstyle = TTF_STYLE_NORMAL;
    int rendertype = RENDER_LATIN1;
    int outline = 0;
    int hinting = TTF_HINTING_NORMAL;
    int kerning = 1;
    /* Default is black and white */
    forecol = &black;
    backcol = &white;
    int wrap = 0;

    font = TTF_OpenFont("./assets/PixeloidMono.ttf", ptsize);
    if (font == NULL) {
        SDL_Log("Couldn't load %d pt font from %s: %s\n",
                ptsize, "./assets/PixeloidMono.ttf", SDL_GetError());
        return SDL_Fail();
    }

    TTF_SetFontStyle(font, renderstyle);
    TTF_SetFontOutline(font, outline);
    TTF_SetFontKerning(font, kerning);
    TTF_SetFontHinting(font, hinting);

    char message[] = "Hello, World!";
    char string[128];
    switch (rendermethod) {
    case TextRenderSolid:
        text = TTF_RenderText_Solid(font, string, *forecol);
        break;
    case TextRenderShaded:
        text = TTF_RenderText_Shaded(font, string, *forecol, *backcol);
        break;
    case TextRenderBlended:
        text = TTF_RenderText_Blended(font, string, *forecol);
        break;
    }

    switch (rendertype) {
    case RENDER_LATIN1:
        switch (rendermethod) {
        case TextRenderSolid:
            if (wrap) {
                text = TTF_RenderText_Solid_Wrapped(font, message, *forecol, 0);
            } else {
                text = TTF_RenderText_Solid(font, message, *forecol);
            }
            break;
        case TextRenderShaded:
            if (wrap) {
                text = TTF_RenderText_Shaded_Wrapped(font, message, *forecol, *backcol, 0);
            } else {
                text = TTF_RenderText_Shaded(font, message, *forecol, *backcol);
            }
            break;
        case TextRenderBlended:
            if (wrap) {
                text = TTF_RenderText_Blended_Wrapped(font, message, *forecol, 0);
            } else {
                text = TTF_RenderText_Blended(font, message, *forecol);
            }
            break;
        }
        break;

    case RENDER_UTF8:
        switch (rendermethod) {
        case TextRenderSolid:
            if (wrap) {
                text = TTF_RenderUTF8_Solid_Wrapped(font, message, *forecol, 0);
            } else {
                text = TTF_RenderUTF8_Solid(font, message, *forecol);
            }
            break;
        case TextRenderShaded:
            if (wrap) {
                text = TTF_RenderUTF8_Shaded_Wrapped(font, message, *forecol, *backcol, 0);
            } else {
                text = TTF_RenderUTF8_Shaded(font, message, *forecol, *backcol);
            }
            break;
        case TextRenderBlended:
            if (wrap) {
                text = TTF_RenderUTF8_Blended_Wrapped(font, message, *forecol, 0);
            } else {
                text = TTF_RenderUTF8_Blended(font, message, *forecol);
            }
            break;
        }
        break;

    case RENDER_UNICODE:
    {
        Uint16 *unicode_text = SDL_iconv_utf8_ucs2(message);
        switch (rendermethod) {
        case TextRenderSolid:
            if (wrap) {
                text = TTF_RenderUNICODE_Solid_Wrapped(font, unicode_text, *forecol, 0);
            } else {
                text = TTF_RenderUNICODE_Solid(font, unicode_text, *forecol);
            }
            break;
        case TextRenderShaded:
            if (wrap) {
                text = TTF_RenderUNICODE_Shaded_Wrapped(font, unicode_text, *forecol, *backcol, 0);
            } else {
                text = TTF_RenderUNICODE_Shaded(font, unicode_text, *forecol, *backcol);
            }
            break;
        case TextRenderBlended:
            if (wrap) {
                text = TTF_RenderUNICODE_Blended_Wrapped(font, unicode_text, *forecol, 0);
            } else {
                text = TTF_RenderUNICODE_Blended(font, unicode_text, *forecol);
            }
            break;
        }
        SDL_free(unicode_text);
    } break;
    }

    if (text == NULL) {
        SDL_Log("Couldn't render text: %s\n", SDL_GetError());
        TTF_CloseFont(font);
        return SDL_Fail();
    }

    SDL_Texture *font_texture = SDL_CreateTextureFromSurface(renderer, text);
    SDL_Log("Font is generally %d big, and string is %d big\n",
            TTF_FontHeight(font), text->h);

    // set up the application data
    *appstate = new AppContext{
        window,
        renderer,
        texture,
        font_texture
    };

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
    // Update
    SDL_RenderTexture(app->renderer, app->font_texture, NULL, NULL);
    SDL_RenderTexture(app->renderer, app->texture, NULL, NULL);
    SDL_RenderPresent(app->renderer);

    return app->app_quit;
}

void SDL_AppQuit(void *appstate)
{
    auto *app = static_cast<AppContext *>(appstate);
    if (app) {
        SDL_DestroyTexture(app->texture);
        SDL_DestroyTexture(app->font_texture);
        SDL_DestroyRenderer(app->renderer);
        SDL_DestroyWindow(app->window);
        delete app;
    }

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    SDL_Log("Application quit successfully!");
}
