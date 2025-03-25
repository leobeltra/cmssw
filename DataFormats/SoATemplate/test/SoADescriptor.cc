#include <Eigen/Core>
#include <Eigen/Dense>

#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include <iostream>

#include "DataFormats/SoATemplate/interface/SoALayout.h"

GENERATE_SOA_LAYOUT(SoALayout,
                    SOA_COLUMN(double, x),
                    SOA_COLUMN(int, y),
                    SOA_COLUMN(float, z),
                    SOA_EIGEN_COLUMN(Eigen::Vector3d, a),
                    SOA_SCALAR(const char*, description))

using SoA = SoALayout<>;
using View = SoA::View;
using ConstView = SoA::ConstView;

TEST_CASE("SoADescriptor") {
  // number of elements
  const std::size_t elems = 10;

  // buffer size
  const std::size_t BufferSize = SoA::computeDataSize(elems);

  // memory buffer for the SoA
  std::unique_ptr<std::byte, decltype(std::free) *> buffer{
      reinterpret_cast<std::byte *>(aligned_alloc(SoA::alignment, BufferSize)), std::free};

  // SoA Layouts
  SoA soa{buffer.get(), elems};

  soa.soaToStreamInternal(std::cout);

  // SoA Views
  View view{soa};
  ConstView const_view{soa};

  // fill up
  for (size_t i = 0; i < elems; i++) {
    view.x()[i] = static_cast<double>(i);
    view.y()[i] = i * 2;
    view.z()[i] = static_cast<float>(i) * 3.0f;
    view.a(i)(0) = static_cast<double>(i) * 10.1;
    view.a(i)(1) = static_cast<double>(i) * 20.2;
    view.a(i)(2) = static_cast<double>(i) * 30.3;
  }
  view.description() = "SoA to describe";

  SoA::Descriptor descriptor(view);

//   std::span<double> x_span = descriptor.operator<0>();

  for (unsigned int i = 0; i < elems; i++) {
    REQUIRE(descriptor.data<0>()[i] == view.x()[i]);
    REQUIRE(descriptor.data<1>()[i] == view.y()[i]);
    REQUIRE(descriptor.data<2>()[i] == view.z()[i]);
    for (unsigned int j = 0; j < 3; j++) {
        REQUIRE(descriptor.data<3>()[i + j*16] == view[i].a()(j));
        // descriptor.data<3>()[i](j) 
    }    
  }
  REQUIRE(descriptor.data<4>()[0] == view.description());
}
