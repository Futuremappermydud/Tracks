#include "Animation/PointDefinition/PointDefinition.h"

#include <memory>
#include <utility>
#include <numeric>
#include "Animation/Track.h"
#include "Animation/Easings.h"
#include "TLogger.h"
#include "custom-json-data/shared/CJDLogger.h"
#include "sombrero/shared/HSBColor.hpp"

using namespace NEVector;

void PointDefinitionManager::AddPoint(std::string const& pointDataName, std::shared_ptr<BasePointDefinition> pointData) {
  if (this->pointData.contains(pointDataName)) {
    TLogger::Logger.error("Duplicate point definition name, {} could not be registered!", pointDataName.data());
  } else {
    this->pointData.try_emplace(pointDataName, pointData);
  }
}