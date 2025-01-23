#pragma once

#include "TLogger.h"
#include "Values.h"
#include "Vector.h"
#include <algorithm>
#include <memory>
#include <numeric>

#include "UnityEngine/Vector4.hpp"

enum Operation {
  opNone = 0,
  opAdd,
  opSub,
  opMul,
  opDiv,
};

template <typename T>
class Modifier {
public:
  Modifier(std::optional<T> point, std::optional<std::vector<std::shared_ptr<BaseValues>>> values, std::vector<std::shared_ptr<Modifier<T>>> modifiers, Operation operation, int arraySize) : rawPoint(std::move(point)), values(std::move(values)), modifiers(std::move(modifiers)), reusableArray(arraySize) {}

  virtual T GetPoint() = 0;
  virtual T Translate(std::vector<float> values);

  bool HasBaseProvider() { return values.has_value() || std::any_of(modifiers.begin(), modifiers.end(), [](std::shared_ptr<Modifier<T>> x) { return x->HasBaseProvider(); }); }
  Operation GetOperation() { return operation;}
  //TODO: Throw instead of T()?
  T GetOriginalPoint() { return rawPoint.value_or(values.has_value() ? Convert(values.value()) : T()); };

protected:
  std::vector<std::shared_ptr<Modifier<T>>> modifiers;

private:
  T Convert(std::vector<std::shared_ptr<BaseValues>> values) {
    int i = 0;
    for (auto baseValue : values) {
      for (auto value : baseValue->GetValues()) {
        reusableArray[i++] = value;
        if (i >= reusableArray.size()) {
          return Translate(reusableArray);
        }
      }
    }

    TLogger::Logger.error("Not enough values to fill the modifier.");
  }
  std::optional<std::vector<std::shared_ptr<BaseValues>>> values;
  std::vector<float> reusableArray;
  Operation operation;
  std::optional<T> rawPoint;
};


class FloatModifier: public Modifier<float> {
public:
  FloatModifier(std::optional<float> point, std::optional<std::vector<std::shared_ptr<BaseValues>>> values, std::vector<std::shared_ptr<Modifier<float>>> modifiers, Operation operation) : Modifier<float>(std::move(point), values, std::move(modifiers), operation, 1) {}

  float GetPoint() override {
    return std::accumulate(modifiers.begin(), modifiers.end(), GetOriginalPoint(), [](float acc, std::shared_ptr<Modifier<float>> x) {
      switch (x->GetOperation()) {
        case opAdd:
          return acc + x->GetPoint();
        case opSub:
          return acc - x->GetPoint();
        case opMul:
          return acc * x->GetPoint();
        case opDiv:
          return acc / x->GetPoint();
        default:
          TLogger::Logger.error("Unknown operation: {}", static_cast<int>(x->GetOperation()));
          return x->GetPoint();
      }
    });
  }

  float Translate(std::vector<float> values) override {
    return values[0];
  }
};

class Vector3Modifier: public Modifier<NEVector::Vector3> {
public:
  Vector3Modifier(std::optional<NEVector::Vector3> point, std::optional<std::vector<std::shared_ptr<BaseValues>>> values, std::vector<std::shared_ptr<Modifier<NEVector::Vector3>>> modifiers, Operation operation) : Modifier<NEVector::Vector3>(std::move(point), values, std::move(modifiers), operation, 3) {}

  NEVector::Vector3 GetPoint() override {
    return std::accumulate(modifiers.begin(), modifiers.end(), GetOriginalPoint(), [](NEVector::Vector3 acc, std::shared_ptr<Modifier<NEVector::Vector3>> x) {
      auto point = x->GetPoint();
      switch (x->GetOperation()) {
        case opAdd:
          return acc + point;
        case opSub:
          return acc - point;
        case opMul:
          return NEVector::Vector3(NEVector::Vector3::Scale(acc, point)); //TODO: reimpl in sombrero
        case opDiv:
          return NEVector::Vector3(acc.x / point.x, acc.y / point.y, acc.z / point.z);
        default:
          TLogger::Logger.error("Unknown operation: {}", static_cast<int>(x->GetOperation()));
          return point;
      }
    });
  }

  NEVector::Vector3 Translate(std::vector<float> values) override {
    return NEVector::Vector3(values[0], values[1], values[2]);
  }
};

class Vector4Modifier: public Modifier<NEVector::Vector4> {
public:
  Vector4Modifier(std::optional<NEVector::Vector4> point, std::optional<std::vector<std::shared_ptr<BaseValues>>> values, std::vector<std::shared_ptr<Modifier<NEVector::Vector4>>> modifiers, Operation operation) : Modifier<NEVector::Vector4>(std::move(point), values, std::move(modifiers), operation, 4) {}

