#include <memory>
#include <tuple>
#include <Eigen/Core>
#include <Eigen/Dense>

#include <cassert>
#include <cstdint>
#include <iostream>
#include <string>
#include <cmath>

#include <alpaka/alpaka.hpp>

// #define CATCH_CONFIG_MAIN
// #include <catch.hpp>

#include "DataFormats/Portable/interface/PortableCollection.h"
#include "DataFormats/Portable/interface/PortableHostCollection.h"

#include "DataFormats/SoATemplate/interface/SoALayout.h"
#include "DataFormats/SoATemplate/interface/SoAView.h"
// #include "HeterogeneousCore/ROCmUtilities/interface/hipCheck.h"
// #include "HeterogeneousCore/ROCmUtilities/interface/requireDevices.h"
#include "FWCore/SOA/interface/Table.h"
#include "FWCore/SOA/interface/TableView.h"
#include "FWCore/SOA/interface/RowView.h"
#include "FWCore/SOA/interface/Column.h"
#include "FWCore/SOA/interface/TableItr.h"
#include "FWCore/SOA/interface/TableExaminer.h"


GENERATE_SOA_LAYOUT(SoAHostDeviceLayoutTemplate,
                    /*SoAHostDeviceViewTemplate,*/
                    // predefined static scalars
                    // size_t size;
                    // size_t alignment;

                    // columns: one value per element
                    SOA_COLUMN(double, x),
                    SOA_COLUMN(double, y),
                    SOA_COLUMN(double, z),
                    SOA_EIGEN_COLUMN(Eigen::Vector3d, a),
                    SOA_EIGEN_COLUMN(Eigen::Vector3d, b),
                    SOA_EIGEN_COLUMN(Eigen::Vector3d, r),
                    // scalars: one value for the whole structure
                    SOA_SCALAR(const char*, description),
                    SOA_SCALAR(uint32_t, someNumber))

using SoAHostDeviceLayout = SoAHostDeviceLayoutTemplate<>;
using SoAHostDeviceView = SoAHostDeviceLayout::View;
using SoAHostDeviceRangeCheckingView =
    SoAHostDeviceLayout::ViewTemplate<cms::soa::RestrictQualify::enabled, cms::soa::RangeChecking::enabled>;
using SoAHostDeviceConstView = SoAHostDeviceLayout::ConstView;


void printSoAView(SoAHostDeviceView view) {
    std::cout << "SoAHostDeviceView:" << std::endl;
    std::cout << "description: " << view.description() << std::endl;
    std::cout << "someNumber: " << view.someNumber() << std::endl;
    for (auto i = 0; i < view.metadata().size(); ++i) {
        std::cout << "Element " << i << ": ";
        std::cout << "x = " << view.x()[i] << ", ";
        std::cout << "y = " << view.y()[i] << ", ";
        std::cout << "z = " << view.z()[i] << std::endl;
    }
}


int main () {

    std::size_t numElements = 10;

    std::size_t dataSize = SoAHostDeviceLayout::computeDataSize(numElements);
      std::unique_ptr<std::byte, decltype(std::free) *> slBuffer{
      reinterpret_cast<std::byte *>(aligned_alloc(SoAHostDeviceLayout::alignment, dataSize)), std::free};

    SoAHostDeviceLayout soa(slBuffer.get(), numElements);

    SoAHostDeviceLayout::View soav{soa};
    SoAHostDeviceLayout::ConstView soacv{soa};
    // const auto &soa0 = soav[0];
    soav.x()[0] = soav.y()[0] = soav.z()[0] = 0.;

    for (size_t i = 1; i < numElements; ++i) {
          soav.x()[i] = static_cast<double>(i);
          soav.y()[i] = static_cast<double>(i) * 2.0;
          soav.z()[i] = static_cast<double>(i) * 3.0;
          // for (size_t j = 0; j < 3; ++j) {
          //       soav.a()(j)[i] = 0.1*i*j; 
          //       soav.b()(j)[i] = 0.2*i*j;
          //       soav.r()(j)[i] = 0.3*i*j; 
          // }

          // soav.a()[i] = Eigen::Vector3d(i, i + 1, i + 2);
          // soav.b()[i] = Eigen::Vector3d(i + 3, i + 4, i + 5);
          // soav.r()[i] = Eigen::Vector3d(i + 6, i + 7, i + 8);
    }
    soav.description() = "Example SoA";
    soav.someNumber() = 1; 

    printSoAView(soav);

    return 0;

}
