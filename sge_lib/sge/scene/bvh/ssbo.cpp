#include <sge/scene/bvh/ssbo.hpp>

SGE::RT::BVHSSBO::BVHSSBO()
{
    allocatedNodes = nullptr;
    allocatedTris = nullptr;
    attachedTrisSSBO = -1;
    attachedBVHNodesSSBO = -1;
}

void SGE::RT::BVHSSBO::toSSBO(BVH* bvh)
{
    /* 1: Extract nodes in simple format */
    if(allocatedNodes)
        delete[] allocatedNodes;
    allocatedNodes = new struct BVHSSBO::BVHNode[bvh->mCurrentlyAllocatedNodes];
    allocatedNodes[0].parentIdx = -1;
    for(int i = 0; i < bvh->mCurrentlyAllocatedNodes; ++i)
    {
        Node n = bvh->mAllocatedNodes[i];

        int nodeIdx = n.mNodeIndex;
        allocatedNodes[nodeIdx].aabbMin = glm::vec4(n.mAABB.mMin.x, n.mAABB.mMin.y, n.mAABB.mMin.z, 0.0f);
        allocatedNodes[nodeIdx].aabbMax = glm::vec4(n.mAABB.mMax.x, n.mAABB.mMax.y, n.mAABB.mMax.z, 0.0f);
        allocatedNodes[nodeIdx].isLeaf = n.mIsLeaf;
        if(n.mIsLeaf)
        {
            allocatedNodes[nodeIdx].leftIdx = n.mTris[0] == nullptr ? -1 : n.mTris[0]->mTriIndex;
            allocatedNodes[nodeIdx].rightIdx = n.mTris[1] == nullptr ? -1 : n.mTris[1]->mTriIndex;
            allocatedNodes[nodeIdx].tri2 = n.mTris[2] == nullptr ? -1 : n.mTris[2]->mTriIndex;
            allocatedNodes[nodeIdx].tri3 = n.mTris[3] == nullptr ? -1 : n.mTris[3]->mTriIndex;
        }
        else
        {
            allocatedNodes[nodeIdx].leftIdx = n.mLeft->mNodeIndex;
            allocatedNodes[nodeIdx].rightIdx = n.mRight->mNodeIndex;
            allocatedNodes[n.mLeft->mNodeIndex].parentIdx = i;
            allocatedNodes[n.mRight->mNodeIndex].parentIdx = i;
        }
    }

    /* 2: Extract tris in simple format */
    if(allocatedTris)
        delete[] allocatedTris;
    size_t totalTris = bvh->mExtractedTris.size();
    allocatedTris = new struct BVHSSBO::Tri[totalTris];
    for(int i = 0; i < totalTris; ++i)
    {
        RT::Tri tri = bvh->mExtractedTris[i];
        BVHSSBO::Tri newTri;
        newTri.v0 = tri.mV0;
        newTri.v1 = tri.mV1;
        newTri.v2 = tri.mV2;
        allocatedTris[tri.mTriIndex] = newTri;
    }

    /* 3: Transfer BVH nodes to SSBO */
    glGenBuffers(1, &attachedBVHNodesSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, attachedBVHNodesSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(struct BVHSSBO::BVHNode) * bvh->mCurrentlyAllocatedNodes, &allocatedNodes[0], GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, attachedBVHNodesSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    /* 4: Transfer Tris to SSBO */
    glGenBuffers(1, &attachedTrisSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, attachedTrisSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(struct BVHSSBO::Tri) * totalTris, &allocatedTris[0], GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, attachedTrisSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void SGE::RT::BVHSSBO::bind(GLuint nodesLoc, GLuint trisLoc)
{
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, attachedBVHNodesSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, nodesLoc, attachedBVHNodesSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, attachedTrisSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, trisLoc, attachedTrisSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void SGE::RT::BVHSSBO::unbind()
{
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void SGE::RT::BVHSSBO::freeSSBO()
{
    glDeleteBuffers(1, &attachedBVHNodesSSBO);
    glDeleteBuffers(1, &attachedTrisSSBO);
}
