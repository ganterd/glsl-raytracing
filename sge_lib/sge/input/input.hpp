#pragma once

/* System includes */
#include <map>

/* External includes */
#include <glm/glm.hpp>

/* Internal includes */
#include <sge/configmanager/ConfigManager.hpp>
#include <sge/display/DisplayManager.hpp>

namespace SGE
{
	class Export Input
    {
    private:
        static Input* mInstance;
        static Input* Init();
        static Input* GetInstance();

    public:
        enum Key
        {
            A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X,Y,Z,Space,
            N0,N1,N2,N3,N4,N5,N6,N7,N8,N9,
            Left, Right, Up, Down,
            LeftControl
        };

        static void update();
        static bool isKeyPressed(Key k);
        static glm::vec2 mouseDelta(){ return mMouseDelta; };
        static glm::vec2 mousePosition(){ return mMousePosition; };

    protected:
        static glm::vec2 mMousePosition;
        static glm::vec2 mMouseDelta;

        virtual void InternalUpdate() = 0;
        virtual bool InternalKeyPressed(Key k) = 0;
    };
}
