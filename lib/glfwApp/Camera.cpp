//
// Created by JeremyGuo on 2022/3/8.
//

#include "Camera.h"
#include <cmath>

namespace glfw {
#ifndef M_PI
#define M_PI acos(-1.0)
#endif

    static inline
    float Rad2Deg(const float rad) {
        return rad * (180.0f / M_PI);
    }

    static inline
    float Deg2Rad(const float deg) {
        return deg * (M_PI / 180.0f);
    }

    constexpr auto MatLookAt = glm::lookAtRH<float, glm::highp>;
    constexpr auto MatProjection = glm::perspectiveRH_ZO<float>;

    static inline
    quat QAngleAxis(const float angleRad, const vec3& axis) { return glm::angleAxis(angleRad, axis); }

    static inline
    vec3 QRotate(const quat& q, const vec3& v) { return glm::rotate(q, v); }

    using namespace glm;
    static const vec3 sCameraUp(0.0f, 1.0f, 0.0f);
    Camera::Camera()
            : mFovY(65.0f)
            , mNearZ(0.01f)
            , mFarZ(1000.0f)
            , mPosition(0.0f, 0.0f, 0.0f)
            , mDirection(0.0f, 0.0f, 1.0f)
    {
    }

    Camera::~Camera() {
    }

    void Camera::SetViewport(const Recti& viewport) {
        mViewport = viewport;
        this->MakeProjection();
    }

    void Camera::SetFovY(const float fovy) {
        mFovY = fovy;
        this->MakeProjection();
    }

    void Camera::SetViewPlanes(const float nearZ, const float farZ) {
        mNearZ = nearZ;
        mFarZ = farZ;
        this->MakeProjection();
    }

    void Camera::SetPosition(const vec3& pos) {
        mPosition = pos;
        this->MakeTransform();
    }

    void Camera::LookAt(const vec3& pos, const vec3& target) {
        mPosition = pos;
        mDirection = normalize(target - pos);

        this->MakeTransform();
    }

    void Camera::Move(const float side, const float direction) {
        vec3 cameraSide = normalize(cross(mDirection, sCameraUp));

        mPosition += cameraSide * side;
        mPosition += mDirection * direction;

        this->MakeTransform();
    }

    void Camera::Rotate(const float angleX, const float angleY) {
        vec3 side = cross(mDirection, sCameraUp);
        quat pitchQ = QAngleAxis(Deg2Rad(angleY), side);
        quat headingQ = QAngleAxis(Deg2Rad(angleX), sCameraUp);
        //add the two quaternions
        quat temp = normalize(pitchQ * headingQ);
        // finally rotate our direction
        mDirection = normalize(QRotate(temp, mDirection));

        this->MakeTransform();
    }

    float Camera::GetNearPlane() const {
        return mNearZ;
    }

    float Camera::GetFarPlane() const {
        return mFarZ;
    }

    float Camera::GetFovY() const {
        return mFovY;
    }

    const mat4& Camera::GetProjection() const {
        return mProjection;
    }

    const mat4& Camera::GetTransform() const {
        return mTransform;
    }

    const vec3& Camera::GetPosition() const {
        return mPosition;
    }

    const vec3& Camera::GetDirection() const {
        return mDirection;
    }

    const vec3 Camera::GetUp() const {
        return vec3(mTransform[0][1], mTransform[1][1], mTransform[2][1]);
    }

    const vec3 Camera::GetSide() const {
        return vec3(mTransform[0][0], mTransform[1][0], mTransform[2][0]);
    }

    void Camera::MakeProjection() {
        const float aspect = static_cast<float>(mViewport.right - mViewport.left) / static_cast<float>(mViewport.bottom - mViewport.top);
        mProjection = MatProjection(Deg2Rad(mFovY), aspect, mNearZ, mFarZ);
    }

    void Camera::MakeTransform() {
        mTransform = MatLookAt(mPosition, mPosition + mDirection, sCameraUp);
    }
}