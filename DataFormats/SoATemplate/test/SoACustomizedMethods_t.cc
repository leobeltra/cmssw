#include <Eigen/Core>
#include <Eigen/Dense>

#define CATCH_CONFIG_MAIN
#include <catch.hpp>

#include "DataFormats/SoATemplate/interface/SoALayout.h"

GENERATE_SOA_LAYOUT(SoAPositionTemplate,
                    SOA_COLUMN(float, x),
                    SOA_COLUMN(float, y),
                    SOA_COLUMN(float, z),
                    SOA_COLUMN(double, v_x),
                    SOA_COLUMN(double, v_y),
                    SOA_COLUMN(double, v_z),
                    SOA_METHODS(auto r() { 
                                    return x() * y() * z();
                                }
                                template <typename T1, typename T2>
                                auto time(T1 pos, T2 vel) {
                                    return pos / vel;
                                })
                    SOA_SCALAR(int, detectorType))

using SoAPosition = SoAPositionTemplate<>;
using SoAPositionView = SoAPosition::View;
using SoAPositionConstView = SoAPosition::ConstView;

int main {
    
}