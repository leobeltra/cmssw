#include <memory>
#include <tuple>
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

using SoAHostDeviceLayout = SoAHostDeviceLayoutTemplate<cms::soa::CacheLineSize::IntelCPU>;
using SoAHostDeviceView = SoAHostDeviceLayout::View;
using SoAHostDeviceRangeCheckingView =
    SoAHostDeviceLayout::ViewTemplate<cms::soa::RestrictQualify::enabled, cms::soa::RangeChecking::enabled>;
using SoAHostDeviceConstView = SoAHostDeviceLayout::ConstView;

GENERATE_SOA_LAYOUT(SoADeviceOnlyLayoutTemplate,
                    /*SoADeviceOnlyViewTemplate,*/
                    SOA_COLUMN(uint16_t, color),
                    SOA_COLUMN(double, value),
                    SOA_COLUMN(double*, py),
                    SOA_COLUMN(uint32_t, count),
                    SOA_COLUMN(uint32_t, anotherCount))

using SoADeviceOnlyLayout = SoADeviceOnlyLayoutTemplate<>;
using SoADeviceOnlyView = SoADeviceOnlyLayout::View;

// A 1 to 1 view of the store (except for unsupported types).
GENERATE_SOA_VIEW(SoAFullDeviceConstViewTemplate,
                  SoAFullDeviceViewTemplate,
                  SOA_VIEW_LAYOUT_LIST(SOA_VIEW_LAYOUT(SoAHostDeviceLayout, soaHD),
                                       SOA_VIEW_LAYOUT(SoADeviceOnlyLayout, soaDO)),
                  SOA_VIEW_VALUE_LIST(SOA_VIEW_VALUE(soaHD, x),
                                      SOA_VIEW_VALUE(soaHD, y),
                                      SOA_VIEW_VALUE(soaHD, z),
                                      SOA_VIEW_VALUE(soaDO, color),
                                      SOA_VIEW_VALUE(soaDO, value),
                                      SOA_VIEW_VALUE(soaDO, py),
                                      SOA_VIEW_VALUE(soaDO, count),
                                      SOA_VIEW_VALUE(soaDO, anotherCount),
                                      SOA_VIEW_VALUE(soaHD, description),
                                      SOA_VIEW_VALUE(soaHD, someNumber)))

using SoAFullDeviceView =
    SoAFullDeviceViewTemplate<cms::soa::CacheLineSize::NvidiaGPU, cms::soa::AlignmentEnforcement::enforced>;

void printSoAView(SoAHostDeviceView view) {
    std::cout << "SoAHostDeviceView:" << std::endl;
    std::cout << "description: " << view.description() << std::endl;
    std::cout << "someNumber: " << view.someNumber() << std::endl;
    for (auto i = 0; i < view.metadata().size(); ++i) {
        std::cout << "Element " << i << ": ";
        std::cout << "x = " << view.x()[i] << ", ";
        std::cout << "y = " << view.y()[i] << ", ";
        std::cout << "z = " << view.z()[i] << std::endl;
        std::cout << "a = " << view[i].a().transpose() << ", ";
        std::cout << "b = " << view[i].b().transpose() << ", ";
        std::cout << "r = " << view[i].r().transpose() << std::endl;
    }
}

/*  
    TODO:
    - write my alpaka kernel and test in parallel
    - use some producers and test the functioning
    - write my total and cpu time (maybe other metrics) to evaluate efficiency
*/

int main () {
    std::size_t numElements = 16;

    // Enables parallel execution
    // PortableCollection<SoAHostDeviceLayout, alpaka::DevCpu> coll(numElements, cms::alpakatools::host());

    //Memory definition for the host
    std::size_t hostDeviceSize = SoAHostDeviceLayout::computeDataSize(numElements);
    static constexpr std::size_t number_columns = SoAHostDeviceLayout::computeColumnNumber();
    std::array<const char*, number_columns> col_names = SoAHostDeviceLayout::generateColumnNames();
    std::cout << "Names: ";
    for (const char* i : col_names)
        std::cout << i << " ";
    std::cout << std::endl;
    std::unique_ptr<std::byte, decltype(std::free) *> slBuffer{
      reinterpret_cast<std::byte *>(aligned_alloc(SoAHostDeviceLayout::alignment, hostDeviceSize)), std::free};
    //Memory allocation for the host
    SoAHostDeviceLayout h_soa(slBuffer.get(), numElements);

    //SoAView object to manipulate columns for the host
    SoAHostDeviceLayout::View h_soav{h_soa};
    SoAHostDeviceLayout::ConstView soacv{h_soa};

    h_soav.x()[0] = h_soav.y()[0] = h_soav.z()[0] = 0.;

    Eigen::Vector3d vec_a;
    vec_a.setConstant(1.);


    // Eigen::Vector3d vec_b;
    // vec_b.setConstant(2.);


    // Eigen::Vector3d vec_r;
    // vec_r.setConstant(3.);

    //Two different ways to fill the eigen columns, via eigen vectors or via [i,j] indexing
    for (size_t i = 1; i < numElements; i++) {
          h_soav.x()[i] = static_cast<double>(i);
          h_soav.y()[i] = static_cast<double>(i) * 2.0;
          h_soav.z()[i] = static_cast<double>(i) * 3.0;
          h_soav[i].a() = vec_a;
          for (size_t j = 0; j < 3; ++j) {
                //h_soav[i].a()(j) = 0.1*i*(j+1);
                h_soav[i].b()(j) = 0.2*i*(j+1);
                h_soav[i].r()(j) = 0.3*i*(j+1); 
          }
    }
    h_soav.description() = "Example SoA";
    h_soav.someNumber() = 1; 

    printSoAView(h_soav);

    std::cout << std::endl << "Memory information: " << std::endl << std::endl;

    h_soa.soaToStreamInternal(std::cout);

    // Push to device
    //cudaCheck(cudaMemcpyAsync(d_buf, h_buf, hostDeviceSize, cudaMemcpyDefault, stream));

    return 0;

}
