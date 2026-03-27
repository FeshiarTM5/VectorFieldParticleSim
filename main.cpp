#include "raylib.h"
#include "raymath.h"
#include <stdio.h>
#include <math.h>
#include <vector>

const Vector2 windowSize = {1400, 900};
const float res = 20.0f;

float min(float a, float b){
    return (a < b) ? a : b;
}
float max(float a, float b){
    return (a > b) ? a : b;
}
float fract(float a){
    return a - floor(a);
}

Vector2 fract(Vector2 a){
    return {fract(a.x), fract(a.y)};
}
Vector2 floor(Vector2 a){
    return {floor(a.x), floor(a.y)};
}
Vector2 ceil(Vector2 a){
    return {ceil(a.x), ceil(a.y)};
}

float rand(Vector2 p) {
    return fract(sin(Vector2DotProduct(p, Vector2{127.1, 311.7})) * 43758.5453);
}
Vector2 randVec(Vector2 n){
    return Vector2{cos(rand(n) * PI * 2.0f), sin(rand(n) * PI * 2.0f)};
}
float fade(float n){
    return n * n * n * (6 * n * n - 15 * n + 10);
}
float mix(float a, float b, float c){
    return a * (1.0f - c) + b * c;
}

float noise(Vector2 a, float n){
    Vector2 b = a * 1.0f;
    Vector2 v = fract(b);
    Vector2 i = floor(b);
    Vector2 c00 = randVec(Vector2Add(Vector2Add(i, Vector2{0, 0}), {n, n}));
    Vector2 c10 = randVec(Vector2Add(Vector2Add(i, Vector2{1, 0}), {n, n}));
    Vector2 c01 = randVec(Vector2Add(Vector2Add(i, Vector2{0, 1}), {n, n}));
    Vector2 c11 = randVec(Vector2Add(Vector2Add(i, Vector2{1, 1}), {n, n}));

    float d00 = Vector2DotProduct(c00, Vector2Subtract(v, Vector2{0, 0}));
    float d10 = Vector2DotProduct(c10, Vector2Subtract(v, Vector2{1, 0}));
    float d01 = Vector2DotProduct(c01, Vector2Subtract(v, Vector2{0, 1}));
    float d11 = Vector2DotProduct(c11, Vector2Subtract(v, Vector2{1, 1}));

    float f = mix(mix(d00, d10, fade(v.x)), mix(d01, d11, fade(v.x)), fade(fract(v.y)));
    return f;
}
Vector3 normalNoise(Vector2 a, float n){
    Vector3 aA = {a.x, a.y, noise({a.x, a.y}, n)};
    Vector3 aX = {a.x + 0.001f, a.y, noise({a.x + 0.001f, a.y}, n)};
    Vector3 aY = {a.x, a.y + 0.001f, noise({a.x, a.y + 0.001f}, n)};
    aX = Vector3Subtract(aX, aA);
    aY = Vector3Subtract(aY, aA);
    return Vector3CrossProduct(aX, aY);
}

struct Particle{
    Vector2 pos = {0};
    Vector2 posI = {0};
    Vector2 vel = {0};
    float r = 1.0f;
    Color color = {219, 0, 190, 255};
    float life = 0.0f;
    float lifeSpan = 3.0f;
    void Update(std::vector<Vector2> field){
        Vector2 indexPos = Vector2Scale(pos, 1.0f / res);
        //bilinear interpolation
        Vector2 v1 = field[floor(indexPos.x) * windowSize.y / res + floor(indexPos.y)];
        Vector2 v2 = field[floor(indexPos.x) * windowSize.y / res + ceil(indexPos.y)];
        Vector2 v3 = field[ceil(indexPos.x) * windowSize.y / res + floor(indexPos.y)];
        Vector2 v4 = field[ceil(indexPos.x) * windowSize.y / res + ceil(indexPos.y)];
        float f1 = fract(indexPos.x);
        float f2 = fract(indexPos.y);
        vel = Vector2Lerp(Vector2Lerp(v1, v2, f2), Vector2Lerp(v3, v4, f2), f1);
    }
    void Move(float dt){
        pos = Vector2Add(pos, Vector2Scale(vel, 200.0f * dt));
        life += dt;
    }
    void Draw(){
        float length = Vector2Length(vel);
        float f = length / (0.25f + length);
        Color color1 = {234, 252, 154, 255};
        Color color2 = {221, 158, 255, 255};
        color = ColorLerp(color1, color2, f);
        posI = Vector2Add(pos, Vector2Scale(vel, 10.0f / length));
        DrawLineEx(pos, posI, 3.0f, color);
    }
};