  NEVector::Vector4 GetPoint() override {
    return std::accumulate(modifiers.begin(), modifiers.end(), GetOriginalPoint(), [](NEVector::Vector4 acc, std::shared_ptr<Modifier<NEVector::Vector4>> x) {
      auto point = x->GetPoint();
      //TODO: Add operator overloads for Vector4
      switch (x->GetOperation()) {
        case opAdd:
          return NEVector::Vector4(acc.x + point.x, acc.y + point.y, acc.z + point.z, acc.w + point.w);
        case opSub:
          return NEVector::Vector4(acc.x - point.x, acc.y - point.y, acc.z - point.z, acc.w - point.w);
        case opMul:
          return NEVector::Vector4(acc.x * point.x, acc.y * point.y, acc.z * point.z, acc.w * point.w); //TODO: what
        case opDiv:
          return NEVector::Vector4(acc.x / point.x, acc.y / point.y, acc.z / point.z, acc.w / point.w);
        default:
          TLogger::Logger.error("Unknown operation: {}", static_cast<int>(x->GetOperation()));
          return point;
      }
    });
  }

  NEVector::Vector4 Translate(std::vector<float> values) override {
    return NEVector::Vector4(values[0], values[1], values[2], values[3]);
  }
};

class QuaternionModifier: public Modifier<NEVector::Quaternion> {
public:
  QuaternionModifier(std::optional<NEVector::Quaternion> point, std::optional<std::vector<std::shared_ptr<BaseValues>>> values, std::vector<std::shared_ptr<Modifier<NEVector::Quaternion>>> modifiers, Operation operation) : Modifier<NEVector::Quaternion>(std::move(point), values, std::move(modifiers), operation, 3), reusableArray(3), values(values)  {}

  NEVector::Vector3 GetEulerPoint() {
    auto original = vectorPoint.value_or(TranslateEuler(values.value()));
    return std::accumulate(modifiers.begin(), modifiers.end(), original, [](NEVector::Vector3 acc, std::shared_ptr<Modifier<NEVector::Quaternion>> x) {
      auto quaternionModifier = std::dynamic_pointer_cast<QuaternionModifier>(x);
      auto point = quaternionModifier->GetEulerPoint();
      switch (x->GetOperation()) {
        case opAdd:
          return acc + point;
        case opSub:
          return acc - point;
        case opMul:
          return NEVector::Vector3(NEVector::Vector3::Scale(acc, point)); //TODO: reimpl in sombrero
        case opDiv:
          return NEVector::Vector3(acc.x / point.x, acc.y / point.y, acc.z / point.z);
        default:
          TLogger::Logger.error("Unknown operation: {}", static_cast<int>(x->GetOperation()));
          return point;
      }
    });
  }

  NEVector::Quaternion GetPoint() override {
    return std::accumulate(modifiers.begin(), modifiers.end(), GetOriginalPoint(), [](NEVector::Quaternion acc, std::shared_ptr<Modifier<NEVector::Quaternion>> x) {
      auto point = x->GetPoint();
      //TODO: Add operator overloads for Vector4
      switch (x->GetOperation()) {
        case opAdd:
          return NEVector::Quaternion(acc.x + point.x, acc.y + point.y, acc.z + point.z, acc.w + point.w);
        case opSub:
          return NEVector::Quaternion(acc.x - point.x, acc.y - point.y, acc.z - point.z, acc.w - point.w);
        case opMul:
          return NEVector::Quaternion(acc.x * point.x, acc.y * point.y, acc.z * point.z, acc.w * point.w); //TODO: what
        case opDiv:
          return NEVector::Quaternion(acc.x / point.x, acc.y / point.y, acc.z / point.z, acc.w / point.w);
        default:
          TLogger::Logger.error("Unknown operation: {}", static_cast<int>(x->GetOperation()));
          return point;
      }
    });
  }

  NEVector::Quaternion Translate(std::vector<float> values) override {
    return NEVector::Quaternion::Euler(values[0], values[1], values[2]);
  }
private:
  std::optional<std::vector<std::shared_ptr<BaseValues>>> values;
  std::optional<NEVector::Vector3> vectorPoint;
  std::vector<float> reusableArray;

  NEVector::Vector3 TranslateEuler(std::vector<std::shared_ptr<BaseValues>> values) {
    int i = 0;
    for (auto baseValue : values) {
      for (auto value : baseValue->GetValues()) {
        reusableArray[i++] = value;
        if (i >= reusableArray.size()) {
          return NEVector::Vector3(reusableArray[0], reusableArray[1], reusableArray[2]);
        }
      }
    }
    return NEVector::Vector3(reusableArray[0], reusableArray[1], reusableArray[2]);
  }
};