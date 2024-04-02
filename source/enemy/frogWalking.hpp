#pragma once

#include "frog.hpp"

namespace stay
{
    class FrogWalking : public Frog::State
    {
        public:
            FrogWalking(Vector2 speed)
                : mSpeed{std::move(speed)}
            {}
            void enter() override
            {
                mElapsedTime = 0.F;
                context().body->setVelocity(mSpeed);
            }
            void exit() override
            {
                context().body->setVelocity(Vector2{});
            }
            bool update(float dt) override
            {
                mElapsedTime += dt;
                if (mElapsedTime >= 3.F) 
                {
                    requestPop();
                    requestPush(Frog::StateName::Idle);
                }
                return false;
            }
        private:
            Vector2 mSpeed{};
            float mElapsedTime{};
    };
} // namespace stay