void DrawArrow(Vector2 start, Vector2 end, Color color = WHITE, float thickness = 2.0f, bool triangle = false){
    float length = Vector2Distance(start, end);
    Vector2 direction = Vector2Normalize(Vector2Subtract(end, start));
    Vector2 normal = {direction.y, -direction.x};
    Vector2 middle = Vector2Lerp(start, end, 0.8f);
    Vector2 end1 = Vector2Add(middle, Vector2Scale(normal, length * 0.15f));
    Vector2 end2 = Vector2Add(middle, Vector2Scale(normal, -length * 0.15f));
    if(triangle){
        DrawTriangle(end1, end2, end, color);
        DrawTriangle(end2, end1, end, color);
        DrawLineEx(start, middle, thickness, color);
    }
    else{
        DrawLineEx(end1, end, thickness, color);
        DrawLineEx(end2, end, thickness, color);
        DrawLineEx(start, end, thickness, color);
    }
}

void DrawNoise(){
    float scale = 0.005f;
    for(int x = 0; x < windowSize.x; x++){
        for(int y = 0; y < windowSize.y; y++){
            float value = noise({scale * x, scale * y}, 453) * 255.0f;
            DrawPixel(x, y, {value, value, value, 255});
        }
    }
}

void GenerateArrowGrid(std::vector<Vector2>& directions){
    for(int i = 0; i <= windowSize.x; i+=res){
        for(int j = 0; j <= windowSize.y; j+=res){
            float scale = 0.005f;
            float x = float(i);
            float y = float(j);
            Vector3 normalVector = normalNoise({scale * x, scale * y}, 453);
            Vector3 normal = Vector3Normalize(normalVector);
            Vector2 proj = {normal.x, normal.y};
            directions.push_back(proj);
        }
    }
}
void DrawArrowGrid(std::vector<Vector2> directions){
    for(int i = 0; i <= windowSize.x / res; i++){
        for(int j = 0; j <= windowSize.y / res; j++){
            float x = res * float(i);
            float y = res * float(j);
            Vector2 proj = directions[i * windowSize.y / res + j];
            Color color1 = {209, 192, 100, 255};
            Color color2 = {110, 170, 196, 255};
            Color color = ColorLerp(color1, color2, Vector2Length(proj));
            DrawArrow({x, y}, {x + 0.8f * res * proj.x, y + 0.8f * res * proj.y}, color);
        }
    }
}

void AddParticle(std::vector<Particle>& particles, float dist, float spacing){
    for(float x = -dist / 2; x < dist / 2; x++){
        for(float y = -dist / 2; y < dist / 2; y++){
            Vector2 currPos = Vector2Add(GetMousePosition(), {x * spacing, y * spacing});
            Vector2 newPos = {0};
            newPos.x = min(max(0.0f, currPos.x), windowSize.x);
            newPos.y = min(max(0.0f, currPos.y), windowSize.y);
            if(newPos == currPos){
                particles.push_back({newPos});
            }
        }
    }
}
void CheckParticles(std::vector<Particle>& particles){
    for(auto i = particles.begin(); i != particles.end();) {
        auto& a = *i;
        if(a.pos.x < 0.0f || a.pos.y < 0.0f || a.pos.x > windowSize.x || a.pos.y > windowSize.y || a.life > a.lifeSpan){
            i = particles.erase(i);
        }else{
            ++i;
        }
    }
}
void UpdateParticles(std::vector<Vector2> field, std::vector<Particle>& particles){
    for(auto& a : particles){
        a.Update(field);
    }
}
void MoveParticles(std::vector<Particle>& particles, float dt){
    for(auto& a : particles){
        a.Move(dt);
    }
}
void DrawParticles(std::vector<Particle> particles){
    for(auto a : particles){
        a.Draw();
    }
}

int main(){
    InitWindow(windowSize.x, windowSize.y, "Window");
    std::vector<Vector2> directions;
    std::vector<Particle> particles;
    SetTargetFPS(60);
    float dt = 0.0f;
    float t = 0.0f;
    GenerateArrowGrid(directions);
    while(!WindowShouldClose()){
        dt = GetFrameTime();
        t += dt;
        BeginDrawing();
        DrawRectangle(0, 0, windowSize.x, windowSize.y, {15, 18, 18, 30});
//        DrawNoise();
        if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
            AddParticle(particles, 10.0f, 2.0f);
        }
        CheckParticles(particles);
        if(int(t / 10.0f) % 1 == 0){
            UpdateParticles(directions, particles);
        }
        MoveParticles(particles, dt);
        DrawParticles(particles);
//        DrawArrowGrid(directions);

        EndDrawing();
    }
    CloseWindow();
    return 0;
}
