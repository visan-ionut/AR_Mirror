#pragma once

#include "components/simple_scene.h"
#include "components/transform.h"

#include <string>
#include "core/gpu/particle_effect.h"

namespace m2
{
    class Lab6 : public gfxc::SimpleScene
    {
    public:
        Lab6();
        ~Lab6();

        void Init() override;

    private:
        void CreateFramebuffer(int width, int height);
        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;

        unsigned int UploadCubeMapTexture(const std::string& pos_x, const std::string& pos_y, const std::string& pos_z, const std::string& neg_x, const std::string& neg_y, const std::string& neg_z);
        void LoadShader(const std::string& name, bool hasGeomtery = true);

        void OnInputUpdate(float deltaTime, int mods) override;
        void OnKeyPress(int key, int mods) override;
        void OnKeyRelease(int key, int mods) override;
        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
        void OnWindowResize(int width, int height) override;

    private:
        int cubeMapTextureID;
        float angle;
        unsigned int framebuffer_object;
        unsigned int color_texture;
        unsigned int depth_texture;
        unsigned int type;
        float carRotationAngle = -45.0f; // Current rotation angle
        float carRotationSpeed = 0.0f; // Speed of rotation (degrees per second)
        bool useContourShader;
        float unghiperspectiva;
        float totalTime;
    protected:
        glm::mat4 modelMatrix;
        float translateX, translateY, translateZ;
        float scaleX, scaleY, scaleZ;
        float angularStepOX, angularStepOY, angularStepOZ;
        GLenum polygonMode;
    };
}   // namespace m2
