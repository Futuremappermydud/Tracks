#pragma once

#include "Animation/Easings.h"
#include "Modifiers.h"
#include "Values.h"
#include <memory>

template <typename T>
class BasePointData {
public:
  virtual Functions GetEasing() = 0;
  virtual bool HasBaseProvider() = 0;
  virtual T GetPoint() = 0;
  virtual float GetTime() = 0;
};

class FloatPointData: public Modifier<float>, public BasePointData<float> {
public:
  FloatPointData(std::optional<float> point, std::optional<std::vector<std::shared_ptr<BaseValues>>>& values, float time, std::vector<std::shared_ptr<Modifier<float>>> modifiers, Functions easing) : Modifier<float>(std::move(point), values, std::move(modifiers), Operation::opNone, 0), easing(easing), time(time) {}

  Functions GetEasing() override { return easing; }
  float GetTime() override { return time; }
private:
  Functions easing;
  float time;
};