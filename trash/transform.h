#pragma once

#include "common.h"

class Transform {
 public:
  gVec3 position;
  Quat rotation;
  gVec3 scale;

  Transform(const gVec3& position, const Quat& rotation, const gVec3& scale);
  Transform();

  Transform(const Transform&) = default;
  Transform(Transform&&) = default;
  Transform& operator=(const Transform&) = default;
  Transform& operator=(Transform&&) = default;
  ~Transform() = default;

  [[nodiscard]] gMat3 RotationMat() const;
  [[nodiscard]] gMat4 ModelMat() const;
  [[nodiscard]] gMat4 ModelMatNotScaled() const;

  [[nodiscard]] gVec3 Right()   const;
  [[nodiscard]] gVec3 Left()    const;
  [[nodiscard]] gVec3 Up()      const;
  [[nodiscard]] gVec3 Down()    const;
  [[nodiscard]] gVec3 Forward() const;
  [[nodiscard]] gVec3 Back()    const;

  void Rotate(const gVec3& axis, Float radians);
  void RotateAround(const gVec3& axis, const gVec3& pivot, Float radians);

  [[nodiscard]] gVec3 TransformPoint(const gVec3& point) const;
  static gVec3 TransformPoint(const gVec3& point, const gMat4& model);
};
