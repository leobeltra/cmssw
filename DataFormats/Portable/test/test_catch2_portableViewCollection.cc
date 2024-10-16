#include <catch.hpp>

#include "DataFormats/Portable/interface/PortableCollection.h"
#include "DataFormats/Portable/interface/PortableHostCollection.h"
#include "DataFormats/Portable/interface/PortableView.h"
#include "DataFormats/SoATemplate/interface/SoACommon.h"
#include "DataFormats/SoATemplate/interface/SoALayout.h"
#include "DataFormats/SoATemplate/interface/SoAView.h"

namespace {
  GENERATE_SOA_LAYOUT(TestLayout1, SOA_COLUMN(double, x), SOA_COLUMN(double, y), SOA_COLUMN(int32_t, id))
  GENERATE_SOA_LAYOUT(TestLayout2, SOA_COLUMN(double, v_x), SOA_COLUMN(double, v_y))

  GENERATE_SOA_LAYOUT(SoA, SOA_COLUMN(double, p_x), SOA_COLUMN(double, p_y))

  using TestSoA1 = TestLayout1<>;
  using TestSoA2 = TestLayout2<>;
  using TestSoA1View = TestSoA1::View;
  using TestSoA2View = TestSoA2::View;

  using TestSoA1Collection = PortableHostCollection<TestSoA1>;
  using TestSoA2Collection = PortableHostCollection<TestSoA2>;

  // using TestSoAView = PortableView<TestSoA1, TestSoA2>;

  constexpr auto s_tag = "[PortableView]";
}  // namespace

// This test is currently mostly about the code compiling
TEST_CASE("Use of PortableView<Colls...> on host code", s_tag) {
    std::size_t elems = 16;
    auto SoACollection_0 = std::make_unique<TestSoA1Collection>(elems, cms::alpakatools::host());
    auto SoACollection_1 = std::make_unique<TestSoA2Collection>(elems, cms::alpakatools::host());

    // // myView owns pointers to the columns corresponding to the input name
    // TestSoAView myView(TestSoA1Collection, {"x", "y"}, TestSoA2Collection, {"v_x"});

    // // Adding other columns
    // myView.addColumns(TestSoA1, {"id"}, TestSoA2, {"v_y"});
}