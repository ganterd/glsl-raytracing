#include <sge/scene/bvh/bvh.hpp>

SGE::RT::BVH::BVH()
{
    mNumBins = 512;
    mRoot = nullptr;
    mTargetSceneRoot = nullptr;
}

SGE::RT::BVH::~BVH()
{

}

void SGE::RT::BVH::construct(Entity* scene)
{
    mExtractedTris = extractTris(scene);
    size_t numTris = mExtractedTris.size();
    Tri** triPointers = new Tri*[numTris];

    AABB rootCentroidAABB = AABB::infinity();
    for(unsigned int i = 0; i < numTris; ++i)
    {
        rootCentroidAABB.grow(mExtractedTris[i].mCentroid);
        triPointers[i] = &mExtractedTris[i];
    }

    mCurrentlyAllocatedNodes = 0;
    mAllocatedNodes = new Node[numTris * 2 - 1];

    deepestLevel = 0;
    numSplitNodes = 0;
    numLeafNodes = 0;

    std::cout << "Building BVH..." << std::endl;

    createBins();
    Timer constructionTimer;
    constructionTimer.start();
    mRoot = recursiveConstruct(triPointers, numTris, rootCentroidAABB, 0);
    constructionTimer.stop();
    freeBins();

    std::cout << "BVH Complete:" << std::endl;
    std::cout << "|- Time: " << constructionTimer.getTime() << "s" << std::endl;
    std::cout << "|- Deepest Level: " << deepestLevel << std::endl;
    std::cout << "|- Split Nodes: " << numSplitNodes << std::endl;
    std::cout << "|- Leaf Nodes: " << numLeafNodes << std::endl;
    std::cout << "|- Top AABB :" << mRoot->mAABB << std::endl;

    mTargetSceneRoot = scene;
}

std::vector<SGE::RT::Tri> SGE::RT::BVH::extractTris(SGE::Entity* n)
{
    std::vector<Tri> extractedTris;
    glm::mat4 m(1.0f);
    recursiveExtractTris(n, m, extractedTris);
    return extractedTris;
}

void SGE::RT::BVH::recursiveExtractTris(SGE::Entity* n, glm::mat4 m, std::vector<SGE::RT::Tri>& tris)
{
    m *= n->modelMat;

    for(int i = 0; i < n->meshes.size(); ++i)
    {
        Mesh* mesh = n->meshes[i];
        for(size_t triIdx = 0; triIdx < mesh->numTris; ++triIdx)
        {
            glm::vec4 verts[3];
            glm::vec2 uv[3];

            unsigned int* iboData = &mesh->iboData[triIdx * 3];
            for(size_t vertIdx = 0; vertIdx < 3; ++vertIdx)
            {
                size_t loc = iboData[vertIdx];
                verts[vertIdx] = glm::vec4(
                    mesh->vboData[loc * 3 + 0],
                    mesh->vboData[loc * 3 + 1],
                    mesh->vboData[loc * 3 + 2],
                    0.0f
                );

                if(mesh->uvData)
                {
                    uv[vertIdx] = glm::vec2(
                        mesh->uvData[loc * 2 + 0],
                        mesh->uvData[loc * 2 + 1]
                    );
                }
                else
                {
                    uv[vertIdx] = glm::vec2(0);
                }
            }
            Tri t(verts[0], verts[1], verts[2], uv[0], uv[1], uv[2]);
            t.mTriIndex = tris.size();
            tris.push_back(t);
        }
    }

    for(int i = 0; i < n->mChildren.size(); ++i)
        recursiveExtractTris(n->mChildren[i], m, tris);
}

SGE::RT::Node* SGE::RT::BVH::newNode()
{
    return &mAllocatedNodes[mCurrentlyAllocatedNodes++];
}

void SGE::RT::BVH::createBins()
{
    mBins = new Bin*[3];
    totalAABBRight = new AABB*[3];
    totalCentroidAABBRight = new AABB*[3];
    totalPrimitivesRight = new int*[3];

    for(int axis = 0; axis < 3; ++axis)
    {
        mBins[axis] = new Bin[mNumBins];
        totalAABBRight[axis] = new AABB[mNumBins];
        totalCentroidAABBRight[axis] = new AABB[mNumBins];
        totalPrimitivesRight[axis] = new int[mNumBins];
    }
}

