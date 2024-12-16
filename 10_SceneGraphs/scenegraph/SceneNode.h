#pragma once

#include <directxmath.h>
#include <memory>
#include <array>
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

    void SetLocalRotation(float yaw, float pitch, float roll);
    void SetLocalTranslation(float x, float y, float z);
    void SetLocalScale(float x, float y, float z);

    std::array<float, 3> GetLocalRotation();
    std::array<float, 3> GetLocalTranslation();
    std::array<float, 3> GetLocalScale();

    std::array<float, 3> GetWorldRotationQuat();
    std::array<float, 3> GetWorldTranslation();
    std::array<float, 3> GetWorldScale();

    std::vector<std::shared_ptr<SceneNode>> GetChildren()
    {
        return children;
    }

    virtual void Update(double deltatime);

    virtual void Draw(ID3D11DeviceContext* pD3DContext);

    std::string name;

protected:
    std::weak_ptr<SceneNode> parent;
    std::vector<std::shared_ptr<SceneNode>> children;

    DirectX::XMVECTOR rotation = {0.0f, 0.0f, 0.0f};
    DirectX::XMVECTOR translation = {0.0f, 0.0f, 0.0f};
    DirectX::XMVECTOR scale = {1.0f, 1.0f, 1.0f};

    DirectX::XMVECTOR worldRotationQuat;
    DirectX::XMVECTOR worldTranslation;
    DirectX::XMVECTOR worldScale;

    DirectX::XMMATRIX localTransform;
    DirectX::XMMATRIX worldTransform;

    std::weak_ptr<RenderBase> renderNode;
    std::weak_ptr<Shader> shader;
};