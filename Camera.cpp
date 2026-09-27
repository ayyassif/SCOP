#include "Camera.hpp"

Camera::Camera()
    : Camera(
        Vec3(0.0f, 0.0f, 0.0f),
        Vec3(0.0f, 1.0f, 0.0f),
        YAW,
        PITCH
    )
{}

Camera::Camera(
    float posX, float posY, float posZ,
    float upX, float upY, float upZ,
    float yaw, float pitch
)
    : Camera(
        Vec3(posX, posY, posZ),
        Vec3(upX, upY, upZ),
        yaw, pitch
    )
{}

Camera::Camera(
    Vec3 position, Vec3 up,
    float yaw, float pitch
)
    : Front(Vec3(0.0f, 0.0f, -1.0f)),
    MovementSpeed(SPEED),
    MouseSensitivity(SENSITIVITY),
    Zoom(ZOOM),
    position(position),
    WorldUp(up),
    Yaw(yaw), Pitch(pitch)
{
    updateCameraVectors();
}

Mat4 Camera::GetViewMatrix()
{
    return Mat4::lookAt(Position, Position + Front, Up);
}

void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime)
{
    float velocity = MovementSpeed * deltaTime;
    if (direction == SPRINT)
        velocity *= ACCELERATION;
    if (direction == FORWARD)
        Position += Front * velocity;
    if (direction == BACKWARD)
        Position -= Front * velocity;
    if (direction == LEFT)
        Position -= Right * velocity;
    if (direction == RIGHT)
        Position += Right * velocity;
    if (direction == UP)
        Position += WorldUp * velocity;
    if (direction == DOWN)
        Position -= WorldUp * velocity;
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
{
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw   += xoffset;
    Pitch += yoffset;

    if (constrainPitch)
    {
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;
    }
    updateCameraVectors();
}

void Camera::ProcessMouseScroll(float yoffset)
{
    Zoom -= (float)yoffset;
    if (Zoom < 1.0f)
        Zoom = 1.0f;
    if (Zoom > 45.0f)
        Zoom = 45.0f;
}

void Camera::updateCameraVectors()
{
    Vec3 front;
    const float radiantYaw = Math::radians(Yaw);
    const float radiantPitch = Math::radians(Pitch);

    front.x = cos(radiantYaw) * cos(radiantPitch);
    front.y = sin(radiantPitch);
    front.z = sin(radiantYaw) * cos(radiantPitch);
    Front   = front.normalize();

    Right = Front.cross(WorldUp).normalize();
    Up    = Right.cross(Front).normalize();
}