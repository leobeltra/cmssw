#include <memory>
#include <tuple>

#define CATCH_CONFIG_MAIN
#include <catch.hpp>

#include "DataFormats/SoATemplate/interface/SoALayout.h"

// clang-format off
GENERATE_SOA_LAYOUT(SoABlockTemplate,
  SOA_BLOCK_0(  
    SOA_COLUMN(float, x),
    SOA_COLUMN(float, y),
    SOA_COLUMN(float, z),
    SOA_COLUMN(float, t)),
  SOA_BLOCK_1(  
    SOA_SCALAR(size_t, scalar)))
// clang-format on


using SoABlock = SoABlockTemplate<>;

TEST_CASE("SoATemplate") {}