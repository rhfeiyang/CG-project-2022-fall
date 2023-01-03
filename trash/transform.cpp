#include "transform.h"

Transform::Transform(const gVec3& position, const Quat& rotation, const gVec3& scale) : position(position), rotation(rotation), scale(scale) { }
Transform::Transform() : Transform(gVec3(0, 0, 0), Quat(1, 0, 0, 0), gVec3(1, 1, 1)) { }

gMat3 Transform::RotationMat() const {
  Float
    xx = rotation.x * rotation.x, yy = rotation.y * rotation.y, zz = rotation.z * rotation.z,
    xy = rotation.x * rotation.y, yz = rotation.y * rotation.z, zw = rotation.z * rotation.w, wx = rotation.w * rotation.x,
    xz = rotation.x * rotation.z, yw = rotation.y * rotation.w;
  return {
    1 - 2 * (yy + zz), 2 * (xy + zw), 2 * (xz - yw),
    2 * (xy - zw), 1 - 2 * (xx + zz), 2 * (yz + wx),
    2 * (xz + yw), 2 * (yz - wx), 1 - 2 * (xx + yy) };
}

gMat4 Transform::ModelMat() const {
  Float
    xx = rotation.x * rotation.x, yy = rotation.y * rotation.y, zz = rotation.z * rotation.z,
    xy = rotation.x * rotation.y, yz = rotation.y * rotation.z, zw = rotation.z * rotation.w, wx = rotation.w * rotation.x,
    xz = rotation.x * rotation.z, yw = rotation.y * rotation.w;
  return {
    scale.x * (1 - 2 * (yy + zz)), scale.x * (2 * (xy + zw)), scale.x * (2 * (xz - yw)), 0,
    scale.y * (2 * (xy - zw)), scale.y * (1 - 2 * (xx + zz)), scale.y * (2 * (yz + wx)), 0,
    scale.z * (2 * (xz + yw)), scale.z * (2 * (yz - wx)), scale.z * (1 - 2 * (xx + yy)), 0,
    position.x, position.y, position.z, 1 };
}

gMat4 Transform::ModelMatNotScaled() const {
  Float
    xx = rotation.x * rotation.x, yy = rotation.y * rotation.y, zz = rotation.z * rotation.z,
    xy = rotation.x * rotation.y, yz = rotation.y * rotation.z, zw = rotation.z * rotation.w, wx = rotation.w * rotation.x,
    xz = rotation.x * rotation.z, yw = rotation.y * rotation.w;
  return {
    1 - 2 * (yy + zz), 2 * (xy + zw), 2 * (xz - yw), 0,
    2 * (xy - zw), 1 - 2 * (xx + zz), 2 * (yz + wx), 0,
    2 * (xz + yw), 2 * (yz - wx), 1 - 2 * (xx + yy), 0,
    position.x, position.y, position.z, 1 };
}

gVec3 Transform::Right()   const { return rotation * gVec3( 1,  0,  0); }
gVec3 Transform::Left()    const { return rotation * gVec3(-1,  0,  0); }
gVec3 Transform::Up()      const { return rotation * gVec3( 0,  1,  0); }
gVec3 Transform::Down()    const { return rotation * gVec3( 0, -1,  0); }
gVec3 Transform::Forward() const { return rotation * gVec3( 0,  0, -1); }
gVec3 Transform::Back()    const { return rotation * gVec3( 0,  0,  1); }

void Transform::Rotate(const gVec3& axis, Float radians) {
  rotation = glm::quat_cast(glm::rotate(gMat4(one), radians, axis)) * rotation;
}

void Transform::RotateAround(const gVec3& axis, const gVec3& pivot, Float radians) {
  position = (glm::quat_cast(glm::rotate(gMat4(one), radians, axis)) * (position - pivot)) + pivot;
}

gVec3 Transform::TransformPoint(const gVec3& point) const {
  gMat4 model = ModelMat();
  return TransformPoint(point, model);
}

/*static*/ gVec3 Transform::TransformPoint(const gVec3& point, const gMat4& model) {
  return { model[0][0] * point.x + model[1][0] * point.y + model[2][0] * point.z + model[3][0],
           model[0][1] * point.x + model[1][1] * point.y + model[2][1] * point.z + model[3][1],
           model[0][2] * point.x + model[1][2] * point.y + model[2][2] * point.z + model[3][2] };
}
