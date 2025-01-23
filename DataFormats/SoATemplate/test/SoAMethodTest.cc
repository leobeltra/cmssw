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

GENERATE_SOA_LAYOUT_NOT_CLOSED(SoALayout,
                                SOA_COLUMN(double, x),
                                SOA_EIGEN_COLUMN(Eigen::Vector3d, a))

SOA_METHOD(double, number(), return 10 * 0.1;)   

using SoA = SoALayout<>;
using View = SoA::View;                                

int main () {
  std::size_t numElements = 12;
  std::size_t size = SoA::computeDataSize(numElements);    
  std::unique_ptr<std::byte, decltype(std::free) *> slBuffer{
      reinterpret_cast<std::byte *>(aligned_alloc(SoA::alignment, size)), std::free};    
  SoA soa(slBuffer.get(), 10);
  double num = soa.number();

  std::cout << "It works? " << num << std::endl;
}