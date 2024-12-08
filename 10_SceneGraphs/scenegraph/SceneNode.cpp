#include "SceneNode.h"

SceneNode::~SceneNode()
{
}

void SceneNode::AddChild(std::shared_ptr<SceneNode> child)
{
    children.push_back(child);
    child->parent = weak_from_this();
}

void SceneNode::Update(float deltatime)
{
    if (auto parentPtr = parent.lock())
    {
        worldTransform = DirectX::XMMatrixMultiply(parentPtr->worldTransform, localTransform);
    }
    else
    {
        worldTransform = localTransform;
    }

    for (auto& child : children)
    {
        child->Update(deltatime);
    }
}

void SceneNode::Draw()
{
}
