#include "Camera.hpp"

#include <algorithm>

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
    Position(position),
    WorldUp(up),
    Yaw(yaw), Pitch(pitch),
    isSprinting(false),
    mode(CameraMode::Orbit),
    orbitTarget(),
    orbitDistance(3.0f),
    maxOrbitDistance(15.0f),
    initialPosition(position),
    initialYaw(yaw),
    initialPitch(pitch),
    initialZoom(ZOOM)
{
    updateCameraVectors();
}

Mat4 Camera::GetViewMatrix() const
{
    return Mat4::lookAt(Position, Position + Front, Up);
}

void Camera::FrameOn(const Vec3& center, float radius)
{
    const float safeRadius = radius > 0.0f ? radius : 1.0f;
    const float halfFov = Math::radians(Zoom) * 0.5f;
    const float distance = safeRadius / std::sin(halfFov) * 1.2f;

    orbitTarget = center;
    orbitDistance = distance;
    maxOrbitDistance = distance * 3.0f;
    initialPosition = center + Vec3(0.0f, 0.0f, distance);
    initialYaw = -90.0f;
    initialPitch = 0.0f;
    initialZoom = Zoom;
    resetToInitialView();
}

void Camera::SetMode(CameraMode newMode)
{
    if (mode == newMode)
        return;

    mode = newMode;
    resetToInitialView();
}

CameraMode Camera::GetMode() const
{
    return mode;
}

void Camera::resetToInitialView()
{
    Position = initialPosition;
    Yaw = initialYaw;
    Pitch = initialPitch;
    Zoom = initialZoom;
    orbitDistance = (initialPosition - orbitTarget).length();
    updateCameraVectors();
}

void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime)
{
    switch (direction)
    {
        case SPRINT_ON:
            isSprinting = true;
            return;
        case SPRINT_OFF:
            isSprinting = false;
            return;
        default:
            break;
    }

    const float velocity = MovementSpeed * deltaTime
        * (isSprinting ? SPRINT : 1.0f);

    if (mode == CameraMode::Orbit)
    {
        switch (direction)
        {
            case FORWARD:
                orbitDistance = std::max(0.1f, orbitDistance - velocity);
                break;
            case BACKWARD:
                orbitDistance = std::min(maxOrbitDistance, orbitDistance + velocity);
                break;
            case LEFT:
                Yaw -= velocity * 20.0f;
                break;
            case RIGHT:
                Yaw += velocity * 20.0f;
                break;
            case UP:
                Pitch += velocity * 20.0f;
                break;
            case DOWN:
                Pitch -= velocity * 20.0f;
                break;
            default:
                return;
        }

        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;
        updateCameraVectors();
        Position = orbitTarget - Front * orbitDistance;
        return;
    }

    switch (direction)
    {
        case FORWARD:
            Position += Front * velocity;
            break;
        case BACKWARD:
            Position -= Front * velocity;
            break;
        case LEFT:
            Position -= Right * velocity;
            break;
        case RIGHT:
            Position += Right * velocity;
            break;
        case UP:
            Position += WorldUp * velocity;
            break;
        case DOWN:
            Position -= WorldUp * velocity;
            break;
        default:
            break;
    }
}

void Camera::ProcessMouseMovement(Vec2 offset, bool constrainPitch)
{
    offset.x *= MouseSensitivity;
    offset.y *= MouseSensitivity;

    Yaw   += offset.x;
    Pitch += offset.y;

    if (constrainPitch)
    {
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;
    }
    updateCameraVectors();
    if (mode == CameraMode::Orbit)
        updateOrbitPosition();
}

void Camera::ProcessMouseScroll(float yoffset)
{
    if (mode == CameraMode::Orbit)
    {
        orbitDistance = std::clamp(orbitDistance - yoffset * MovementSpeed,
                                   0.1f, maxOrbitDistance);
        Position = orbitTarget - Front * orbitDistance;
        return;
    }

    Zoom -= yoffset;
    if (Zoom < 1.0f)
        Zoom = 1.0f;
    if (Zoom > 45.0f)
        Zoom = 45.0f;
}

void Camera::updateCameraVectors()
{
    Vec3 front;
    const float rYaw = Math::radians(Yaw);
    const float rPitch = Math::radians(Pitch);

    front.x = cos(rYaw) * cos(rPitch);
    front.y = sin(rPitch);
    front.z = sin(rYaw) * cos(rPitch);
    Front   = front.normalize();

    Right = Front.cross(WorldUp).normalize();
    Up    = Right.cross(Front).normalize();
}

void Camera::updateOrbitPosition()
{
    Position = orbitTarget - Front * orbitDistance;
}