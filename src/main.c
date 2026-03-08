#define SDL_MAIN_USE_CALLBACKS 1 /* use the callbacks instead of main() */

#include "SDL3_mixer/SDL_mixer.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_BUNNIES 100000

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *bunny_texture;
    SDL_Texture *font_texture;
    MIX_Audio *music;
    MIX_Mixer *mixer; // Add mixer to app context
    float pixel_density;
    int app_quit;
    int music_started;
    // Fixed-size bunny arrays.
    SDL_FRect bunnies[MAX_BUNNIES];
    float bunny_x_speeds[MAX_BUNNIES];
    float bunny_y_speeds[MAX_BUNNIES];
    size_t bunny_count;
    TTF_Font *font;
    Uint32 prevTime;
    Uint32 lastTime;
    Uint32 lastBunnyUpdate;
    int frameCount;
    SDL_Texture *bunnyCountTexture;
} AppContext;

static SDL_AppResult SDL_AppFail(void)
{
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Error %s", SDL_GetError());
    return SDL_APP_FAILURE;
}

static void BunnyArrayPush(AppContext *app, SDL_FRect rect, float x_speed, float y_speed)
{
    if (app->bunny_count >= MAX_BUNNIES) {
        SDL_Log("Max bunny count (%d) reached", MAX_BUNNIES);
        return;
    }
    app->bunnies[app->bunny_count] = rect;
    app->bunny_x_speeds[app->bunny_count] = x_speed;
    app->bunny_y_speeds[app->bunny_count] = y_speed;
    app->bunny_count++;
}

// Return a random float in [min, max].
static float random_float(float min, float max)
{
    return min + ((float)rand() / (float)RAND_MAX) * (max - min);
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        return SDL_AppFail();
    }

    // Initialize SDL_mixer
    if (!MIX_Init()) {
        SDL_Log("Couldn't initialize mixer: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_Window *window =
        SDL_CreateWindow("Window", 352, 430, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (!window) {
        return SDL_AppFail();
    }
    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        return SDL_AppFail();
    }
#ifdef __EMSCRIPTEN__
    SDL_SetRenderVSync(renderer, 1); // use requestAnimationFrame on Emscripten
#endif
    float pixel_density = SDL_GetWindowPixelDensity(window);

#ifndef __ANDROID__
    const char *basePath = SDL_GetBasePath();
    if (!basePath) {
        return SDL_AppFail();
    }
    char assets_path[512];
    SDL_snprintf(assets_path, sizeof(assets_path), "%sassets/", basePath);
    basePath = assets_path;
#else
    const char *basePath = "";
#endif

    // Initialize audio.
    SDL_AudioSpec spec;
    spec.freq = 44100;
    spec.format = SDL_AUDIO_S16;
    spec.channels = 2;

    MIX_Mixer *mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec);
    if (!mixer) {
        SDL_Log("Couldn't open audio: %s\n", SDL_GetError());
        return SDL_AppFail();
    } else {
        MIX_GetMixerFormat(mixer, &spec);
        SDL_Log("Opened audio at %d Hz %d bit%s %s", spec.freq, (spec.format & 0xFF),
                (SDL_AUDIO_ISFLOAT(spec.format) ? " (float)" : ""),
                (spec.channels > 2)   ? "surround"
                : (spec.channels > 1) ? "stereo"
                                      : "mono");
    }

    // Load background music.
    char combined_path[512];
    SDL_snprintf(combined_path, sizeof(combined_path), "%s%s", basePath, "background.mp3");

    MIX_Audio *music = MIX_LoadAudio(mixer, combined_path, false); // Changed to local variable
    if (music == NULL) {
        SDL_Log("Couldn't load %s: %s\n", combined_path, SDL_GetError());
        return SDL_AppFail();
    }

    SDL_ShowWindow(window);

    // Load bunny texture.
    SDL_snprintf(combined_path, sizeof(combined_path), "%s%s", basePath, "bunny.png");
    SDL_Surface *surface = IMG_Load(combined_path);
    if (!surface) {
        SDL_Log("Couldn't load %s: %s\n", combined_path, SDL_GetError());
    }
    SDL_Surface *temp = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32);
    SDL_DestroySurface(surface);
    SDL_Texture *bunny_texture = SDL_CreateTextureFromSurface(renderer, temp);
    SDL_DestroySurface(temp);
    if (!bunny_texture) {
        SDL_Log("Couldn't create texture: %s\n", SDL_GetError());
    }
    SDL_SetTextureScaleMode(bunny_texture, SDL_SCALEMODE_NEAREST);

    // Initialize TTF.
    if (!TTF_Init()) {
        SDL_Log("Couldn't initialize TTF: %s\n", SDL_GetError());
        return SDL_AppFail();
    }
    float ptsize = 32 * pixel_density;
    SDL_snprintf(combined_path, sizeof(combined_path), "%s%s", basePath, "monogram.ttf");
    TTF_Font *font = TTF_OpenFont(combined_path, ptsize);
    if (!font) {
        SDL_Log("Couldn't load %f pt font from %s: %s\n", ptsize, combined_path, SDL_GetError());
        return SDL_AppFail();
    }
    TTF_SetFontStyle(font, TTF_STYLE_NORMAL);
    TTF_SetFontOutline(font, 0);
    TTF_SetFontKerning(font, 1);
    TTF_SetFontHinting(font, TTF_HINTING_MONO);

    // Create initial FPS texture.
    const char *message = "FPS: 60";
    SDL_Color white = {255, 255, 255, 0};
    SDL_Surface *text = TTF_RenderText_Solid(font, message, SDL_strlen(message), white);
    if (!text) {
        SDL_Log("Couldn't render text: %s\n", SDL_GetError());
        TTF_CloseFont(font);
        return SDL_AppFail();
    }
    SDL_Texture *font_texture = SDL_CreateTextureFromSurface(renderer, text);
    SDL_DestroySurface(text);
    SDL_SetTextureScaleMode(font_texture, SDL_SCALEMODE_NEAREST);

    srand((unsigned int)time(NULL));

    AppContext *app = malloc(sizeof(AppContext));
    if (!app) {
        return SDL_AppFail();
    }
    app->window = window;
    app->renderer = renderer;
    app->bunny_texture = bunny_texture;
    app->font_texture = font_texture;
    app->music = music;
    app->mixer = mixer; // Store mixer in app context
    app->pixel_density = pixel_density;
    app->app_quit = 0;
    app->bunny_count = 0;
    float bunnyW = 26 * pixel_density;
    float bunnyH = 37 * pixel_density;
    for (int i = 0; i < 1000; i++) {
        SDL_FRect r = {random_float(0.0f, 352.0f), random_float(0.0f, 430.0f), bunnyW, bunnyH};
        BunnyArrayPush(app, r, random_float(-50.0f, 50.0f), random_float(-50.0f, 50.0f));
    }
    app->font = font;
    app->prevTime = 0;
    app->lastTime = 0;
    app->lastBunnyUpdate = 0;
    app->frameCount = 0;
    app->bunnyCountTexture = NULL;

