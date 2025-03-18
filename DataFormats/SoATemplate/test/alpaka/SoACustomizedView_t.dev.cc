#include <Eigen/Core>
#include <Eigen/Dense>
#include <alpaka/alpaka.hpp>  

#include <ostream>

#define CATCH_CONFIG_MAIN
#include <catch.hpp>

#include "DataFormats/SoATemplate/interface/SoALayout.h"
#include "HeterogeneousCore/AlpakaInterface/interface/config.h"
#include "HeterogeneousCore/AlpakaInterface/interface/memory.h"
#include "HeterogeneousCore/AlpakaInterface/interface/workdivision.h"

#define TIME 0.01

using namespace ALPAKA_ACCELERATOR_NAMESPACE;

GENERATE_SOA_LAYOUT(SoAPositionTemplate,
                    SOA_COLUMN(float, x),
                    SOA_COLUMN(float, y),
                    SOA_COLUMN(float, z),
                    SOA_SCALAR(int, detectorType))

using SoAPosition = SoAPositionTemplate<>;
using SoAPositionView = SoAPosition::View;
using SoAPositionConstView = SoAPosition::ConstView;

GENERATE_SOA_LAYOUT(SoAPCATemplate,
                    SOA_COLUMN(float, eigenvalues),
                    SOA_COLUMN(float, eigenvector_1),
                    SOA_COLUMN(float, eigenvector_2),
                    SOA_COLUMN(float, eigenvector_3),
                    SOA_EIGEN_COLUMN(Eigen::Vector3d, candidateDirection))

using SoAPCA = SoAPCATemplate<>;
using SoAPCAView = SoAPCA::View;
using SoAPCAConstView = SoAPCA::ConstView;

GENERATE_SOA_LAYOUT(CustomizedSoATemplate,
                    SOA_COLUMN(float, x),
                    SOA_COLUMN(float, y),
                    SOA_COLUMN(float, z),
                    SOA_EIGEN_COLUMN(Eigen::Vector3d, candidateDirection),
                    SOA_SCALAR(int, detectorType))

using CustomizedSoA = CustomizedSoATemplate<cms::soa::CacheLineSize::IntelCPU>;
using CustomizedSoAView = CustomizedSoA::View;
using CustomizedSoAConstView = CustomizedSoA::ConstView;

// Kernel Alpaka for filling the SoA
struct FillSoA
{
  template <typename TAcc, typename PositionView, typename PCAView>
  ALPAKA_FN_ACC void operator()(TAcc const &acc, PositionView positionView, PCAView pcaView) const {

    if (cms::alpakatools::once_per_grid(acc)) 
      positionView.detectorType() = 1;

    for (auto local_idx : cms::alpakatools::uniform_elements(acc, positionView.metadata().size())) {
      positionView.x()[local_idx] = static_cast<float>(local_idx);
      positionView.y()[local_idx] = static_cast<float>(local_idx) * 2.0f;
      positionView.z()[local_idx] = static_cast<float>(local_idx) * 3.0f;

      pcaView.eigenvector_1()[local_idx] = positionView.x()[local_idx] / TIME;
      pcaView.eigenvector_2()[local_idx] = positionView.y()[local_idx] / TIME;
      pcaView.eigenvector_3()[local_idx] = positionView.z()[local_idx] / TIME;
      pcaView[local_idx].candidateDirection()(0) = positionView[local_idx].x() / TIME;
      pcaView[local_idx].candidateDirection()(1) = positionView[local_idx].y() / TIME;
      pcaView[local_idx].candidateDirection()(2) = positionView[local_idx].z() / TIME;
    }
  }
};

