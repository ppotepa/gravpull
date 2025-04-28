#pragma once

#include "pch.h"
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <vector>
#include <memory> // For smart pointers

// Base class for all game objects
class Object {
public:
    virtual ~Object() = default;
    virtual int remove() { return 1; }
};


class Circle : public Object {
public:
    float x, y, r;
    ALLEGRO_COLOR color;

    Circle(float x, float y, float r, ALLEGRO_COLOR color)
        : x(x), y(y), r(r), color(color) {
    }
};


class Game {
private:
    std::vector<std::unique_ptr<Object>> objects; 

public:
    Game() = default;
   
    void AddBall(float x, float y, float r, ALLEGRO_COLOR color) {
        objects.emplace_back(std::make_unique<Circle>(x, y, r, color));
    }
    
};
