#pragma once

#include "TLogger.h"
#include "Values.h"
#include <algorithm>
#include <memory>
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
  T GetOriginalPoint() { return rawPoint.value_or(values.has_value() ? Convert(values) : T()); };

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
}