#ifdef __EMSCRIPTEN__
    // On web, music must wait for a user gesture due to browser autoplay policy.
    app->music_started = 0;
#else
    MIX_PlayAudio(app->mixer, app->music);
    app->music_started = 1;
#endif
    *appstate = app;
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    AppContext *app = (AppContext *)appstate;
    if (event->type == SDL_EVENT_QUIT) {
        app->app_quit = 1;
    }
#ifdef __EMSCRIPTEN__
    // Start music on first user gesture to satisfy browser autoplay policy.
    if (!app->music_started &&
        (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN || event->type == SDL_EVENT_KEY_DOWN ||
         event->type == SDL_EVENT_FINGER_DOWN)) {
        app->music_started = 1;
        MIX_PlayAudio(app->mixer, app->music);
    }
#endif
    if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        if (event->button.button == SDL_BUTTON_LEFT) {
            float bunnyW = 26 * app->pixel_density;
            float bunnyH = 37 * app->pixel_density;
            float mouseX = (float)event->button.x * app->pixel_density - bunnyW / 2;
            float mouseY = (float)event->button.y * app->pixel_density - bunnyH / 2;
            for (int i = 0; i < 1000; i++) {
                SDL_FRect r = {mouseX, mouseY, bunnyW, bunnyH};
                float xs = random_float(-50.0f, 50.0f);
                float ys = random_float(-50.0f, 50.0f);
                BunnyArrayPush(app, r, xs, ys);
            }
        }
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    AppContext *app = (AppContext *)appstate;
    // Clear screen white.
    SDL_SetRenderDrawColor(app->renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(app->renderer);

    Uint32 currentTime = (Uint32)SDL_GetTicks();

    int rendererWidth;
    int rendererHeight;
    SDL_GetCurrentRenderOutputSize(app->renderer, &rendererWidth, &rendererHeight);

    float deltaTime = (app->prevTime == 0) ? 0 : (currentTime - app->prevTime) / 1000.0f;
    app->prevTime = currentTime;

    // Update bunny positions.
    for (size_t i = 0; i < app->bunny_count; i++) {
        app->bunnies[i].x += app->bunny_x_speeds[i] * deltaTime;
        app->bunnies[i].y += app->bunny_y_speeds[i] * deltaTime;
        if (app->bunnies[i].x < 0) {
            app->bunnies[i].x = 0;
            app->bunny_x_speeds[i] = SDL_fabsf(app->bunny_x_speeds[i]);
        } else if (app->bunnies[i].x + app->bunnies[i].w > (float)rendererWidth) {
            app->bunnies[i].x = (float)rendererWidth - app->bunnies[i].w;
            app->bunny_x_speeds[i] = -SDL_fabsf(app->bunny_x_speeds[i]);
        }
        if (app->bunnies[i].y < 0) {
            app->bunnies[i].y = 0;
            app->bunny_y_speeds[i] = SDL_fabsf(app->bunny_y_speeds[i]);
        } else if (app->bunnies[i].y + app->bunnies[i].h > (float)rendererHeight) {
            app->bunnies[i].y = (float)rendererHeight - app->bunnies[i].h;
            app->bunny_y_speeds[i] = -SDL_fabsf(app->bunny_y_speeds[i]);
        }
    }

    SDL_FRect bunny_srcrect = {0, 0, 26, 37};
    for (size_t i = 0; i < app->bunny_count; i++) {
        SDL_RenderTexture(app->renderer, app->bunny_texture, &bunny_srcrect, &app->bunnies[i]);
    }

    // Update FPS texture every second.
    app->frameCount++;
    if (currentTime - app->lastTime >= 1000) {
        int fps = app->frameCount;
        app->frameCount = 0;
        app->lastTime = currentTime;
        char fpsText[32];
        SDL_snprintf(fpsText, sizeof(fpsText), "FPS: %d", fps);
        SDL_Color fpsColor = {0, 0, 0, 255};
        SDL_Surface *fpsSurface =
            TTF_RenderText_Blended(app->font, fpsText, SDL_strlen(fpsText), fpsColor);
        if (fpsSurface) {
            SDL_Texture *newFontTexture = SDL_CreateTextureFromSurface(app->renderer, fpsSurface);
            SDL_DestroySurface(fpsSurface);
            if (newFontTexture) {
                SDL_DestroyTexture(app->font_texture);
                app->font_texture = newFontTexture;
            }
        }
    }
    // Render FPS at bottom left.
    float txtW;
    float txtH;
    SDL_GetTextureSize(app->font_texture, &txtW, &txtH);
    float margin = (float)(16 * app->pixel_density);
    float fpsY = (float)(rendererHeight - txtH - margin);
    SDL_FRect fps_dst = {margin, fpsY, txtW, txtH};
    SDL_RenderTexture(app->renderer, app->font_texture, NULL, &fps_dst);

    // Render bunny count at top left.
    if (currentTime - app->lastBunnyUpdate >= 1000) {
        app->lastBunnyUpdate = currentTime;
        char bunnyText[32];
        SDL_snprintf(bunnyText, sizeof(bunnyText), "Bunnies: %d", (int)app->bunny_count);
        SDL_Color black = {0, 0, 0, 255};
        SDL_Surface *bunnySurface =
            TTF_RenderText_Blended(app->font, bunnyText, SDL_strlen(bunnyText), black);
        if (bunnySurface) {
            SDL_Texture *newTexture = SDL_CreateTextureFromSurface(app->renderer, bunnySurface);
            SDL_DestroySurface(bunnySurface);
            if (newTexture) {
                if (app->bunnyCountTexture) {
                    SDL_DestroyTexture(app->bunnyCountTexture);
                }
                app->bunnyCountTexture = newTexture;
            }
        }
    }
    if (app->bunnyCountTexture) {
        float txtW2;
        float txtH2;
        SDL_GetTextureSize(app->bunnyCountTexture, &txtW2, &txtH2);
        SDL_FRect bunny_dst = {margin, margin, txtW2, txtH2};
        SDL_RenderTexture(app->renderer, app->bunnyCountTexture, NULL, &bunny_dst);
    }

    SDL_RenderPresent(app->renderer);
    return app->app_quit ? SDL_APP_SUCCESS : SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    (void)result;
    AppContext *app = (AppContext *)appstate;
    if (app) {
        if (app->music) {
            MIX_DestroyAudio(app->music);
        }
        if (app->mixer) {
            MIX_DestroyMixer(app->mixer); // Clean up mixer
        }
        if (app->bunnyCountTexture) {
            SDL_DestroyTexture(app->bunnyCountTexture);
        }
        SDL_DestroyTexture(app->bunny_texture);
        SDL_DestroyTexture(app->font_texture);
        SDL_DestroyRenderer(app->renderer);
        SDL_DestroyWindow(app->window);
        TTF_CloseFont(app->font);
        free(app);
    }
    TTF_Quit();
    MIX_Quit();
    SDL_Quit();
    SDL_Log("Application quit successfully!");
}
