#include "player.h"

#include <iostream>

#include "input.h"

#include "constants.h"

#include "manager.h"
#include "context.h"


Player::Player() : Entity(), camera_(getPosition()), canJump_(false), noclip_(true)
{
    camera_.setFarPlane(World::renderDistance * 1.25f * 3);

    teleport(glm::vec3(520.5f, 102.0f, -320.5f));
    //teleport(glm::vec3(1.f, 0.0f, 0.0f));
}

void Player::update(float dt)
{
    Input& input = Input::Instance();
    Manager& chunks = Manager::Instance();
    const Uint8* keyboardState = SDL_GetKeyboardState(nullptr);
    float x, y;
    Uint32 mouseButtons = SDL_GetMouseState(&x, &y);

    // close
            //if (input.getKey(Key::Menu)) { Context::Instance().close(); }

    // build
    bool placing = (mouseButtons & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
    bool destroying = (mouseButtons & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;

    if (placing != destroying)
    {
        Manager::RaycastResult raycast = chunks.raycast(camera_.getPosition(), camera_.getForward(), INFINITY);
      //  std::cout << "balls\n";
        if (raycast.hit)
        {
            glm::ivec3 pos = raycast.block.first;

            if (destroying)
                chunks.setBlock(pos, { Block::AIR }, true); // set to air if destroying
            if (placing)
            {
                pos += glm::ivec3(Math::directionVectors[raycast.normal]);

                if (!Math::AABBCollision(getPosition(), getSize(), glm::vec3(pos) + glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f)))
                {
                    chunks.setBlock(pos, { Block::LOG }, true); // set to dirt if placing and not overlapping player
                }
            }
        }
    }

    
   
    // look
    glm::vec2 deltaMouse = input.getDeltaMouse() * 0.003f;
    if (keyboardState[SDL_SCANCODE_V] || (SDL_BUTTON(SDL_BUTTON_LEFT) && mouseButtons))
    {
    camera_.setYaw(camera_.getYaw() - deltaMouse.x);
    camera_.setPitch(camera_.getPitch() - deltaMouse.y);
    }
    
   //noclip_ = true; //if on then fly
    if (!noclip_)
    {
        glm::vec3 dir = glm::vec3(0.0f);

        // move
        if (keyboardState[SDL_SCANCODE_W])
            dir += camera_.getForwardAligned();
        if (keyboardState[SDL_SCANCODE_S])
            dir -= camera_.getForwardAligned();
        if (keyboardState[SDL_SCANCODE_A])
            dir -= camera_.getRight();
        if (keyboardState[SDL_SCANCODE_D])
            dir += camera_.getRight();
        
        // move
        if (dir != glm::vec3(0.0f))
            move(glm::normalize(dir) * (keyboardState[SDL_SCANCODE_LSHIFT] ? 10.f : 5.f) * dt);

        // jump
        if (canJump_ && keyboardState[SDL_SCANCODE_SPACE])
        {
            setVelocity(glm::vec3(0.0f, 7.f, 0.0f));
            canJump_ = false;
        }

        // physics
        Entity::update(dt);
    }
    else // noclip
    {
        setVelocity(glm::vec3(0.0f, 0.0f, 0.0f));

        glm::vec3 dir = glm::vec3(0.0f);

        // move
        if (keyboardState[SDL_SCANCODE_W]) 
            dir += camera_.getForwardAligned();
        if (keyboardState[SDL_SCANCODE_S])
            dir -= camera_.getForwardAligned();
        if (keyboardState[SDL_SCANCODE_A]) 
            dir -= camera_.getRight();
        if (keyboardState[SDL_SCANCODE_D]) 
            dir += camera_.getRight();
        if (keyboardState[SDL_SCANCODE_SPACE]) 
            dir += camera_.getUp();
        if (keyboardState[SDL_SCANCODE_LSHIFT]) 
            dir -= camera_.getUp();

        if (dir != glm::vec3(0.0f))
            teleport(getPosition() + glm::normalize(dir) * (keyboardState[SDL_SCANCODE_LCTRL] ? 100.f : 10.f) * dt);
        //teleport(getPosition() + glm::normalize(dir) * (input.getKey(Key::Run) ? 100.f : 10.f) * dt);
    }

    // update camera
    camera_.setPosition(getPosition() + glm::vec3(0.0f, getSize().y * 0.4f, 0.0f));
    camera_.setAspect(16.0f / 9.0f);

}

const Camera& Player::getCamera() const
{
    return camera_;
}

void Player::onCollision(std::pair<std::vector<BlockInfo>, Math::Direction> collision)
{
    if (collision.second == Math::DIRECTION_DOWN)
        canJump_ = true;

    Entity::onCollision(collision);
}