#pragma once
#include <memory>
#include <numeric>
#include <utility>

#include "Animation/Easings.h"
#include "PointData.h"
#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"
#include "../Easings.h"
#include "../Track.h"
#include "Hash.h"
#include "UnityEngine/Color.hpp"

class BasePointDefinition {
  virtual int GetCount() = 0;
  virtual bool HasBaseProvider() = 0;
};

template <typename T>
constexpr void SearchIndex(sbo::small_vector<BasePointData<T>, 8> points, float time, int& l, int& r) {
  l = 0;
  r = points.size();

  while (l < r - 1) {
    int m = std::midpoint(l, r);
    float pointTime = points[m].time;

    if (pointTime < time) {
      l = m;
    } else {
      r = m;
    }
  }
}

template <typename T>
class PointDefinition: public BasePointDefinition {
public:
  explicit PointDefinition(rapidjson::Value const& value);

  int GetCount() override { return points.size(); }
  virtual bool HasBaseProvider() = 0;

  virtual T InterpolatePoints(sbo::small_vector<std::shared_ptr<BasePointData<T>>, 8> points, int l, int r, float time) = 0;
  T Interpolate(float time) { return Interpolate(time, std::ignore); }
  T Interpolate(float time, bool& last) {
    last = false;
    if (GetCount() == 0) {
      return T();
    }

    auto lastPoint = points[GetCount() - 1];
    if (lastPoint->GetTime() <= time) {
      last = true;
      return lastPoint->GetPoint();
    }

    auto firstPoint = points[0];
    if (firstPoint->GetTime() >= time) {
      return firstPoint->GetPoint();
    }

    int l, r;
    SearchIndex(points, time, l, r);
    auto pointL = points[l];
    auto pointR = points[r];

    float normalTime;
    float divisor = pointR->GetTime() - pointL->GetTime();
    if (divisor != 0.f) {
      normalTime = (time - pointL->GetTime()) / divisor;
    } else {
      normalTime = 0.f;
    }

    normalTime = Easings::Interpolate(normalTime, pointR->GetEasing());

    return InterpolatePoints(points, l, r, normalTime);
  }

private:
  
  virtual std::shared_ptr<Modifier<T>> CreateModifier(std::vector<std::shared_ptr<BaseValues>> values, std::vector<std::shared_ptr<Modifier<T>>> modifiers, Operation operation) = 0;
  virtual std::shared_ptr<BasePointData<T>> CreatePointData(std::vector<std::shared_ptr<BaseValues>> values, std::vector<std::string> flags, std::vector<std::shared_ptr<Modifier<T>>> modifiers, Functions easing) = 0;
  sbo::small_vector<std::shared_ptr<BasePointData<T>>, 8> points;
};

class PointDefinitionManager {
public:
  std::unordered_map<std::string, BasePointDefinition, TracksAD::string_hash, TracksAD::string_equal> pointData;

  void AddPoint(std::string const& pointDataName, std::shared_ptr<BasePointDefinition> pointData);
};

