#ifndef Position_SoA_h
#define Position_SoA_h

#include <Eigen/Core>
#include <Eigen/Dense>

#include "DataFormats/SoATemplate/interface/SoALayout.h"
#include "DataFormats/SoATemplate/interface/SoAView.h"

GENERATE_SOA_LAYOUT(PositionSoATemplate,
                    SOA_COLUMN(double, x),
                    SOA_COLUMN(double, y),
                    SOA_COLUMN(double, z),
                    SOA_SCALAR(int, detectorType))

using PositionSoA = PositionSoATemplate<>;
using PositionSoAView = PositionSoA::View;

#endif 
