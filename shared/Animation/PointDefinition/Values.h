#pragma once

#include "Vector.h"
#include "sv/small_vector.h"

class BaseValues {
  //TODO: Can I use a small_vector here?
  virtual std::vector<float> GetValues() = 0;
};

class BaseRotationValues {
  virtual NEVector::Quaternion GetRotation() = 0;
};

class StaticValues: public BaseValues {
public:
  explicit StaticValues(std::vector<float> values) : values(std::move(values)) {}
  std::vector<float> GetValues() override {
    return values;
  }

private:
  std::vector<float> values;
};

class BaseProviderValues: public BaseValues {
public:
  explicit BaseProviderValues(std::vector<float> values) : values(std::move(values)) {}
  std::vector<float> GetValues() override {
    return values;
  }

private:
  std::vector<float> values;
};

class UpdateableValues: public BaseValues {
  virtual void Update() = 0;
};

class QuaternionProviderValues: public UpdateableValues, public BaseRotationValues {

  std::vector<float> GetValues() override;
  void Update() override;
private:
  std::vector<float> source;
};