#include "stdafx.h"

#include "level/Block.h"
#include "level/World.h"

World::World() :
    chunkWidth(),
    width(),
    numChunks(),
    chunks() {
}

World::~World() {
}

void World::Init(int chunksX, int chunksY, int chunksZ) {
    chunkWidth = Vector3I(chunksX, chunksY, chunksZ);
    width = chunkWidth * Chunk::DIM;

    numChunks = chunksX * chunksY * chunksZ;
    chunks = new Chunk*[numChunks]();
}

void World::Shutdown() {
    for (int i = 0; i < numChunks; i++) {
        delete chunks[i];
    }
    delete[] chunks;
    chunks = nullptr;
}

void World::Fill(int t) {
    VEC3_RANGE(chunkWidth) {
        int i = GetChunkIndex(p);
        chunks[i] = new Chunk();
        chunks[i]->Init(t, p.x, p.y, p.z);
    }
}

void World::Generate() {
    VEC3_RANGE_AB(Vector3I(16, 16, 16), width - Vector3I(17, 17, 17)) {
        SetBlock(p, Block::Stone);
    }
}

bool World::Intersects(const BoundingBox& bb) {
    VEC3_RANGE_AB(bb.Min().Floor(), bb.Max().Floor()) {
        if (GetBlock(p) == Block::Stone) {
            return true;
        }
    }
    return false;
}