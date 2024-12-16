#include "SceneNode.h"

#include "mathutils.h"

SceneNode::~SceneNode()
{
}

void SceneNode::SetRenderable(std::weak_ptr<RenderBase> renderable, std::weak_ptr<Shader> shaderPtr)
{
    renderNode = renderable;
    shader = shaderPtr;
}

void SceneNode::SetLocalTransform(const DirectX::XMMATRIX& local)
{
    localTransform = local;
}

void SceneNode::AddChild(std::shared_ptr<SceneNode> child)
{
    children.push_back(child);
    child->parent = weak_from_this();
}

void SceneNode::SetLocalRotation(float yaw,float pitch,float roll)
{
    rotation.m128_f32[0] = yaw;
    rotation.m128_f32[1] = pitch;
    rotation.m128_f32[2] = roll;
}

void SceneNode::SetLocalTranslation(float x,float y,float z)
{
    translation.m128_f32[0] = x;
    translation.m128_f32[1] = y;
    translation.m128_f32[2] = z;
}

void SceneNode::SetLocalScale(float x,float y,float z)
{
    scale.m128_f32[0] = x;
    scale.m128_f32[1] = y;
    scale.m128_f32[2] = z;
}

std::array<float, 3> SceneNode::GetLocalRotation()
{
    return { rotation.m128_f32[0], rotation.m128_f32[1], rotation.m128_f32[2] };
}

std::array<float, 3> SceneNode::GetLocalTranslation()
{
    return { translation.m128_f32[0], translation.m128_f32[1], translation.m128_f32[2] };
}

std::array<float, 3> SceneNode::GetLocalScale()
{
    return { scale.m128_f32[0], scale.m128_f32[1], scale.m128_f32[2] };
}

std::array<float,3> SceneNode::GetWorldRotationQuat()
{
    return { worldRotationQuat.m128_f32[0], worldRotationQuat.m128_f32[1], worldRotationQuat.m128_f32[2] };
}

std::array<float,3> SceneNode::GetWorldTranslation()
{
    return { worldTranslation.m128_f32[0], worldTranslation.m128_f32[1], worldTranslation.m128_f32[2] };
}

std::array<float,3> SceneNode::GetWorldScale()
{
    return { worldScale.m128_f32[0], worldScale.m128_f32[1], worldScale.m128_f32[2] };
}

void SceneNode::Update(double deltatime)
{
    // update localTransform
    localTransform = DirectX::XMMatrixScaling(scale.m128_f32[0], scale.m128_f32[1], scale.m128_f32[2]) *
    DirectX::XMMatrixRotationY(degreesToRadians(rotation.m128_f32[1])) *
    DirectX::XMMatrixRotationX(degreesToRadians(rotation.m128_f32[0])) *
    DirectX::XMMatrixRotationZ(degreesToRadians(rotation.m128_f32[2])) *
    DirectX::XMMatrixTranslation(translation.m128_f32[0], translation.m128_f32[1], translation.m128_f32[2]);

    if (auto parentPtr = parent.lock())
    {
        worldTransform = DirectX::XMMatrixMultiply(parentPtr->worldTransform, localTransform);
    }
    else
    {
        worldTransform = localTransform;
    }

    DirectX::XMMatrixDecompose(&worldScale, &worldRotationQuat, &worldTranslation, worldTransform);

    for (auto& child : children)
    {
        child->Update(deltatime);
    }
}

void SceneNode::Draw(ID3D11DeviceContext* pD3DContext)
{
    if (auto sharedPtr = renderNode.lock())
    {
        if (auto shaderPtr = shader.lock())
        {
            sharedPtr->Draw(pD3DContext, shaderPtr, worldTransform);
        }
    }

    for (auto& child : children)
    {
        child->Draw(pD3DContext);
    }

}
