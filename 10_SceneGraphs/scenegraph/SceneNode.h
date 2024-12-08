#pragma once

#include <directxmath.h>
#include <memory>
#include <vector>

#include "RenderBase.h"

class SceneNode : public std::enable_shared_from_this<SceneNode>
{
public:
    SceneNode() = default;
    ~SceneNode();

    void AddChild(std::shared_ptr<SceneNode> child);

    virtual void Update(float deltatime);

    virtual void Draw();

protected:
    std::weak_ptr<SceneNode> parent;
    std::vector<std::shared_ptr<SceneNode>> children;
    DirectX::XMMATRIX localTransform;
    DirectX::XMMATRIX worldTransform;

    std::weak_ptr<RenderBase> renderNode;

};