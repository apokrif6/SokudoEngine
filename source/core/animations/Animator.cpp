#include "Animator.h"

void Core::Animations::Animator::update(Renderer::Mesh* mesh) { updateBonesTransform(mesh); }

void Core::Animations::Animator::updateBonesTransform(Renderer::Mesh* mesh)
{
    for (Renderer::Primitive& primitive : mesh->getPrimitives())
    {
        Animations::BonesInfo& bonesInfo = primitive.getBonesInfo();
        size_t bonesInfoSize = bonesInfo.bones.size();

        bonesInfo.finalTransforms.resize(bonesInfoSize);

        for (int i = 0; i < bonesInfoSize; i++)
        {
            bonesInfo.finalTransforms[i] = bonesInfo.bones[i].finalTransform;
        }
    }
}
