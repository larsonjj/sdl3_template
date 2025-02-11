#define SDL_MAIN_USE_CALLBACKS 1 /* use the callbacks instead of main() */
#include "SDL3_mixer/SDL_mixer.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <random>
#include <vector>

static int audio_open = 0;
static Mix_Music *music = NULL;

struct AppContext
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *bunny_texture;
    SDL_Texture *font_texture; // Texture used for the FPS text.
    Mix_Music *music;
    float pixel_density;
    SDL_AppResult app_quit = SDL_APP_CONTINUE;
    std::vector<SDL_FRect> bunnies;
    std::mt19937 rng;
    std::vector<float> bunny_x_speeds;
    std::vector<float> bunny_y_speeds;
    TTF_Font *font; // Store the font so we can update the FPS text.
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

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    (void)argv;
    (void)argc;

    if (not SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        return SDL_Fail();
    }

    SDL_Window *window = SDL_CreateWindow("Window", 352, 430, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (not window) {
        return SDL_Fail();
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
    if (not renderer) {
        return SDL_Fail();
    }

    float pixel_density = SDL_GetWindowPixelDensity(window);

#if __ANDROID__
    const char *basePath = "";
#else
    auto basePathPtr = SDL_GetBasePath();
    if (not basePathPtr) {
        return SDL_Fail();
    }
    char assets_path[512];
    SDL_snprintf(assets_path, sizeof(assets_path), "%s%s", basePathPtr, "assets/");
    const char *basePath = assets_path;
#endif

    SDL_AudioSpec spec;
    int loops = -1; // Infinite
    spec.freq = MIX_DEFAULT_FREQUENCY;
    spec.format = MIX_DEFAULT_FORMAT;
    spec.channels = MIX_DEFAULT_CHANNELS;

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

    const char *bg_music_asset_filepath = "background.mp3";
    const char *root_filepath = basePath;
    char combined_path[512];
    SDL_snprintf(combined_path, sizeof(combined_path), "%s%s", root_filepath, bg_music_asset_filepath);

    music = Mix_LoadMUS(combined_path);
    if (music == NULL) {
        SDL_Log("Couldn't load %s: %s\n", combined_path, SDL_GetError());
        return SDL_Fail();
    }

    // Optionally enable vsync (currently commented out)
    // SDL_SetRenderVSync(renderer, -1);

    SDL_ShowWindow(window);
    {
        int width, height, bbwidth, bbheight;
        SDL_GetWindowSize(window, &width, &height);
        SDL_GetWindowSizeInPixels(window, &bbwidth, &bbheight);
        SDL_Log("Window size: %ix%i", width, height);
        SDL_Log("Backbuffer size: %ix%i", bbwidth, bbheight);
        SDL_Log("Device pixel ratio: %f", SDL_GetWindowPixelDensity(window));
        if (width != bbwidth) {
            SDL_Log("This is a highdpi environment.");
        }
    }
    SDL_Log("Application started successfully!");

    SDL_Surface *surface = NULL;
    SDL_Surface *temp = NULL;
    SDL_Texture *bunny_texture = NULL;
    float w, h;
    const char *bunny_asset_filepath = "bunny.png";
    memset(combined_path, 0, sizeof(combined_path));
    SDL_snprintf(combined_path, sizeof(combined_path), "%s%s", root_filepath, bunny_asset_filepath);

    surface = IMG_Load(combined_path);
    if (!surface) {
        SDL_Log("Couldn't load %s: %s\n", combined_path, SDL_GetError());
    }

    temp = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32);
    SDL_DestroySurface(surface);

    bunny_texture = SDL_CreateTextureFromSurface(renderer, temp);
    SDL_DestroySurface(temp);
    if (!bunny_texture) {
        SDL_Log("Couldn't create texture: %s\n", SDL_GetError());
    }

    SDL_GetTextureSize(bunny_texture, &w, &h);
    SDL_Log("W: %f | H: %f - %s\n", w, h, SDL_GetError());
    SDL_SetTextureScaleMode(bunny_texture, SDL_SCALEMODE_NEAREST);

    if (!TTF_Init()) {
        SDL_Log("Couldn't initialize TTF: %s\n", SDL_GetError());
        return SDL_Fail();
    }

    TTF_Font *font;
    SDL_Surface *text = NULL;
    SDL_Color white = { 0xFF, 0xFF, 0xFF, 0 };
    SDL_Color black = { 0x00, 0x00, 0x00, 0 };
    SDL_Color *forecol = &black;
    SDL_Color *backcol = &white;
    float ptsize = 32 * pixel_density;
    TextRenderMethod rendermethod = TextRenderSolid;
    int renderstyle = TTF_STYLE_NORMAL;
    int rendertype = RENDER_LATIN1;
    int outline = 0;
    TTF_HintingFlags hinting = TTF_HINTING_MONO;
    int kerning = 1;
    int wrap = 0;

    const char *font_asset_filepath = "monogram.ttf";
    memset(combined_path, 0, sizeof(combined_path));
    SDL_snprintf(combined_path, sizeof(combined_path), "%s%s", root_filepath, font_asset_filepath);

    font = TTF_OpenFont(combined_path, ptsize);
    if (font == NULL) {
        SDL_Log("Couldn't load %f pt font from %s: %s\n", ptsize, combined_path, SDL_GetError());
        return SDL_Fail();
    }
    TTF_SetFontStyle(font, renderstyle);
    TTF_SetFontOutline(font, outline);
    TTF_SetFontKerning(font, kerning);
    TTF_SetFontHinting(font, hinting);

    char message[] = "FPS: 60";
    char string[7];
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
    SDL_Log("Font is generally %d big, and string is %d big\n", TTF_GetFontHeight(font), text->h);
    SDL_SetTextureScaleMode(font_texture, SDL_SCALEMODE_NEAREST);

    // Initialize bunnies: only 1 bunny initially.
    std::vector<SDL_FRect> bunnies;
    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> speed(-50.0f, 50.0f);
    std::uniform_real_distribution<float> x_pos(0.0f, 352.0f);
    std::uniform_real_distribution<float> y_pos(0.0f, 430.0f);
    std::vector<float> bunny_x_speeds;
    std::vector<float> bunny_y_speeds;

    bunnies.push_back({ x_pos(rng), y_pos(rng), 26 * pixel_density, 37 * pixel_density });
    bunny_x_speeds.push_back(speed(rng));
    bunny_y_speeds.push_back(speed(rng));

    *appstate = new AppContext{
        window,
        renderer,
        bunny_texture,
        font_texture,
        music,
        pixel_density,
        SDL_APP_CONTINUE,
        bunnies,
        rng,
        bunny_x_speeds,
        bunny_y_speeds,
        font // Store the font pointer for later FPS updates.
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

    if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        if (event->button.button == SDL_BUTTON_LEFT) { // Process only left mouse clicks
            // Compute bunny dimensions
            float bunnyW = 26 * app->pixel_density;
            float bunnyH = 37 * app->pixel_density;
            // Offset the mouse coordinate so the bunny is centered on the click
            float mouseX = static_cast<float>(event->button.x * app->pixel_density) - bunnyW / 2;
            float mouseY = static_cast<float>(event->button.y * app->pixel_density) - bunnyH / 2;

            std::uniform_real_distribution<float> speed(-50.0f, 50.0f);
            for (int i = 0; i < 100; ++i) {
                app->bunnies.push_back({ mouseX, mouseY, bunnyW, bunnyH });
                app->bunny_x_speeds.push_back(speed(app->rng));
                app->bunny_y_speeds.push_back(speed(app->rng));
            }
        }
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    auto *app = static_cast<AppContext *>(appstate);

    // Draw white background.
    auto red = 255;
    auto green = 255;
    auto blue = 255;
    SDL_SetRenderDrawColor(app->renderer, static_cast<uint8_t>(red), static_cast<uint8_t>(green), static_cast<uint8_t>(blue), SDL_ALPHA_OPAQUE);
    SDL_RenderClear(app->renderer);

    // Get renderer dimensions.
    int rendererWidth, rendererHeight;
    SDL_GetCurrentRenderOutputSize(app->renderer, &rendererWidth, &rendererHeight);

    // Compute delta time (in seconds) from the actual frame times.
    static Uint32 prevTime = SDL_GetTicks();
    Uint32 currentTime = SDL_GetTicks();
    float deltaTime = (currentTime - prevTime) / 1000.0f;
    prevTime = currentTime;

    // Update bunny positions using the computed deltaTime.
    for (size_t i = 0; i < app->bunnies.size(); ++i) {
        app->bunnies[i].x += app->bunny_x_speeds[i] * deltaTime;
        app->bunnies[i].y += app->bunny_y_speeds[i] * deltaTime;

        if (app->bunnies[i].x < 0 || app->bunnies[i].x + app->bunnies[i].w > rendererWidth) {
            app->bunny_x_speeds[i] *= -1;
        }
        if (app->bunnies[i].y < 0 || app->bunnies[i].y + app->bunnies[i].h > rendererHeight) {
            app->bunny_y_speeds[i] *= -1;
        }
    }

    SDL_FRect bunny_srcrect = { .x = 0, .y = 0, .w = 26, .h = 37 };
    for (const auto &bunny : app->bunnies) {
        SDL_RenderTexture(app->renderer, app->bunny_texture, &bunny_srcrect, &bunny);
    }

    // FPS update: count frames and update the FPS texture once per second.
    {
        static Uint32 lastTime = SDL_GetTicks();
        static int frameCount = 0;
        frameCount++;
        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - lastTime >= 1000) {
            int fps = frameCount;
            frameCount = 0;
            lastTime = currentTime;
            char fpsText[32];
            SDL_snprintf(fpsText, sizeof(fpsText), "FPS: %d", fps);
            SDL_Color fpsColor = { 0, 0, 0, 255 }; // Black color.
                                                   // Pass the actual length of the string instead of sizeof(fpsText).
            SDL_Surface *fpsSurface = TTF_RenderText_Blended(app->font, fpsText, static_cast<int>(strlen(fpsText)), fpsColor);
            if (fpsSurface) {
                SDL_Texture *newFontTexture = SDL_CreateTextureFromSurface(app->renderer, fpsSurface);
                SDL_DestroySurface(fpsSurface);
                if (newFontTexture) {
                    SDL_DestroyTexture(app->font_texture);
                    app->font_texture = newFontTexture;
                }
            }
        }
    }

    // Render the FPS text at the bottom left.
    float width, height;
    int margin = 16 * app->pixel_density;
    SDL_GetTextureSize(app->font_texture, &width, &height);
    SDL_FRect font_srcrect = { .x = 0, .y = 0, .w = width, .h = height };
    SDL_FRect font_dstrect = { .x = static_cast<float>(margin), .y = static_cast<float>(rendererHeight - height - margin), .w = width, .h = height };
    SDL_RenderTexture(app->renderer, app->font_texture, &font_srcrect, &font_dstrect);

    // Render bunny count text at the top left.
    {
        static Uint32 lastBunnyUpdate = SDL_GetTicks();
        static SDL_Texture *bunnyCountTexture = nullptr;
        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - lastBunnyUpdate >= 1000) {
            lastBunnyUpdate = currentTime;
            char bunnyText[32];
            SDL_snprintf(bunnyText, sizeof(bunnyText), "Bunnies: %d", static_cast<int>(app->bunnies.size()));
            SDL_Color black = { 0, 0, 0, 255 };
            SDL_Surface *bunnySurface = TTF_RenderText_Blended(app->font, bunnyText, static_cast<int>(strlen(bunnyText)), black);
            if (bunnySurface) {
                SDL_Texture *newTexture = SDL_CreateTextureFromSurface(app->renderer, bunnySurface);
                SDL_DestroySurface(bunnySurface);
                if (newTexture) {
                    if (bunnyCountTexture) {
                        SDL_DestroyTexture(bunnyCountTexture);
                    }
                    bunnyCountTexture = newTexture;
                }
            }
        }
        if (bunnyCountTexture) {
            float txtW, txtH;
            SDL_GetTextureSize(bunnyCountTexture, &txtW, &txtH);
            int margin = 16 * app->pixel_density;
            SDL_FRect srcRect = { 0, 0, txtW, txtH };
            SDL_FRect dstRect = { static_cast<float>(margin), static_cast<float>(margin), txtW, txtH };
            SDL_RenderTexture(app->renderer, bunnyCountTexture, &srcRect, &dstRect);
        }
    }

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
            SDL_DestroyTexture(app->bunny_texture);
            SDL_DestroyTexture(app->font_texture);
            SDL_DestroyRenderer(app->renderer);
            SDL_DestroyWindow(app->window);
            TTF_CloseFont(app->font);
            delete app;
        }
    }
    TTF_Quit();
    SDL_Quit();
    SDL_Log("Application quit successfully!");
}
