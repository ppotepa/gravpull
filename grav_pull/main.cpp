#include "pch.h"
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <omp.h>
#include <algorithm>

const double M_PI = 3.14159265358979323846;

// helper: random float in [lo,hi]
template<typename T>
inline T randf(T lo, T hi) {
    return lo + (hi - lo) * (static_cast<T>(std::rand()) / static_cast<T>(RAND_MAX));
}
// overload: random float in [lo,hi] where hi is int
template<typename T>
inline T randf(T lo, int hi) {
    return lo + (hi - static_cast<int>(lo)) * (static_cast<T>(std::rand()) / static_cast<T>(RAND_MAX));
}
// overload: random float in [lo,hi] where hi is double
template<typename T>
inline T randf(T lo, double hi) {
    return lo + (hi - static_cast<int>(lo)) * (static_cast<T>(std::rand()) / static_cast<double>(RAND_MAX));
}

// clamp helper
template <typename T>
inline T clamp(T value, T min, T max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

// ====================
// CONFIGURABLE PARAMETERS
// ====================
static const int SCREEN_WIDTH = 1920;
static const int SCREEN_HEIGHT = 1080;
static constexpr float TIME_STEP = 1.0f / 60.0f;
static constexpr float G_FACTOR = 122.67430e-1f;
static constexpr float INITIAL_SUN_MASS = 1.0f;
static constexpr float VELOCITY_MULTIPLIER = 1.0f;
static const int BALL_COUNT = 3000;
static constexpr float BALL_RAD_MIN = 0.5f;
static constexpr float BALL_RAD_MAX = 5.0f;
static constexpr float BALL_SIZE_MULT = 1.5f;
static constexpr float JITTER_MIN = -1.9f;
static constexpr float JITTER_MAX = 2.1f;
static constexpr int TRACE_ALPHA = 200;
// RED BODIES CONFIG
static constexpr float RED_BODY_PROB = 0.05f;  // 5% red
static constexpr float RED_MASS_MULT = 1.20f;  // 20% heavier
static constexpr float RED_RADIUS_MULT = 1.05f;  // 5% larger

int main() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    omp_set_num_threads(omp_get_max_threads());

    // Allegro init
    if (!al_init()) return 1;
    if (!al_install_keyboard()) return 1;
    if (!al_init_primitives_addon()) return 1;
    al_init_font_addon();
    al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);

    ALLEGRO_DISPLAY* display = al_create_display(SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!display) return 1;

    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
    ALLEGRO_TIMER* timer = al_create_timer(TIME_STEP);
    if (!queue || !timer) {
        al_destroy_display(display);
        return 1;
    }
    al_register_event_source(queue, al_get_display_event_source(display));
    al_register_event_source(queue, al_get_timer_event_source(timer));
    al_register_event_source(queue, al_get_keyboard_event_source());

    ALLEGRO_FONT* font = al_create_builtin_font();
    if (!font) {
        al_destroy_timer(timer);
        al_destroy_event_queue(queue);
        al_destroy_display(display);
        return 1;
    }

    const float cx = SCREEN_WIDTH * 0.5f;
    const float cy = SCREEN_HEIGHT * 0.5f;
    const float R_screen = std::sqrt(cx * cx + cy * cy);
    const float R_min = R_screen;
    const float R_max = R_screen + 500.0f;

    // camera
    float camX = cx, camY = cy;
    float zoom = 1.0f;
    const float PAN_SPEED = 500.0f;
    bool moveUp = false, moveDown = false, moveLeft = false, moveRight = false;
    bool zoomIn = false, zoomOut = false;
    const float ZOOM_FACTOR = 1.1f;

    // SoA arrays
    const int N = BALL_COUNT + 1;
    std::vector<float> posX(N), posY(N), velX(N), velY(N), mass(N), radius(N);
    std::vector<bool> fixed(N), isRed(N, false);
    std::vector<float> accelX(N), accelY(N);

    // loading bar
    auto drawLoading = [&](float frac) {
        al_clear_to_color(al_map_rgb(0, 0, 0));
        int barW = SCREEN_WIDTH / 2, barH = 30;
        int x = (SCREEN_WIDTH - barW) / 2, y = (SCREEN_HEIGHT - barH) / 2;
        al_draw_rectangle(x, y, x + barW, y + barH, al_map_rgb(255, 255, 255), 2);
        al_draw_filled_rectangle(x + 2, y + 2, x + 2 + (barW - 4) * frac, y + barH - 2, al_map_rgb(0, 255, 0));
        char buf[32]; snprintf(buf, 32, "Loading: %d%%", int(frac * 100));
        int tw = al_get_text_width(font, buf);
        al_draw_text(font, al_map_rgb(255, 255, 255), (SCREEN_WIDTH - tw) / 2, y - 40, 0, buf);
        al_flip_display();
        };

    // init balls
    for (int i = 0; i < BALL_COUNT; ++i) {
        float theta = randf(0.f, 2.f * M_PI);
        float r_spawn = randf(R_min, R_max);
        float x = cx + r_spawn * std::cos(theta);
        float y = cy + r_spawn * std::sin(theta);
        float r = randf(BALL_RAD_MIN, BALL_RAD_MAX) * BALL_SIZE_MULT;
        float dx = x - cx, dy = y - cy;
        float dist = std::sqrt(dx * dx + dy * dy) + 1e-6f;
        float v = std::sqrt((G_FACTOR * INITIAL_SUN_MASS) / dist) * randf(JITTER_MIN, JITTER_MAX);
        float ux = -dy / dist, uy = dx / dist;
        if (randf(0.f, 1.f) < RED_BODY_PROB) { isRed[i] = true; r *= RED_RADIUS_MULT; }
        posX[i] = x; posY[i] = y;
        velX[i] = ux * v; velY[i] = uy * v;
        radius[i] = r;
        mass[i] = r * r * r * (isRed[i] ? RED_MASS_MULT : 1.f);
        fixed[i] = false;
        drawLoading(float(i) / BALL_COUNT);
    }
    // sun
    posX[BALL_COUNT] = cx; posY[BALL_COUNT] = cy;
    velX[BALL_COUNT] = 0; velY[BALL_COUNT] = 0;
    radius[BALL_COUNT] = BALL_RAD_MAX * 2;
    mass[BALL_COUNT] = INITIAL_SUN_MASS;
    fixed[BALL_COUNT] = false;
    drawLoading(1.f);
    al_rest(0.5);

    // find max mass
    float maxBallMass = 0;
    for (int i = 0; i < BALL_COUNT; ++i)
        maxBallMass = std::max(maxBallMass, mass[i]);

    // main loop
    al_clear_to_color(al_map_rgb(0, 0, 0)); al_flip_display();
    al_start_timer(timer);
    const ALLEGRO_COLOR fadeC = al_map_rgba(0, 0, 0, TRACE_ALPHA);
    bool running = true, redraw = true;
    double lastTime = al_get_time(), displayFPS = 0;
    int frames = 0;

    while (running) {
        ALLEGRO_EVENT ev; al_wait_for_event(queue, &ev);
        switch (ev.type) {
        case ALLEGRO_EVENT_DISPLAY_CLOSE: running = false; break;
        case ALLEGRO_EVENT_KEY_DOWN:
            if (ev.keyboard.keycode == ALLEGRO_KEY_W) moveUp = true;
            if (ev.keyboard.keycode == ALLEGRO_KEY_S) moveDown = true;
            if (ev.keyboard.keycode == ALLEGRO_KEY_A) moveLeft = true;
            if (ev.keyboard.keycode == ALLEGRO_KEY_D) moveRight = true;
            if (ev.keyboard.keycode == ALLEGRO_KEY_Q) zoomIn = true;
            if (ev.keyboard.keycode == ALLEGRO_KEY_E) zoomOut = true;
            break;
        case ALLEGRO_EVENT_KEY_UP:
            if (ev.keyboard.keycode == ALLEGRO_KEY_W) moveUp = false;
            if (ev.keyboard.keycode == ALLEGRO_KEY_S) moveDown = false;
            if (ev.keyboard.keycode == ALLEGRO_KEY_A) moveLeft = false;
            if (ev.keyboard.keycode == ALLEGRO_KEY_D) moveRight = false;
            if (ev.keyboard.keycode == ALLEGRO_KEY_Q) zoomIn = false;
            if (ev.keyboard.keycode == ALLEGRO_KEY_E) zoomOut = false;
            break;
        case ALLEGRO_EVENT_TIMER:
            redraw = true;
            float pan = PAN_SPEED * TIME_STEP / zoom;
            if (moveUp)    camY -= pan;
            if (moveDown)  camY += pan;
            if (moveLeft)  camX -= pan;
            if (moveRight) camX += pan;
            if (zoomIn)    zoom *= ZOOM_FACTOR;
            if (zoomOut)   zoom /= ZOOM_FACTOR;
            zoom = clamp(zoom, 0.1f, 10.f);
            break;
        }
        // compute accelerations
#pragma omp parallel for
        for (int i = 0; i < N; ++i) {
            if (fixed[i]) { accelX[i] = accelY[i] = 0; continue; }
            float ax = 0, ay = 0;
            float xi = posX[i], yi = posY[i];
            for (int j = 0; j < N; ++j) {
                if (i == j) continue;
                float dx = xi - posX[j], dy = yi - posY[j];
                float d2 = dx * dx + dy * dy + 1e-6f;
                float invD = 1.f / std::sqrt(d2);
                float f = -G_FACTOR * mass[j] * invD * invD * invD;
                ax += f * dx;
                ay += f * dy;
            }
            accelX[i] = ax;
            accelY[i] = ay;
        }
        // integrate positions
        for (int i = 0; i < N; ++i) {
            if (fixed[i]) continue;
            velX[i] += accelX[i] * TIME_STEP;
            velY[i] += accelY[i] * TIME_STEP;
            posX[i] += velX[i] * TIME_STEP * VELOCITY_MULTIPLIER;
            posY[i] += velY[i] * TIME_STEP * VELOCITY_MULTIPLIER;
        }
        // render
        if (redraw && al_is_event_queue_empty(queue)) {
            al_draw_filled_rectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, fadeC);
            for (int i = 0; i < N; ++i) {
                float sx = (posX[i] - camX) * zoom + SCREEN_WIDTH * 0.5f;
                float sy = (posY[i] - camY) * zoom + SCREEN_HEIGHT * 0.5f;
                float sr = radius[i] * zoom; if (sr < 1.f) sr = 1.f;
                ALLEGRO_COLOR col;
                if (i == BALL_COUNT) col = al_map_rgb(0, 0, 255);
                else if (isRed[i]) col = al_map_rgb(255, 0, 0);
                else {
                    float t = clamp(mass[i] / maxBallMass, 0.f, 1.f);
                    unsigned char r = static_cast<unsigned char>(t * 255);
                    unsigned char g = static_cast<unsigned char>((1.f - t) * 100);
                    unsigned char b = static_cast<unsigned char>((1.f - t) * 50);
                    col = al_map_rgb(r, g, b);
                }
                if (sx + sr<0 || sx - sr>SCREEN_WIDTH || sy + sr<0 || sy - sr>SCREEN_HEIGHT) continue;
                al_draw_filled_circle(sx, sy, sr, col);
            }
            // draw FPS
            frames++;
            double now = al_get_time();
            if (now - lastTime >= 1.0) {
                displayFPS = frames / (now - lastTime);
                frames = 0;
                lastTime = now;
            }
            al_draw_textf(font, al_map_rgb(255, 255, 0), 10, 10, 0, "FPS: %.1f", displayFPS);
            // draw controls instructions
            al_draw_text(font, al_map_rgb(200, 200, 200), 10, SCREEN_HEIGHT - 20, 0,
                "W/A/S/D: Pan | Q/E: Zoom | Esc: Quit");
            al_flip_display();
            redraw = false;
        }
    }
    // cleanup
    al_destroy_font(font);
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);
    al_destroy_display(display);
    return 0;
}
