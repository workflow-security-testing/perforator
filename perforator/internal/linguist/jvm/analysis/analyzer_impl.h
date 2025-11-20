#pragma once

#include "output.h"

#include "offset_registry.h"

namespace NPerforator::NLinguist::NJvm {

TJvmAnalysis ProcessOffsetRegistry(const TJvmMetadata& metadata);

}
