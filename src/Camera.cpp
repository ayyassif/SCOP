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
    Vec3 position, Vec3 up,
    float yaw, float pitch
)
    : MovementSpeed(SPEED),
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
    initialZoom(ZOOM),
    cameraModePressedLastFrame(false)
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

        Pitch = std::clamp(Pitch, -89.0f, 89.0f);
        updateCameraVectors();
        updateOrbitPosition();
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
        updateOrbitPosition();
        return;
    }

    Zoom -= yoffset;
    Zoom = std::clamp(Zoom, 1.0f, 45.0f);
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

void Camera::frameCameraOnVertices(const std::vector<Vertex>& vertices)
{
    if (vertices.empty())
        return;

    Vec3 minimum(
        std::numeric_limits<float>::max(),
        std::numeric_limits<float>::max(),
        std::numeric_limits<float>::max());
    Vec3 maximum(
        std::numeric_limits<float>::lowest(),
        std::numeric_limits<float>::lowest(),
        std::numeric_limits<float>::lowest());

    for (const Vertex& vertex : vertices)
    {
        minimum.x = std::min(minimum.x, vertex.position.x);
        minimum.y = std::min(minimum.y, vertex.position.y);
        minimum.z = std::min(minimum.z, vertex.position.z);
        maximum.x = std::max(maximum.x, vertex.position.x);
        maximum.y = std::max(maximum.y, vertex.position.y);
        maximum.z = std::max(maximum.z, vertex.position.z);
    }

    const Vec3 center = (minimum + maximum) * 0.5f;
    const float radius = (maximum - minimum).length() * 0.5f;
    FrameOn(center, radius);
}

bool Camera::cameraSwitch(const bool isSwitchPressed)
{
    const bool isSwitchJustPressed = isSwitchPressed && !cameraModePressedLastFrame;
    if (isSwitchJustPressed)
    {
        const CameraMode nextMode = GetMode() == CameraMode::Orbit
            ? CameraMode::Free
            : CameraMode::Orbit;
        SetMode(nextMode);
        Debugger::cameraModeChanged(nextMode == CameraMode::Free ? "Free" : "Orbit");
    }
    cameraModePressedLastFrame = isSwitchPressed;
    return isSwitchJustPressed;
}