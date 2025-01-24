#include <Eigen/Core>
#include <Eigen/Dense>

#include <iostream>

// #include <alpaka/alpaka.hpp>

// #define CATCH_CONFIG_MAIN
// #include <catch.hpp>

// #include "DataFormats/Portable/interface/PortableCollection.h"
// #include "DataFormats/Portable/interface/PortableHostCollection.h"

#include "DataFormats/SoATemplate/interface/SoALayout.h"
#include "DataFormats/SoATemplate/interface/SoAView.h"

GENERATE_SOA_LAYOUT(SoALayout,
                    SOA_COLUMN(double, x),
                    SOA_EIGEN_COLUMN(Eigen::Vector3d, a),
                    SOA_METHOD(double, number))

using SoA = SoALayout<>;
using SoAView = SoA::View;

int main () {

}