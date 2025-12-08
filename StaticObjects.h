#pragma once

#include "GameObject.h"

//////////////////////////////////////////       StaticObject         //////////////////////////////////////////

// Base class for immovable objects
class StaticObject : public GameObject {
    public:
        StaticObject() : GameObject() {}
        StaticObject(const Point& pos, char spr, ObjectType t) : GameObject(pos, spr, t) {}
        
        bool isPickable() const override { return false; }
        bool isInteractable() const override { return false; }
    };
    
    //////////////////////////////////////////           Wall             //////////////////////////////////////////
    
    // Indestructible wall that blocks movement
    class Wall : public StaticObject {
    public:
        Wall() : StaticObject() {
            sprite = 'W';
            type = ObjectType::WALL;
        }
        
        Wall(const Point& pos) : StaticObject(pos, 'W', ObjectType::WALL) {}
        
        GameObject* clone() const override { return new Wall(*this); }
        const char* getName() const override { return "Wall"; }
        
        bool isBlocking() const override { return true; }
        bool onExplosion() override { return false; }
    };
    
    //////////////////////////////////////////      BreakableWall         //////////////////////////////////////////
    
    // Wall that can be destroyed by explosions
    class BreakableWall : public StaticObject {
    public:
        BreakableWall() : StaticObject() {
            sprite = '=';
            type = ObjectType::BREAKABLE_WALL;
        }
        
        BreakableWall(const Point& pos) : StaticObject(pos, '=', ObjectType::BREAKABLE_WALL) {}
        
        GameObject* clone() const override { return new BreakableWall(*this); }
        const char* getName() const override { return "Breakable Wall"; }
        
        bool isBlocking() const override { return true; }
        bool onExplosion() override { return true; }
    };
    
    //////////////////////////////////////////         Obstacle           //////////////////////////////////////////
    
    // Blocks movement until switches activate or bomb destroys it
    class Obstacle : public StaticObject {
    private:
        bool removedBySwitch;
    
    public:
        Obstacle() : StaticObject(), removedBySwitch(true) {
            sprite = '*';
            type = ObjectType::OBSTACLE;
        }
        
        Obstacle(const Point& pos, bool removable = true) 
            : StaticObject(pos, '*', ObjectType::OBSTACLE), removedBySwitch(removable) {}
        
        GameObject* clone() const override { return new Obstacle(*this); }
        const char* getName() const override { return "Obstacle"; }
        
        bool isBlocking() const override { return true; }
        bool onExplosion() override { return true; }
        
        bool isRemovedBySwitch() const { return removedBySwitch; }
        void setRemovedBySwitch(bool removable) { removedBySwitch = removable; }
    };
    