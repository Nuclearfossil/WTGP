#include "SceneNode.h"

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

void SceneNode::Update(double deltatime)
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
