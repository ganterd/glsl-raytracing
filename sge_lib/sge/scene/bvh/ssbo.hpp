#pragma once

#include <sge/graphics/ShaderManager.hpp>
#include <sge/scene/bvh/bvh.hpp>

namespace SGE
{
    namespace RT
    {
        class BVHSSBO
        {
        public:
            struct Tri{
                glm::vec4 v0;
                glm::vec4 v1;
                glm::vec4 v2;
                glm::vec4 pad;
            };

            struct BVHNode {
                glm::vec4 aabbMin;
                glm::vec4 aabbMax;
                int leftIdx;
                int rightIdx;
                int tri2;
                int tri3;
                int isLeaf;
                int parentIdx;
                int pad0_, pad1_;
            };

            GLuint attachedBVHNodesSSBO;
            GLuint attachedTrisSSBO;
            struct BVHSSBO::BVHNode* allocatedNodes;
            struct BVHSSBO::Tri* allocatedTris;

            BVHSSBO();

            void toSSBO(BVH* bvh);
            void bind(GLuint nodesLoc, GLuint trisLoc);
            void unbind();
            void freeSSBO();
        };
    }
}
