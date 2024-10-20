#include "player.h"

#include <iostream>

//#include "input.h"

#include "constants.h"

#include "manager.h"

#include <entry/input.h>


Player::Player() : Entity(), camera_(getPosition()), canJump_(false), noclip_(false)
{
    camera_.setFarPlane(World::renderDistance * 1.25f);
    //inputInit();

    teleport(glm::vec3(520.5f, 102.0f, -320.5f));

    //teleport(glm::vec3(0, -10, 0));

}
glm::vec2 m_lastMousePos;

void Player::update(float dt, entry::MouseState state)
{
    //Input& input = Input::Instance();
    Manager& chunk = Manager::Instance();

    // close
    if (inputGetKeyState(entry::Key::Esc))
    {
        exit(0);
    }


    // build
    bool placing = state.m_buttons[entry::MouseButton::Left];
    bool destroying = state.m_buttons[entry::MouseButton::Right];
    if (placing != destroying)
    {
        Manager::RaycastResult raycast = chunk.raycast(camera_.getPosition(), camera_.getForward(), INFINITY);

        if (raycast.hit)
        {
            glm::ivec3 pos = raycast.block.first;

            if (destroying)
                chunk.setBlock(pos, { Block::AIR }, true); // set to air if destroying
            if (placing)
            {
                pos += glm::ivec3(Math::directionVectors[raycast.normal]);

                if (!Math::AABBCollision(getPosition(), getSize(), glm::vec3(pos) + glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f)))
                {
                    chunk.setBlock(pos, { Block::LOG }, true); // set to dirt if placing and not overlapping player
                }
            }
        }
    }

    // look
    //entry::setMouseLock(entry::kDefaultWindowHandle, true);
    glm::vec2 currentMousePos(state.m_mx, state.m_my);
    glm::vec2 dtMouse = currentMousePos - m_lastMousePos;
    glm::vec2 deltaMouse = dtMouse * 0.003f;
    m_lastMousePos = currentMousePos;
    if (inputGetKeyState(entry::Key::KeyV)) {
        camera_.setYaw(camera_.getYaw() - deltaMouse.x);
        camera_.setPitch(camera_.getPitch() - deltaMouse.y);
    }

    // move
    if (!noclip_)
    {
        glm::vec3 dir = glm::vec3(0.0f);

        // move input
        if (inputGetKeyState(entry::Key::KeyW)) { dir += camera_.getForwardAligned(); }
        if (inputGetKeyState(entry::Key::KeyS)) { dir -= camera_.getForwardAligned(); }
        if (inputGetKeyState(entry::Key::KeyA)) { dir -= camera_.getRight(); }
        if (inputGetKeyState(entry::Key::KeyD)) { dir += camera_.getRight(); }

        // move
        if (dir != glm::vec3(0.0f))
            move(glm::normalize(dir) * ((inputGetModifiersState() == entry::Modifier::LeftCtrl) ? 10.f : 5.f) * dt);


        // jump
        if (canJump_ && inputGetKeyState(entry::Key::Space))
        {
            setVelocity(glm::vec3(0.0f, 7.0f, 0.0f));
            canJump_ = false;
        }

        // physics
        Entity::update(dt);
    }
    else // noclip
    {
        setVelocity(glm::vec3(0.0f, 0.0f, 0.0f));

        glm::vec3 dir = glm::vec3(0.0f);
        if (inputGetKeyState(entry::Key::KeyW)) { dir += camera_.getForwardAligned(); }
        if (inputGetKeyState(entry::Key::KeyS)) { dir -= camera_.getForwardAligned(); }
        if (inputGetKeyState(entry::Key::KeyA)) { dir -= camera_.getRight(); }
        if (inputGetKeyState(entry::Key::KeyD)) { dir += camera_.getRight(); }
        if (inputGetKeyState(entry::Key::Space)) { dir += camera_.getUp(); }
        if (inputGetModifiersState() == entry::Modifier::LeftShift) { dir -= camera_.getUp(); }

        if (dir != glm::vec3(0.0f))
            teleport(getPosition() + glm::normalize(dir) * ((inputGetModifiersState() == entry::Modifier::LeftCtrl) ? 100.f : 10.f) * dt);

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