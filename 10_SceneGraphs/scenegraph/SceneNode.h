#pragma once

#include <directxmath.h>
#include <memory>
#include <vector>

#include "RenderBase.h"
#include "Shader.h"

class SceneNode : public std::enable_shared_from_this<SceneNode>
{
public:
    SceneNode() = default;
    ~SceneNode();

    void SetRenderable(std::weak_ptr<RenderBase> renderable, std::weak_ptr<Shader> shaderPtr);
    void AddChild(std::shared_ptr<SceneNode> child);
    void SetLocalTransform(const DirectX::XMMATRIX& local);

    virtual void Update(double deltatime);

    virtual void Draw(ID3D11DeviceContext* pD3DContext);

protected:
    std::weak_ptr<SceneNode> parent;
    std::vector<std::shared_ptr<SceneNode>> children;
    DirectX::XMMATRIX localTransform;
    DirectX::XMMATRIX worldTransform;

    std::weak_ptr<RenderBase> renderNode;
    std::weak_ptr<Shader> shader;
};