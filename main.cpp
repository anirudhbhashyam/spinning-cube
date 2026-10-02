#include <exception>
#include <print>
#include <array>
#include <numbers>
#include <cmath>
#include "raylib.h"

#ifndef ENABLE_VERTICES
    #define ENABLE_VERTICES 0
#endif

using i32 = int32_t;
using u8 = uint8_t;
using u32 = uint32_t;
using u64 = uint64_t;

consteval Color hex_to_color(u32 hex) {
    // RRGGBBAA
    return {
        .r = static_cast<u8>(hex >> 8 * 3 & 0xFF),
        .g = static_cast<u8>(hex >> 8 * 2 & 0xFF),
        .b = static_cast<u8>(hex >> 8 * 1 & 0xFF),
        .a = static_cast<u8>(hex >> 8 * 0 & 0xFF)
    };
}

constexpr auto FOREGROUND_COLOR{ hex_to_color(0xf77cafaa) };
constexpr auto BACKGROUND_COLOR{ hex_to_color(0x0e1219aa) };

void draw_line_on_screen(Vector2 p1, Vector2 p2) {
    DrawLineV(p1, p2, hex_to_color(0xc9c9c3aa));
}

Vector2 project_point(Vector3 p, float radius) {
    u32 w{ static_cast<u32>(GetRenderWidth()) };
    u32 h{ static_cast<u32>(GetRenderHeight()) };
    // Project on screen
    Vector2 pp{
        p.x / p.z,
        p.y / p.z
    };

    // Coordinate transform
    Vector2 pp_t{
        (pp.x + 1) / 2.0f * w,
        (1 - (pp.y + 1) / 2.0f) * h
    };

    // Center the point
    Vector2 pp_c{
        pp_t.x - radius,
        pp_t.y - radius
    };
    return pp_c;
}

constexpr Vector3 translate_z(Vector3 p, float distance) {
    Vector3 pp {
        p.x,
        p.y,
        p.z + distance,
    };
    return pp;
}

constexpr Vector3 rotate_xz(Vector3 p, float angle) {
    const float sin{ std::sin(angle) };
    const float cos{ std::cos(angle) };
    return {
        p.x * cos - p.z * sin,
        p.y,
        p.x * sin + p.z * cos
    };
}

Vector2 update_point(Vector3 p, float angle, float radius, float camera_distance) {
    Vector3 rotated{ rotate_xz(p, angle) };
    return project_point(translate_z(rotated, camera_distance), radius);
}

i32 main() {
    constexpr i32 window_width{ 600 };
    constexpr i32 window_height{ 600 };
    constexpr u64 n_vertices{ 8 };
    constexpr u64 n_edges{ (n_vertices * 3) / 2 };
    constexpr float point_radius{ 5.0f };
    constexpr float pi_float{ std::numbers::pi_v<float> };
    constexpr float min_speed{ 0.25f * pi_float };
    constexpr float max_speed{ 2.0f * pi_float };
    constexpr float time_period{ 8.0f };
    constexpr u32 n_glow_layers{ 8 };
    constexpr float glow_width{ 18.0 };
    constexpr float glow_strength{ 0.5 };

    constexpr std::array<Vector3, n_vertices> vertices{{
        {  0.25f,  0.25f, 0.25f },
        { -0.25f,  0.25f, 0.25f },
        {  0.25f, -0.25f, 0.25f },
        { -0.25f, -0.25f, 0.25f },

        {  0.25f,  0.25f, -0.25f },
        { -0.25f,  0.25f, -0.25f },
        {  0.25f, -0.25f, -0.25f },
        { -0.25f, -0.25f, -0.25f }
    }};

    constexpr std::array<std::array<u64, 2>, n_edges> edges{{
        {0, 1},
        {0, 2},
        {0, 4},
        {4, 6},
        {6, 7},
        {7, 3},
        {3, 1},
        {2, 3},
        {2, 6},
        {5, 7},
        {5, 4},
        {5, 1}
    }};

    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(window_width, window_height, "Cube");

    float camera_distance{ 1.0f };
    float angle{ 0 };
    float time{ 0 };

    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        const float dt{ GetFrameTime() };
        time = std::fmod(time  + dt, time_period);
        const float t{ 0.5f - 0.5f * std::cos(2.0f * pi_float * time / time_period) };
        const float speed{ min_speed + (max_speed - min_speed) * t };
        angle = std::fmod(angle + speed * dt, 2.0f * pi_float);
        BeginDrawing();
            ClearBackground(BACKGROUND_COLOR);
            #if ENABLE_VERTICES
                BeginBlendMode(BLEND_ADDITIVE);
                for (const auto& point: vertices) {
                    for (u32 i{ 0 }; i < n_glow_layers; ++i) {
                        const float t{ static_cast<float>(i) / (n_glow_layers - 1) };
                        const float radius{ glow_width * (1.0f - t) + 2.0f * t };
                        DrawCircleV(
                            update_point(point, angle, point_radius, camera_distance),
                            radius,
                            FOREGROUND_COLOR
                        );
                    }
                }
                EndBlendMode();
            #endif
            BeginBlendMode(BLEND_ADDITIVE);
                for (const auto& [v1, v2]: edges) {
                    const auto p1{
                        update_point(vertices[v1], angle, point_radius, camera_distance)
                    };
                    const auto p2{
                        update_point(vertices[v2], angle, point_radius, camera_distance)
                    };
                    for (u32 i{ 0 }; i < n_glow_layers; ++i) {
                        const float t{ static_cast<float>(i) / (n_glow_layers - 1) };
                        const float width{ glow_width * (1.0f - t) + 2.0f * t };
                        DrawLineEx(
                            p1,
                            p2,
                            width,
                            Fade(FOREGROUND_COLOR, glow_strength * (0.3f + t))
                        );
                    }
                }
            EndBlendMode();

            for (const auto& [v1, v2]: edges) {
                const auto p1{
                    update_point(vertices[v1], angle, point_radius, camera_distance)
                };
                const auto p2{
                    update_point(vertices[v2], angle, point_radius, camera_distance)
                };
                DrawLineEx(
                    p1,
                    p2,
                    1.0f,
                    hex_to_color(0xfff0f8ff)
                );
            }
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
