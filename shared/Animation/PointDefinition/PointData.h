#pragma once

#include "Animation/Easings.h"

template <typename T>
class BasePointData {
public:
  virtual Functions GetEasing() = 0;
  virtual bool HasBaseProvider() = 0;
  virtual T Point() = 0;
  virtual float GetTime() = 0;
};