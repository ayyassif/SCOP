#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <vector>
#include "math/Math.hpp"
#include "mesh/Vertex.hpp"
#include "Debugger.hpp"

enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN,
    SPRINT_ON,
    SPRINT_OFF
};

enum class CameraMode
{
    Orbit,
    Free
};

constexpr float YAW         = -90.0f;
constexpr float PITCH       = 0.0f;
constexpr float SPEED       = 2.5f;
constexpr float SENSITIVITY = 0.1f;
constexpr float ZOOM        = 45.0f;
constexpr float SPRINT      = 5.0f;

class Camera
{
public:
    Vec3 Position;
    Vec3 Front;
    Vec3 Up;
    Vec3 Right;
    Vec3 WorldUp;

    float Yaw;
    float Pitch;

    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;
    
    Camera();

    Camera(
        Vec3 position, Vec3 up,
        float yaw = YAW, float pitch = PITCH
    );

    Mat4 GetViewMatrix() const;
    void FrameOn(const Vec3& center, float radius);
    void SetMode(CameraMode newMode);
    CameraMode GetMode() const;
    void ProcessKeyboard(Camera_Movement direction, float deltaTime);
    void ProcessMouseMovement(Vec2 offset, bool constrainPitch = true);
    void ProcessMouseScroll(float yoffset);
    void frameCameraOnVertices(const std::vector<Vertex>& vertices);
    bool cameraSwitch(const bool isSwitchPressed);

private:
    bool isSprinting;
    CameraMode mode;
    Vec3 orbitTarget;
    float orbitDistance;
    float maxOrbitDistance;
    Vec3 initialPosition;
    float initialYaw;
    float initialPitch;
    float initialZoom;
    bool  cameraModePressedLastFrame;

    void updateCameraVectors();
    void updateOrbitPosition();
    void resetToInitialView();

};

#endif