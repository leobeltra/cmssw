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

#define TIME 0.01

// Helper per verificare la presenza di un metodo
#define HAS_METHOD(method) \
template <typename T> \
class has_##method { \
private: \
    template <typename U> static auto test(int) -> decltype(std::declval<U>().method(), std::true_type()); \
    template <typename> static std::false_type test(...); \
public: \
    static constexpr bool value = std::is_same<decltype(test<T>(0)), std::true_type>::value; \
}; \

HAS_METHOD(description)
HAS_METHOD(someNumber)
HAS_METHOD(metadata)
HAS_METHOD(x)
HAS_METHOD(y)
HAS_METHOD(z)
HAS_METHOD(a)
HAS_METHOD(b)
HAS_METHOD(r)
HAS_METHOD(p_x)
HAS_METHOD(p_y)
HAS_METHOD(p_z)
HAS_METHOD(p_a)
HAS_METHOD(p_b)
HAS_METHOD(num)

GENERATE_SOA_LAYOUT(SoAHostDeviceLayoutTemplate,
                    SOA_COLUMN(double, x),
                    SOA_COLUMN(double, y),
                    SOA_COLUMN(double, z),
                    SOA_EIGEN_COLUMN(Eigen::Vector3d, a),
                    SOA_EIGEN_COLUMN(Eigen::Vector3d, b),
                    SOA_EIGEN_COLUMN(Eigen::Vector3d, r),
                    SOA_SCALAR(const char*, description),
                    SOA_SCALAR(uint32_t, someNumber))

using SoAHostDeviceLayout = SoAHostDeviceLayoutTemplate<cms::soa::CacheLineSize::IntelCPU>;
using SoAHostDeviceView = SoAHostDeviceLayout::View;
using SoAHostDeviceRangeCheckingView =
    SoAHostDeviceLayout::ViewTemplate<cms::soa::RestrictQualify::enabled, cms::soa::RangeChecking::enabled>;
using SoAHostDeviceConstView = SoAHostDeviceLayout::ConstView;

GENERATE_SOA_LAYOUT(SoAHostLayoutTemplate,
                    /*CustomSoAViewTemplate,*/
                    SOA_COLUMN(double, v_x),
                    SOA_COLUMN(double, v_y),
                    SOA_COLUMN(double, v_z),
                    SOA_EIGEN_COLUMN(Eigen::Vector3d, v_a),
                    SOA_EIGEN_COLUMN(Eigen::Vector3d, v_b),
                    SOA_EIGEN_COLUMN(Eigen::Vector3d, v_r),
                    SOA_SCALAR(const char*, v_description),
                    SOA_SCALAR(uint32_t, v_someNumber))

using SoAHostLayout = SoAHostLayoutTemplate<>;
using SoAHostLayoutView = SoAHostLayout::View;

GENERATE_SOA_LAYOUT(CustomSoATemplate,
                    /*CustomSoAViewTemplate,*/
                    SOA_COLUMN(double, p_x),
                    SOA_COLUMN(double, p_y),
                    SOA_COLUMN(double, p_z),
                    SOA_EIGEN_COLUMN(Eigen::Vector3d, p_a),
                    SOA_SCALAR(uint32_t, num),
                    SOA_EIGEN_COLUMN(Eigen::Vector3d, p_b))

using CustomSoA = CustomSoATemplate<>;
using CustomSoAView = CustomSoA::View;


template <typename View>
void printSoAView(View view) {
    std::cout << "SoAView:" << std::endl;

    if constexpr (has_description<View>::value) {
        std::cout << "description: " << view.description() << std::endl;
    }

    if constexpr (has_someNumber<View>::value) {
        std::cout << "someNumber: " << view.someNumber() << std::endl;
    }

    if constexpr (has_num<View>::value) {
        std::cout << "num: " << view.num() << std::endl;
    }

    if constexpr (has_metadata<View>::value) {
        for (auto i = 0; i < view.metadata().size(); ++i) {
            std::cout << "Element " << i << ": ";

            if constexpr (has_x<View>::value) {
                std::cout << "x = " << view.x()[i] << ", ";
            }

            if constexpr (has_y<View>::value) {
                std::cout << "y = " << view.y()[i] << ", ";
            }

            if constexpr (has_z<View>::value) {
                std::cout << "z = " << view.z()[i] << std::endl;
            }

            if constexpr (has_p_x<View>::value) {
                std::cout << "p_x = " << view.p_x()[i] << ", ";
            }

            if constexpr (has_p_y<View>::value) {
                std::cout << "p_y = " << view.p_y()[i] << ", ";
            }

            if constexpr (has_p_z<View>::value) {
                std::cout << "p_z = " << view.p_z()[i] << std::endl;
            }

            
            if constexpr (has_p_a<View>::value) {
                std::cout << "p_a = " << view[i].p_a().transpose() << ", ";
            }

            if constexpr (has_p_a<View>::value) {
                std::cout << "p_b = " << view[i].p_b().transpose() << ", ";
            }


            if constexpr (has_a<View>::value) {
                std::cout << "a = " << view[i].a().transpose() << ", ";
            }

            if constexpr (has_b<View>::value) {
                std::cout << "b = " << view[i].b().transpose() << ", ";
            }

            if constexpr (has_r<View>::value) {
                std::cout << "r = " << view[i].r().transpose() << std::endl;
            }
        }
    }
}

