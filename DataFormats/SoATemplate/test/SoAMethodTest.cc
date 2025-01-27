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
                    SOA_METHOD(auto, r, return x() * x() + y() * y() + z() * z();))
                    // SOA_METHOD(auto, mean(const element& other), element center;
                    //                                              center.x() = (x() + other.x()) / 2;
                    //                                              center.y() = (y() + other.y()) / 2;
                    //                                              center.z() = (z() + other.z()) / 2;
                    //                                              return center;))

using SoA = SoALayout<>;
using SoAView = SoA::View;

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

  std::cout << "It works? " << view[0].r() << std::endl;
}