TEST_CASE("SoACustomizedView") {

  auto const& devices = cms::alpakatools::devices<Platform>();
  if (devices.empty()) {
    FAIL("No devices available for the " EDM_STRINGIZE(ALPAKA_ACCELERATOR_NAMESPACE) " backend, "
        "the test will be skipped.");
  }

  auto devHost = alpaka::getDevByIdx(alpaka::PlatformCpu{}, 0u);

  for (auto const& device : cms::alpakatools::devices<Platform>()) {

    std::cout << "Running on " << alpaka::getName(device) << std::endl;

    Queue queue(device);

    // common number of elements for the SoAs
    const std::size_t elems = 17;

    // buffer sizes
    const std::size_t positionBufferSize = SoAPosition::computeDataSize(elems);
    const std::size_t pcaBufferSize = SoAPCA::computeDataSize(elems);

    // memory buffer for the SoA of positions
    auto bufferPos = alpaka::allocBuf<std::byte, size_t>(device, positionBufferSize);  
  
    // memory buffer for the SoA of the PCA
    auto bufferPCA = alpaka::allocBuf<std::byte, size_t>(device, pcaBufferSize);   

    // SoA Layouts
    SoAPosition position{alpaka::getPtrNative(bufferPos), elems};
    SoAPCA pca{alpaka::getPtrNative(bufferPCA), elems};

    // SoA Views
    SoAPositionView positionView{position};
    SoAPositionConstView positionConstView{position};
    SoAPCAView pcaView{pca};
    SoAPCAConstView pcaConstView{pca};

    // fill up
    auto blockSize = 64;
    auto numberOfBlocks = cms::alpakatools::divide_up_by(elems, blockSize);
    
    const auto workDiv = cms::alpakatools::make_workdiv<Acc1D>(numberOfBlocks, blockSize);
    
    alpaka::exec<Acc1D>(
        queue,
        workDiv,
        FillSoA{},
        positionView,
        pcaView);
    
    alpaka::wait(queue);

    SECTION("Customized View") {
      // addresses and size of the SoA columns
      const auto posRecs = positionView.records();
      const auto pcaRecs = pcaView.records();

      // building the View with runtime check for the size
      CustomizedSoAView customizedView(posRecs.x(), posRecs.y(), posRecs.z(), pcaRecs.candidateDirection(), posRecs.detectorType());

      // Check for equality of memory addresses
      REQUIRE(customizedView.metadata().addressOf_x() == positionView.metadata().addressOf_x());
      REQUIRE(customizedView.metadata().addressOf_y() == positionView.metadata().addressOf_y());
      REQUIRE(customizedView.metadata().addressOf_z() == positionView.metadata().addressOf_z());
      REQUIRE(customizedView.metadata().addressOf_candidateDirection() ==
              pcaView.metadata().addressOf_candidateDirection());
      REQUIRE(customizedView.metadata().addressOf_detectorType() == positionView.metadata().addressOf_detectorType());        

      // Check for reference to original SoA
      auto alpakaView = alpaka::createView(device, &customizedView.x()[3], 1);
      alpaka::memset(queue, alpakaView, 0.);
      
      auto alpakaViewpos = alpaka::createView(device, &positionConstView.x()[3], 1);

      float xCustom, xPos;
      alpaka::memcpy(queue, alpaka::createView(devHost, &xCustom, 1), alpakaView);
      alpaka::memcpy(queue, alpaka::createView(devHost, &xPos, 1), alpakaViewpos);

      REQUIRE(xCustom == xPos);
    }

    SECTION("Customized ConstView") {
      // addresses and size of the SoA columns
      const auto posRecs = positionConstView.records();
      const auto pcaRecs = pcaConstView.records();

      // building the ConstView with runtime check for the size
      CustomizedSoAConstView customizedConstView(posRecs.x(), posRecs.y(), posRecs.z(), pcaRecs.candidateDirection(), posRecs.detectorType());

      // Check for equality of memory addresses
      REQUIRE(customizedConstView.metadata().addressOf_x() == positionConstView.metadata().addressOf_x());
      REQUIRE(customizedConstView.metadata().addressOf_y() == positionConstView.metadata().addressOf_y());
      REQUIRE(customizedConstView.metadata().addressOf_z() == positionConstView.metadata().addressOf_z());
      REQUIRE(customizedConstView.metadata().addressOf_candidateDirection() ==
              pcaConstView.metadata().addressOf_candidateDirection());
      REQUIRE(customizedConstView.metadata().addressOf_detectorType() == positionConstView.metadata().addressOf_detectorType());                
    }

    SECTION("Customized ConstView from Views") {
      // addresses and size of the SoA columns
      const auto posRecs = positionView.records();
      const auto pcaRecs = pcaView.records();

      // building the ConstView with runtime check for the size
      CustomizedSoAConstView customizedConstView(posRecs.x(), posRecs.y(), posRecs.z(), pcaRecs.candidateDirection(), posRecs.detectorType());

      // Check for reference to the Custom SoA - it is possible to modify the ConstView by reference modifying the Views
      auto alpakaView = alpaka::createView(device, &customizedConstView.x()[3], 1);
      auto alpakaViewpos = alpaka::createView(device, &positionView.x()[3], 1);
      alpaka::memset(queue, alpakaViewpos, 0.);

      float xCustom, xPos;
      alpaka::memcpy(queue, alpaka::createView(devHost, &xCustom, 1), alpakaView);
      alpaka::memcpy(queue, alpaka::createView(devHost, &xPos, 1), alpakaViewpos);

      REQUIRE(xCustom == xPos);
    }

    SECTION("Aggregate the Customized View") {
      // building the Layout
      const std::size_t customBufferSize = CustomizedSoA::computeDataSize(elems);

      auto bufferCustom = alpaka::allocBuf<std::byte, std::size_t>(device, customBufferSize);    
          
      CustomizedSoA customSoA(bufferCustom.data(), elems);

      // building the Customized View
      const auto posRecs = positionView.records();
      const auto pcaRecs = pcaView.records();
      CustomizedSoAView customizedView(posRecs.x(), posRecs.y(), posRecs.z(), pcaRecs.candidateDirection(), posRecs.detectorType());

      // aggregate the columns from the view with runtime check for the size
      customSoA.deepCopy(customizedView, queue);
      // building the View of the aggregated SoA
      CustomizedSoAView customizedAggregatedView{customSoA};

      REQUIRE(customizedAggregatedView.metadata().addressOf_x() != positionConstView.metadata().addressOf_x());
      REQUIRE(customizedAggregatedView.metadata().addressOf_y() != positionConstView.metadata().addressOf_y());
      REQUIRE(customizedAggregatedView.metadata().addressOf_z() != positionConstView.metadata().addressOf_z());
      REQUIRE(customizedAggregatedView.metadata().addressOf_candidateDirection() !=
              pcaConstView.metadata().addressOf_candidateDirection());
      REQUIRE(customizedAggregatedView.metadata().addressOf_detectorType() != positionConstView.metadata().addressOf_detectorType());        

      // Check for column alignments
      REQUIRE(0 == reinterpret_cast<uintptr_t>(customizedAggregatedView.metadata().addressOf_x()) %
                      decltype(customSoA)::alignment);
      REQUIRE(0 == reinterpret_cast<uintptr_t>(customizedAggregatedView.metadata().addressOf_y()) %
                      decltype(customSoA)::alignment);
      REQUIRE(0 == reinterpret_cast<uintptr_t>(customizedAggregatedView.metadata().addressOf_z()) %
                      decltype(customSoA)::alignment);
      REQUIRE(0 == reinterpret_cast<uintptr_t>(customizedAggregatedView.metadata().addressOf_candidateDirection()) %
                      decltype(customSoA)::alignment);
      REQUIRE(0 == reinterpret_cast<uintptr_t>(customizedAggregatedView.metadata().addressOf_detectorType()) %
                      decltype(customSoA)::alignment);                      

      // Check for contiguity of columns
      REQUIRE(reinterpret_cast<std::byte *>(customizedAggregatedView.metadata().addressOf_x()) +
                  cms::soa::alignSize(elems * sizeof(float), CustomizedSoA::alignment) ==
              reinterpret_cast<std::byte *>(customizedAggregatedView.metadata().addressOf_y()));
      REQUIRE(reinterpret_cast<std::byte *>(customizedAggregatedView.metadata().addressOf_y()) +
                  cms::soa::alignSize(elems * sizeof(float), CustomizedSoA::alignment) ==
              reinterpret_cast<std::byte *>(customizedAggregatedView.metadata().addressOf_z()));
      REQUIRE(reinterpret_cast<std::byte *>(customizedAggregatedView.metadata().addressOf_z()) +
                  cms::soa::alignSize(elems * sizeof(float), CustomizedSoA::alignment) ==
              reinterpret_cast<std::byte *>(customizedAggregatedView.metadata().addressOf_candidateDirection()));
      REQUIRE(reinterpret_cast<std::byte *>(customizedAggregatedView.metadata().addressOf_candidateDirection()) +
                  cms::soa::alignSize(elems * sizeof(Eigen::Vector3d::Scalar), CustomizedSoA::alignment) *
                  Eigen::Vector3d::RowsAtCompileTime * Eigen::Vector3d::ColsAtCompileTime ==
              reinterpret_cast<std::byte *>(customizedAggregatedView.metadata().addressOf_detectorType()));    
              
      // Check for the correctness of the copy
      auto bufferCustom_host = alpaka::allocBuf<std::byte, std::size_t>(devHost, customBufferSize);  
      auto bufferPos_host = alpaka::allocBuf<std::byte, std::size_t>(devHost, positionBufferSize);
      auto bufferPCA_host = alpaka::allocBuf<std::byte, std::size_t>(devHost, pcaBufferSize);

      CustomizedSoA customSoA_host(bufferCustom_host.data(), elems);
      SoAPosition position_host(bufferPos_host.data(), elems);
      SoAPCA pca_host(bufferPCA_host.data(), elems);

      alpaka::memcpy(queue, bufferCustom_host, bufferCustom);
      alpaka::memcpy(queue, bufferPos_host, bufferPos);
      alpaka::memcpy(queue, bufferPCA_host, bufferPCA);

      CustomizedSoAView customizedView_host{customSoA_host};
      SoAPositionView positionView_host{position_host};
      SoAPCAView pcaView_host{pca_host};

      for (size_t i = 0; i < elems; i++) {
        REQUIRE(customizedView_host[i].x() == positionView_host[i].x());
        REQUIRE(customizedView_host[i].y() == positionView_host[i].y());
        REQUIRE(customizedView_host[i].z() == positionView_host[i].z());
        REQUIRE(customizedView_host[i].candidateDirection()(0) == pcaView_host[i].candidateDirection()(0));
        REQUIRE(customizedView_host[i].candidateDirection()(1) == pcaView_host[i].candidateDirection()(1));
        REQUIRE(customizedView_host[i].candidateDirection()(2) == pcaView_host[i].candidateDirection()(2));
      }

      REQUIRE(customizedView_host.detectorType() == positionView_host.detectorType());

      // Check for the independency of the aggregated SoA
      auto alpakaView = alpaka::createView(device, &customizedAggregatedView.x()[3], 1);
      auto alpakaViewpos = alpaka::createView(device, &positionView.x()[3], 1);
      alpaka::memset(queue, alpakaView, 0.);

      float xCustom, xPos;
      alpaka::memcpy(queue, alpaka::createView(devHost, &xCustom, 1), alpakaView);
      alpaka::memcpy(queue, alpaka::createView(devHost, &xPos, 1), alpakaViewpos);

      REQUIRE(xCustom != xPos);
    }
  }  
}
