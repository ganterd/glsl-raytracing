#pragma once

#include <sge/scene/bvh/tri.hpp>
#include <sge/scene/bvh/aabb.hpp>

namespace SGE
{
    namespace RT
    {
        class Node
        {
        public:
            AABB mAABB = AABB::infinity();
            bool mIsLeaf = false;
            Node* mLeft;
            Node* mRight;
            int mNodeIndex;

            char mNumTris;
            Tri* mTris[4];

            Node()
            {
                mTris[0] = nullptr;
                mTris[1] = nullptr;
                mTris[2] = nullptr;
                mTris[3] = nullptr;
            }

            void AssignLeft(struct Node* n)
            {
                mLeft = n;
                mAABB.grow(n->mAABB);
            }

            void AssignRight(struct Node* n)
            {
                mRight = n;
                mAABB.grow(n->mAABB);
            }
        };
    }
}
