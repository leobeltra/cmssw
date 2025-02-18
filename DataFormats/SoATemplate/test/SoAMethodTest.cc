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
                    SOA_COLUMN(double, y),
                    SOA_COLUMN(double, z),
                    SOA_EIGEN_COLUMN(Eigen::Vector3d, a),
                    SOA_METHODS(template <typename T1, typename T2, typename T3>
                                         auto more_then_one_input(T1 x, T2 y, T3 z) { return x + y + z;}         
                                        auto r() { return x() * x() + y() * y() + z() * z(); }
                                        template <typename T6, typename T7, typename T8>
                                        auto mean_x(const element& other, T6 f, T7 h, T8 v) { x() += other.x();
                                                                             x() /= 2;
                                                                             return x(); }),
                    SOA_SCALAR(int, n)                                                                                        
                               )

// GENERATE_SOA_LAYOUT(SoALayout,
//                     SOA_COLUMN(double, x),
//                     SOA_COLUMN(double, y),
//                     SOA_COLUMN(double, z),
//                     SOA_EIGEN_COLUMN(Eigen::Vector3d, a),
//                     SOA_METHODS(FUNCTION(TEMPLATE(template <typename T1, typename T2, typename T3>)
//                                          auto more_then_one_input(T1 x, T2 y) { return x + y;})
//                                 FUNCTION(
//                                          auto r() { return x() * x() + y() * y() + z() * z(); })
//                                 // PROTECT(template <typename T1, typename T2, typename T3>) 
//                                 FUNCTION(
//                                          auto mean_x(const element& other), { x() += other.x();
//                                                                              x() /= 2;
//                                                                              return x(); })
//                                ))

// GENERATE_SOA_LAYOUT_WITH_METHODS(SoALayout,
//                                 SOA_MEMBERS(
//                                       SOA_COLUMN(double, x),
//                                       SOA_COLUMN(double, y),
//                                       SOA_COLUMN(double, z),
//                                       SOA_EIGEN_COLUMN(Eigen::Vector3d, a)),
//                                 SOA_METHODS(        
//                                       SOA_METHOD(auto, r(), { return x() * x() + y() * y() + z() * z(); } )
//                                       SOA_METHOD(void, zero(), { x() = y() = z() = 0; } )
//                                       SOA_METHOD(void, shift(), { lambda(*this);})
//                                     ) 
//                                 )                    

using SoA = SoALayout<>;
using SoAView = SoA::View;

  // FUNCTION(PROTECT(template <typename T1, typename T2, typename T3>)
  //          int const r() { return 2 + 1; })

int main () {
  std::size_t numElements = 12;
  std::size_t size = SoA::computeDataSize(numElements);    
  std::unique_ptr<std::byte, decltype(std::free) *> slBuffer{
      reinterpret_cast<std::byte *>(aligned_alloc(SoA::alignment, size)), std::free};    
  SoA soa(slBuffer.get(), numElements);
  SoAView view{soa};
  for (unsigned int i = 0; i < numElements; i++)
  {
    view.x()[i] = 10;
    view[i].y() = 10;
    view.z()[i] = 10;
  }

  // std::cout << "It works? " << view[0].r() << std::endl;
  // std::cout << "The mean of x direction between the first and the second element is: " << view[0].mean_x(view[1]) << std::endl;
}