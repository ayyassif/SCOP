#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "math/Math.hpp"

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

#define YAW         -90.0f
#define PITCH        0.0f
#define SPEED        2.5f
#define SENSITIVITY  0.1f
#define ZOOM         45.0f
#define SPRINT       5.0f

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
        float posX, float posY, float posZ,
        float upX, float upY, float upZ,
        float yaw = YAW, float pitch = PITCH
    );

    Camera(
        Vec3 position, Vec3 up,
        float yaw = YAW, float pitch = PITCH
    );

    Mat4 GetViewMatrix() const;
    void ProcessKeyboard(Camera_Movement direction, float deltaTime);
    void ProcessMouseMovement(Vec2 offset, bool constrainPitch = true);
    void ProcessMouseScroll(float yoffset);

private:
    bool isSprinting;

    void updateCameraVectors();
};

#endif