void SGE::RT::BVH::freeBins()
{
    for(int axis = 0; axis < 3; ++axis)
    {
        delete[] mBins[axis];
        delete[] totalAABBRight[axis];
        delete[] totalCentroidAABBRight[axis];
        delete[] totalPrimitivesRight[axis];
    }

    delete[] mBins;
    delete[] totalAABBRight;
    delete[] totalCentroidAABBRight;
    delete[] totalPrimitivesRight;
}

SGE::RT::Node* SGE::RT::BVH::recursiveConstruct(
    Tri** tris,
    int numTris,
    const AABB& centroidAABB,
    int level
)
{
    if(level > deepestLevel)
        deepestLevel = level;

    if(numTris <= 4)
    {
        Node* n = &mAllocatedNodes[mCurrentlyAllocatedNodes];
        n->mIsLeaf = true;
        n->mNumTris = 0;
        n->mNodeIndex = mCurrentlyAllocatedNodes++;
        for(int i = 0; i < numTris; ++i)
        {
            n->mTris[(size_t)n->mNumTris++] = tris[i];
            n->mAABB.grow(tris[i]->mAABB);
        }
        numLeafNodes++;
        delete[] tris;
        return n;
    }


    glm::vec3 aabbSize = centroidAABB.mSize;
    Tri** trisLeft;
    Tri** trisRight;
    BestSplit best;
    if(aabbSize == glm::vec3(0.0f))
    {
        best.numPrimitives_L = numTris / 2;
        best.numPrimitives_R = numTris - best.numPrimitives_L;
        trisLeft = new Tri*[best.numPrimitives_L];
        trisRight = new Tri*[best.numPrimitives_R];

        for(int i = 0; i < best.numPrimitives_L; ++i)
            trisLeft[i] = tris[i];

        for(int i = 0; i < best.numPrimitives_R; ++i)
            trisRight[i] = tris[i + best.numPrimitives_L];

        best.centroidAABB_L = centroidAABB;
        best.centroidAABB_R = centroidAABB;
    }
    else
    {
        // Initialise the bins
        for(int axis = 0; axis < 3; ++axis)
        {
            float sizePerBin = aabbSize[axis] / (float)mNumBins;
            for(int b = 0; b < mNumBins; ++b)
            {
                mBins[axis][b].mTris.clear();
                mBins[axis][b].mTris.reserve(numTris / mNumBins);
                mBins[axis][b].mLeft = centroidAABB.mMax[axis] + (float)b * sizePerBin;
                mBins[axis][b].mRight = mBins[axis][b].mLeft + sizePerBin;
                mBins[axis][b].mAABB = AABB::infinity();
                mBins[axis][b].mCentroidsAABB = AABB::infinity();
            }
        }

        // Bin all primitives to all 3 axis bins in one pass
        for(int t = 0; t < numTris; ++t)
        {
            Tri* tri = tris[t];
            for(int axis = 0; axis < 3; ++axis)
            {
                int axisBin = 0;
                if(aabbSize[axis] > 0.0f)
                    axisBin = (float)mNumBins * (( 0.9999f * (tri->mCentroid[axis] - centroidAABB.mMin[axis])) / aabbSize[axis]);
                mBins[axis][axisBin].mTris.push_back(tri);
                mBins[axis][axisBin].mAABB.grow(tri->mAABB);
                mBins[axis][axisBin].mCentroidsAABB.grow(tri->mCentroid);
            }
        }

        // Find the best split
        best = findBestSplit();

        trisLeft = new Tri*[best.numPrimitives_L];
        trisRight = new Tri*[best.numPrimitives_R];

        int j = 0;
        for(int i = 0; i <= best.splitIndex; ++i)
        {
            for(unsigned int t = 0; t < mBins[best.axis][i].mTris.size(); ++t)
            {
                trisLeft[j++] = mBins[best.axis][i].mTris[t];
            }
        }

        j = 0;
        for(int i = best.splitIndex + 1; i < mNumBins; ++i)
        {
            for(unsigned int t = 0; t < mBins[best.axis][i].mTris.size(); ++t)
            {
                trisRight[j++] = mBins[best.axis][i].mTris[t];
            }
        }
    }

    // Delete the incoming array (save space when recursing)
    delete[] tris;

    Node* n = &mAllocatedNodes[mCurrentlyAllocatedNodes];
    n->mNodeIndex = mCurrentlyAllocatedNodes++;
    n->mIsLeaf = false;
    n->AssignLeft(recursiveConstruct(trisLeft, best.numPrimitives_L, best.centroidAABB_L, level + 1));
    n->AssignRight(recursiveConstruct(trisRight, best.numPrimitives_R, best.centroidAABB_R, level + 1));
    numSplitNodes++;
    return n;
}

