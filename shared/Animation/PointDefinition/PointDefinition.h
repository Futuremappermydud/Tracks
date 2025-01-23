#pragma once
#include <utility>

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
class PointDefinition: public BasePointDefinition {
public:
  explicit PointDefinition(rapidjson::Value const& value);

  int GetCount() override {
    return points.size();
  }

  virtual bool HasBaseProvider() = 0;

  T Interpolate(float time);
  T Interpolate(float time, bool& last);

private:
  sbo::small_vector<BasePointData<T>, 8> points;
};

class PointDefinitionManager {
public:
  std::unordered_map<std::string, BasePointDefinition, TracksAD::string_hash, TracksAD::string_equal> pointData;

  void AddPoint(std::string const& pointDataName, BasePointDefinition const& pointData);
  void AddPoint(std::string const& pointDataName, BasePointDefinition&& pointData);
};