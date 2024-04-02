#pragma once

#include "frog.hpp"

namespace stay
{
    class FrogIdle : public Frog::State
    {
        public:
            FrogIdle(float rotateSpeed)
                : mRotateSpeed{rotateSpeed}
            {}
            void enter() override
            {
                mElapsedTime = 0.F;
                context().body->setAngularVelocity(mRotateSpeed);
            }
            void exit() override
            {
                context().body->setAngularVelocity(0.F);
            }
            bool update(float dt) override
            {
                mElapsedTime += dt;
                if (mElapsedTime >= 5.F) 
                {
                    requestPop();
                    requestPush(Frog::StateName::Walking);
                }
                return false;
            }
        private:
            float mRotateSpeed{};
            float mElapsedTime{};
    };
} // namespace stay