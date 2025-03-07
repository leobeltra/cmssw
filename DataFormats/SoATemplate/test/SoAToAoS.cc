// #include <memory>
#include <tuple>
#include <Eigen/Core>
#include <Eigen/Dense>

#include <iostream>

// #include <alpaka/alpaka.hpp>

// #define CATCH_CONFIG_MAIN
// #include <catch.hpp>

#include "DataFormats/SoATemplate/interface/SoALayout.h"

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
HAS_METHOD(v_a)
HAS_METHOD(v_b)
HAS_METHOD(v_r)
HAS_METHOD(p_a)
HAS_METHOD(p_b)
HAS_METHOD(num)

GENERATE_SOA_LAYOUT(SoATemplate,
    SOA_COLUMN(double, x),
    SOA_COLUMN(double, y),
    SOA_COLUMN(double, z),
    SOA_EIGEN_COLUMN(Eigen::Vector3d, a),
    SOA_SCALAR(int, num))

using SoA = SoATemplate<>;
using SoAView = SoA::View;
using SoAConstView = SoA::ConstView;
using AoS = AoS_SoATemplate<>;

template <typename View>
void printSoAView(View view) {
    std::cout << "SoAView:" << std::endl;

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
                std::cout << "z = " << view.z()[i] << ", ";
            }

            if constexpr (has_a<View>::value) {
                std::cout << "a = " << view[i].a().transpose() << std::endl;
            }
        }
    }
}

int main() {
    std::size_t numElements = 12;
    std::size_t size = SoA::computeDataSize(numElements);

    std::unique_ptr<std::byte, decltype(std::free) *> slBuffer{
        reinterpret_cast<std::byte *>(aligned_alloc(SoA::alignment, size)), std::free};
        
    SoA soa(slBuffer.get(), numElements); 
    SoAView view{soa};
    SoAConstView{soa};
    
    for (size_t i = 0; i < numElements; i++) {
            view.x()[i] = static_cast<double>(i);
            view.y()[i] = static_cast<double>(i) * 2.0;
            view.z()[i] = static_cast<double>(i) * 3.0;
            Eigen::Vector3d::Scalar* vec = &view.a()[0];
            *vec = 11.11;
            *(vec + 1) = 6.2;
            *(vec + 2*sizeof(Eigen::Vector3d::Scalar)) = 12.12;
            *(vec + 4*sizeof(Eigen::Vector3d::Scalar)) = 13.13;
    }
    view.num() = 42;

    std::cout << sizeof(Eigen::Vector3d::Scalar) << std::endl;
    soa.soaToStreamInternal(std::cout);
    printSoAView<SoAView>(view);

    AoS aos(numElements);

    AoS soa_to_aos = soa.transpose();

    for (size_t i = 0; i < numElements; i++) {
        std::cout << "Element " << i << " :" << soa_to_aos[i].x << soa_to_aos[i].y << soa_to_aos[i].z << std::endl;
    }

    // Calcolare la differenza di indirizzo tra x e y per il primo elemento
    std::ptrdiff_t offset = reinterpret_cast<char*>(&soa_to_aos[0].y) - 
    reinterpret_cast<char*>(&soa_to_aos[0].x);

    std::cout << "Offset tra x e y: " << offset << " bytes\n"; 

    std::ptrdiff_t offsetsoa = reinterpret_cast<char*>(&view[0].y()) - 
    reinterpret_cast<char*>(&view[0].x());

    std::cout << "Offset tra x e y: " << offsetsoa << " bytes\n";
}