SGE::RT::BestSplit SGE::RT::BVH::findBestSplit()
{
    BestSplit best;

    // Sweep from the right (on all axes) to generate the accumulated AABBs and costs


    AABB prevAABB[3];
    AABB prevCentroidAABB[3];
    int prevPrimitivesRight[3];

    prevAABB[0] = prevAABB[1] = prevAABB[2] = AABB::infinity();
    prevCentroidAABB[0] = prevCentroidAABB[1] = prevCentroidAABB[2] = AABB::infinity();
    prevPrimitivesRight[0] = prevPrimitivesRight[1] = prevPrimitivesRight[2] = 0;

    for(int b = mNumBins - 1; b >= 0; --b)
    {
        for(int ax = 0; ax < 3; ++ax)
        {
            prevAABB[ax].grow(mBins[ax][b].mAABB);
            totalAABBRight[ax][b] = prevAABB[ax];

            prevCentroidAABB[ax].grow(mBins[ax][b].mCentroidsAABB);
            totalCentroidAABBRight[ax][b] = prevCentroidAABB[ax];

            prevPrimitivesRight[ax] += mBins[ax][b].mTris.size();
            totalPrimitivesRight[ax][b] = prevPrimitivesRight[ax];
        }
    }

    // Sweep from left on all axes to find the best split
    int totalPrimitivesLeft[3];
    totalPrimitivesLeft[0] = totalPrimitivesLeft[1] = totalPrimitivesLeft[2] = 0;
    AABB totalAABBLeft[3];
    totalAABBLeft[0] = totalAABBLeft[1] = totalAABBLeft[2] = AABB::infinity();
    AABB totalCentroidAABBLeft[3];
    totalCentroidAABBLeft[0] = totalCentroidAABBLeft[1] = totalCentroidAABBLeft[2] = AABB::infinity();

    for(int ax = 0; ax < 3; ++ax)
    {
        totalAABBLeft[ax].grow(mBins[ax][0].mAABB);
        totalCentroidAABBLeft[ax].grow(mBins[ax][0].mCentroidsAABB);
        totalPrimitivesLeft[ax] += mBins[ax][0].mTris.size();
    }

    for(int b = 1; b < mNumBins - 1; ++b)
    {
        int bRight = b + 1;
        for(int ax = 0; ax < 3; ++ax)
        {
            // Accumulate the AABBs and primitives count of the left bins
            totalAABBLeft[ax].grow(mBins[ax][b].mAABB);
            totalCentroidAABBLeft[ax].grow(mBins[ax][b].mCentroidsAABB);
            totalPrimitivesLeft[ax] += mBins[ax][b].mTris.size();

            // Determine the cost of this split
            float costLeft = totalAABBLeft[ax].surfaceArea() * (float)totalPrimitivesLeft[ax];
            float costRight = totalAABBRight[ax][bRight].surfaceArea() * (float)totalPrimitivesRight[ax][bRight];
            float cost = costLeft + costRight;

            // Replace teh current best cose with this one
            if(cost < best.cost && totalPrimitivesLeft[ax] > 0 && totalPrimitivesRight[ax][bRight] > 0)
            {
                best.cost = cost;
                best.axis = ax;
                best.splitIndex = b;
                best.AABBLeft = totalAABBLeft[ax];
                best.AABBRight = totalAABBRight[ax][bRight];
                best.centroidAABB_L = totalCentroidAABBLeft[ax];
                best.centroidAABB_R = totalCentroidAABBRight[ax][bRight];
                best.numPrimitives_L = totalPrimitivesLeft[ax];
                best.numPrimitives_R = totalPrimitivesRight[ax][bRight];
            }
        }
    }

    return best;
}
