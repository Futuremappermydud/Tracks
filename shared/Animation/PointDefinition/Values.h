#pragma once

#include "Vector.h"
#include "sv/small_vector.h"
#include <memory>
#include <vector>

class BaseValues {
public:
  //TODO: Can I use a small_vector here?
  virtual std::vector<float> GetValues() = 0;
};

class BaseRotationValues {
public:
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
public:
  virtual void Update() = 0;
};

class QuaternionProviderValues: public UpdateableValues, public BaseRotationValues {
public:
  explicit QuaternionProviderValues(std::vector<float> values) : source(std::move(values)), values(std::vector<float>(3)) {}
  std::vector<float> GetValues() override;
  NEVector::Quaternion GetRotation() override;
  void Update() override;

private:
  NEVector::Quaternion rotation;
  std::vector<float> source;
  std::vector<float> values;
};

class PartialProviderValues: public UpdateableValues {
public:
  explicit PartialProviderValues(std::vector<float> values, std::vector<int> parts) : source(std::move(values)), parts(std::move(parts)), values(std::vector<float>(parts.size())) {}
  std::vector<float> GetValues() override;
  void Update() override;

private:
  std::vector<float> source;
  std::vector<float> values;
  std::vector<int> parts;
};

class SmoothRotationProvidersValues: public UpdateableValues {
public:
  explicit SmoothRotationProvidersValues(std::shared_ptr<BaseRotationValues> rotationValues, float mult) : rotationValues(rotationValues), mult(mult) {}
  std::vector<float> GetValues() override;
  void Update() override;

private:
  std::shared_ptr<BaseRotationValues> rotationValues;
  NEVector::Quaternion lastRotation;
  std::vector<float> values;
  float mult;
};

class SmoothProvidersValues: public UpdateableValues {
public:
  explicit SmoothProvidersValues(std::vector<float> source, float mult) : source(std::move(source)), mult(mult), values(std::vector<float>(source.size())) {}
  std::vector<float> GetValues() override;
  void Update() override;

private:
  std::vector<float> source;
  std::vector<float> values;
  float mult;
};