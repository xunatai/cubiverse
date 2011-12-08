#include "stdafx.h"

#include "System.h"
#include "graphics/Model.h"
#include "graphics/ModelFactory.h"
#include "graphics/WorldRenderer.h"

WorldRenderer::WorldRenderer() :
world(), visibleChunks(), shader(), texture() {
}

WorldRenderer::~WorldRenderer() {
}

int numBlocks = 4;

bool WorldRenderer::Init(World* w) {
    world = w;
    shader = new Shader();
    if (!shader->Init("res/block.c.v.glsl", "res/block.c.f.glsl")) {
        return false;
    }

    sf::Image image;

    if (!image.LoadFromFile("res/blocks2.png")) {
        return false;
    }

    texture = new Texture();
    texture->Init2DArray(numBlocks, image.GetWidth(), image.GetWidth(), (byte*)image.GetPixelsPtr());

    return true;
}

void WorldRenderer::Shutdown() {
    for (int i = 0; i < visibleChunks.size(); i++) {
        if (!visibleChunks[i]) continue;
        visibleChunks[i]->Shutdown();
        delete visibleChunks[i];
    }
    visibleChunks.clear();
    world = nullptr;
}

void WorldRenderer::ConstructVisibleChunks() {
    for (int i = 0; i < world->numChunks; i++) {
        VisibleChunk* vs = ConstructNewVisibleChunk(world->chunks[i]);
        if (vs) {
            visibleChunks.push_back(vs);
        }
    }
}

VisibleChunk* WorldRenderer::ConstructNewVisibleChunk(Chunk* c) {
    if (c == nullptr) return nullptr;
    ModelFactory mf = ConstructChunkModelData(c);
    if (mf.VertexCount() > 0) {
        VisibleChunk* vs = new VisibleChunk();
        vs->chunk = c;
        vs->UpdateModel(mf);
        return vs;
    }
    return nullptr;
}

ModelFactory WorldRenderer::ConstructChunkModelData(Chunk* c) {
    ModelFactory mf = ModelFactory();
    mf.shader = shader;
    mf.texture = texture;
    mf.topology = GL_TRIANGLE_STRIP;
    mf.AddAttribute("position", 3);
    mf.AddAttribute("color", 4);
    mf.AddAttribute("texcoord", 3);

    VEC3_RANGE_OFFSET(c->pos, Chunk::DIM_VEC) {
        int b = world->GetBlock(p);
        if (b == Block::Air) {
            continue;
        }
        if (world->GetBlock(p.x + 1, p.y, p.z) == Block::Air) {
            ConstructFace(mf, b, Vector3I::AXIS_X, p, p.x + 1, p.y, p.z, 0, 1, 2, 0.85);
        }
        if (world->GetBlock(p.x - 1, p.y, p.z) == Block::Air) {
            ConstructFace(mf, b, -Vector3I::AXIS_X, p, p.x, p.y, p.z, 0, 2, 1, 0.7);
        }
        if (world->GetBlock(p.x, p.y + 1, p.z) == Block::Air) {
            ConstructFace(mf, b, Vector3I::AXIS_Y, p, p.x, p.y + 1, p.z, 1, 2, 0, 0.8);
        }
        if (world->GetBlock(p.x, p.y - 1, p.z) == Block::Air) {
            ConstructFace(mf, b, -Vector3I::AXIS_Y, p, p.x, p.y, p.z, 1, 0, 2, 0.75);
        }
        if (world->GetBlock(p.x, p.y, p.z + 1) == Block::Air) {
            ConstructFace(mf, b, Vector3I::AXIS_Z, p, p.x, p.y, p.z + 1, 2, 0, 1, 1.0);
        }
        if (world->GetBlock(p.x, p.y, p.z - 1) == Block::Air) {
            ConstructFace(mf, b, -Vector3I::AXIS_Z, p, p.x, p.y, p.z, 2, 1, 0, 0.5);
        }
    }
    return mf;
}

int GetTextureIndex(int block, const Vector3I& side, const Vector3I& up) {
    switch (block) {
    case Block::Stone:
        return 0;
    case Block::Dirt:
        return 1;
    case Block::Grass:
        if (side == up) {
            return 2;
        }
        return 1;
    case Block::Test:
        return 3;
    }
}

void WorldRenderer::ConstructFace(ModelFactory& mf, int block, const Vector3I& side, const Vector3I& p, int x, int y, int z, int xi, int yi, int zi, double b) {
    Vector3F v = Vector3F(x, y, z);
    Vector3I up = System::world->GetUp(p.ToDouble().Offset(0.5) + side.ToDouble() / 2.0);
    ColorF c(b, b, b);

    float tz = (float)GetTextureIndex(block, side, up) / (float)numBlocks + 1.0f / ((float)numBlocks * 2.0f);

    // Degenerate.
    mf.Next().Set("position", v).Set("texcoord", 0, 0, tz);

    mf.Next().Set("position", v)
             .Set("color", c).Set("texcoord", 0, 0, tz);
    mf.Next().Set("position", v + Vector3F::AXIS[yi])
             .Set("color", c).Set("texcoord", 0, 1, tz);
    mf.Next().Set("position", v + Vector3F::AXIS[zi])
             .Set("color", c).Set("texcoord", 1, 0, tz);
    mf.Next().Set("position", v + Vector3F::AXIS[yi] + Vector3F::AXIS[zi])
             .Set("color", c).Set("texcoord", 1, 1, tz);

    // Degenerate.
    mf.Next().Set("position", v + Vector3F::AXIS[yi] + Vector3F::AXIS[zi]).Set("texcoord", 0, 0, tz);
}

void WorldRenderer::UpdateBlock(Vector3I p) {
    Chunk* c = System::world->GetChunk(p);
    if (c == nullptr) return;
    UpdateChunk(c);
    Chunk* c2;
    SIDES(
        if ((c2 = System::world->GetChunk(p + s)) != c) {
            UpdateChunk(c2);
        }
    );
}

void WorldRenderer::UpdateChunk(Chunk* c) {
    if (c == nullptr) return;
    int i;
    for (i = 0; i < visibleChunks.size(); i++) {
        VisibleChunk* vc = visibleChunks[i];

        if (!vc || vc->chunk != c) continue;

        ModelFactory mf = ConstructChunkModelData(c);
        vc->UpdateModel(mf);

        return;
    }
    if (i == visibleChunks.size()) {
        VisibleChunk* vs = ConstructNewVisibleChunk(c);
        if (vs) {
            visibleChunks.push_back(vs);
        }
    }
}

void WorldRenderer::Render() {
    for (int i = 0; i < visibleChunks.size(); i++) {
        if (!visibleChunks[i]) continue;
        visibleChunks[i]->model->Render();
    }
}