/*  
    TODO:
    - write my alpaka kernel and test in parallel
*/

int main () {
    //Memory definition for the host
    std::size_t numElements = 15;
    std::size_t hostDeviceSize = SoAHostDeviceLayout::computeDataSize(numElements);

    //Total number of columns
    static constexpr std::size_t number_columns = SoAHostDeviceLayout::computeColumnNumber();

    //Array of names
    std::array<const char*, number_columns> col_names = SoAHostDeviceLayout::generateColumnNames();
    std::cout << "Names: ";
    for (const char* i : col_names)
        std::cout << i << " ";
    std::cout << std::endl;

    //Memory allocation
    std::unique_ptr<std::byte, decltype(std::free) *> slBuffer{
      reinterpret_cast<std::byte *>(aligned_alloc(SoAHostDeviceLayout::alignment, hostDeviceSize)), std::free};
    std::unique_ptr<std::byte, decltype(std::free) *> slBuffer2{
      reinterpret_cast<std::byte *>(aligned_alloc(SoAHostLayout::alignment, hostDeviceSize)), std::free};  

    SoAHostDeviceLayout h_soa(slBuffer.get(), numElements);
    SoAHostLayout v_soa(slBuffer2.get(), numElements);

    //SoAView object to manipulate columns for the host
    SoAHostDeviceLayout::View h_soav{h_soa};
    SoAHostLayoutView v_soav{v_soa};

    //Filling the SoAs (different ways)
    h_soav.x()[0] = h_soav.y()[0] = h_soav.z()[0] = 0.;

    Eigen::Vector3d vec_a;
    vec_a.setConstant(1.);

    //Two different ways to fill the eigen columns, via eigen vectors or via [i,j] indexing
    for (size_t i = 1; i < numElements; i++) {
          h_soav.x()[i] = static_cast<double>(i);
          h_soav.y()[i] = static_cast<double>(i) * 2.0;
          h_soav.z()[i] = static_cast<double>(i) * 3.0;
          h_soav[i].a() = vec_a;
          for (size_t j = 0; j < 3; ++j) {
                h_soav[i].a()(j) = 0.1*i*(j+1);
                h_soav[i].b()(j) = 0.2*i*(j+1);
                h_soav[i].r()(j) = 0.3*i*(j+1); 
          }
    }
    h_soav.description() = "Example SoA";
    h_soav.someNumber() = 1; 

        for (size_t i = 0; i < numElements; i++) {
          v_soav.v_x()[i] = h_soav.x()[i] / TIME;
          v_soav.v_y()[i] = h_soav.y()[i] / TIME;
          v_soav.v_z()[i] = h_soav.z()[i] / TIME;
          for (size_t j = 0; j < 3; ++j) {
                v_soav[i].v_a()(j) = h_soav[i].a()(j) / TIME;
                v_soav[i].v_b()(j) = h_soav[i].b()(j) / TIME;
                v_soav[i].v_r()(j) = h_soav[i].r()(j) / TIME; 
          }
    }

    //Constructor by column pointers (for scalar values the reference is needed)
    CustomSoA d_soa(h_soa.metadata().size(), 
                              h_soav.x(), 
                              v_soav.v_x(),
                              v_soav.v_y(), 
                              h_soav.a(),
                              &(h_soav.someNumber()),
                              v_soav.v_b());

    // // Another way to use this feature: default constructor and function adding column by column
    // CustomSoA d_soa;

    // // d_soa.setData(h_soa.metadata().data());

    // d_soa.setColumn_p_x(h_soav.x(), h_soa.metadata().size());

    // d_soa.setColumn_p_y(v_soav.v_x(), v_soa.metadata().size());

    // d_soa.setColumn_p_a(h_soav.a(), h_soa.metadata().size());

    d_soa.soaToStreamInternal(std::cout);

    CustomSoA::View d_soav{d_soa};

    // This action modifies x()[3] too
    d_soav.p_x()[3] = 1000;

    printSoAView<CustomSoAView>(d_soav);

    printSoAView<SoAHostDeviceView>(h_soav);

    std::cout << "Indirizzo della memoria di d_soa: " << static_cast<void*>(d_soa.metadata().data()) << std::endl;
    std::cout << "Indirizzo della memoria di h_soa: " << static_cast<void*>(h_soa.metadata().data()) << std::endl;

    std::cout << "Indirizzo della memoria di x: " << h_soa.metadata().addressOf_x() << std::endl;
    std::cout << "Indirizzo della memoria di p_x: " << d_soa.metadata().addressOf_p_x() << std::endl;

    return 0;

}
