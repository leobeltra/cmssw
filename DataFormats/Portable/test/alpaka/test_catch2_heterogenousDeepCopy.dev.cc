#include <Eigen/Core>
#include <Eigen/Dense>

#include <alpaka/alpaka.hpp>

#define CATCH_CONFIG_MAIN
#include <catch.hpp>

#include "DataFormats/SoATemplate/interface/SoALayout.h"
#include "DataFormats/Portable/interface/PortableCollection.h"
#include "HeterogeneousCore/AlpakaInterface/interface/config.h"
#include "HeterogeneousCore/AlpakaInterface/interface/memory.h"
#include "HeterogeneousCore/AlpakaInterface/interface/workdivision.h"

using namespace ALPAKA_ACCELERATOR_NAMESPACE;

#define TIME 0.01

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
                    SOA_EIGEN_COLUMN(Eigen::Vector3d, candidateDirection))

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

TEST_CASE("Aggregate from SoA Customized View") {
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
    const std::size_t elems = 10;

    // Portable Collections
    PortableCollection<SoAPosition, Device> positionCollection(elems, queue);
    PortableCollection<SoAPCA, Device> pcaCollection(elems, queue);

    // Portable Collection Views
    SoAPositionView& positionCollectionView = positionCollection.view();
    SoAPCAView& pcaCollectionView = pcaCollection.view();
    // Portable Collection ConstViews
    const SoAPositionConstView& positionCollectionConstView = positionCollection.const_view();
    const SoAPCAConstView& pcaCollectionConstView = pcaCollection.const_view();

    // fill up
    auto blockSize = 64;
    auto numberOfBlocks = cms::alpakatools::divide_up_by(elems, blockSize);

    const auto workDiv = cms::alpakatools::make_workdiv<Acc1D>(numberOfBlocks, blockSize);

    alpaka::exec<Acc1D>(
        queue,
        workDiv,
        FillSoA{},
        positionCollectionView,
        pcaCollectionView);

    alpaka::wait(queue);

    SECTION("Aggregate the View host to host and device to device") {
      // addresses and size of the SoA columns
      const auto posRecs = positionCollectionView.records();
      const auto pcaRecs = pcaCollectionView.records();

      // building the View with runtime check for the size
      CustomizedSoAView customView(posRecs.x(), posRecs.y(), posRecs.z(), pcaRecs.candidateDirection());

      // Check for equality of memory addresses
      REQUIRE(customView.metadata().addressOf_x() == positionCollectionView.metadata().addressOf_x());
      REQUIRE(customView.metadata().addressOf_y() == positionCollectionView.metadata().addressOf_y());
      REQUIRE(customView.metadata().addressOf_z() == positionCollectionView.metadata().addressOf_z());
      REQUIRE(customView.metadata().addressOf_candidateDirection() ==
              pcaCollectionView.metadata().addressOf_candidateDirection());

      // PortableCollection that will host the aggregated columns
      PortableCollection<CustomizedSoA, Device> customCollection(elems, queue);
      CustomizedSoA::ConstDescriptor descriptor(customView);
      customCollection.deepCopy(descriptor, queue);

      // Check for inequality of memory addresses
      REQUIRE(customCollection.view().metadata().addressOf_x() != positionCollectionView.metadata().addressOf_x());
      REQUIRE(customCollection.view().metadata().addressOf_y() != positionCollectionView.metadata().addressOf_y());
      REQUIRE(customCollection.view().metadata().addressOf_z() != positionCollectionView.metadata().addressOf_z());
      REQUIRE(customCollection.view().metadata().addressOf_candidateDirection() !=
              pcaCollectionView.metadata().addressOf_candidateDirection());
    }

    SECTION("Aggregate the ConstView host to host and device to device") {
      // addresses and size of the SoA columns
      const auto posRecs = positionCollectionConstView.records();
      const auto pcaRecs = pcaCollectionConstView.records();

      // building the View with runtime check for the size
      CustomizedSoAConstView customConstView(posRecs.x(), posRecs.y(), posRecs.z(), pcaRecs.candidateDirection());

      // Check for equality of memory addresses
      REQUIRE(customConstView.metadata().addressOf_x() == positionCollectionView.metadata().addressOf_x());
      REQUIRE(customConstView.metadata().addressOf_y() == positionCollectionView.metadata().addressOf_y());
      REQUIRE(customConstView.metadata().addressOf_z() == positionCollectionView.metadata().addressOf_z());
      REQUIRE(customConstView.metadata().addressOf_candidateDirection() ==
              pcaCollectionView.metadata().addressOf_candidateDirection());

      // PortableCollection that will host the aggregated columns
      PortableCollection<CustomizedSoA, Device> customCollection(elems, queue);
      CustomizedSoA::ConstDescriptor descriptor(customConstView);
      customCollection.deepCopy(descriptor, queue);

      // Check for inequality of memory addresses
      REQUIRE(customCollection.view().metadata().addressOf_x() != positionCollectionView.metadata().addressOf_x());
      REQUIRE(customCollection.view().metadata().addressOf_y() != positionCollectionView.metadata().addressOf_y());
      REQUIRE(customCollection.view().metadata().addressOf_z() != positionCollectionView.metadata().addressOf_z());
      REQUIRE(customCollection.view().metadata().addressOf_candidateDirection() !=
              pcaCollectionView.metadata().addressOf_candidateDirection());

      PortableHostCollection<CustomizedSoA> customHostCollection(elems, queue);
      PortableHostCollection<SoAPosition> positionHostCollection(elems, queue);
      PortableHostCollection<SoAPCA> pcaHostCollection(elems, queue);

      alpaka::memcpy(queue, customHostCollection.buffer(), customCollection.buffer());
      alpaka::memcpy(queue, positionHostCollection.buffer(), positionCollection.buffer());
      alpaka::memcpy(queue, pcaHostCollection.buffer(), pcaCollection.buffer());

      alpaka::wait(queue);

      const CustomizedSoAConstView& customizedViewHostCollection = customHostCollection.const_view();
      const SoAPositionConstView& positionViewHostCollection = positionHostCollection.const_view();
      const SoAPCAConstView& pcaViewHostCollection = pcaHostCollection.const_view();

      for (size_t i = 0; i < elems; i++) {
        REQUIRE(customizedViewHostCollection[i].x() == positionViewHostCollection[i].x());
        REQUIRE(customizedViewHostCollection[i].y() == positionViewHostCollection[i].y());
        REQUIRE(customizedViewHostCollection[i].z() == positionViewHostCollection[i].z());
        REQUIRE(customizedViewHostCollection[i].candidateDirection()(0) == pcaViewHostCollection[i].candidateDirection()(0));
        REQUIRE(customizedViewHostCollection[i].candidateDirection()(1) == pcaViewHostCollection[i].candidateDirection()(1));
        REQUIRE(customizedViewHostCollection[i].candidateDirection()(2) == pcaViewHostCollection[i].candidateDirection()(2));
      }
    }

    SECTION("Aggregate the View device to host") {
      // addresses and size of the SoA columns
      const auto posRecs = positionCollectionConstView.records();
      const auto pcaRecs = pcaCollectionConstView.records();

      // building the View with runtime check for the size
      CustomizedSoAConstView customConstView(posRecs.x(), posRecs.y(), posRecs.z(), pcaRecs.candidateDirection());

      // Check for equality of memory addresses
      REQUIRE(customConstView.metadata().addressOf_x() == positionCollectionView.metadata().addressOf_x());
      REQUIRE(customConstView.metadata().addressOf_y() == positionCollectionView.metadata().addressOf_y());
      REQUIRE(customConstView.metadata().addressOf_z() == positionCollectionView.metadata().addressOf_z());
      REQUIRE(customConstView.metadata().addressOf_candidateDirection() ==
              pcaCollectionView.metadata().addressOf_candidateDirection());

      // PortableCollection that will host the aggregated columns
      PortableHostCollection<CustomizedSoA> customCollection(elems, queue);
      CustomizedSoA::ConstDescriptor descriptor(customConstView);
      customCollection.deepCopy(descriptor, queue);

      // Check for inequality of memory addresses
      REQUIRE(customCollection.view().metadata().addressOf_x() != positionCollectionView.metadata().addressOf_x());
      REQUIRE(customCollection.view().metadata().addressOf_y() != positionCollectionView.metadata().addressOf_y());
      REQUIRE(customCollection.view().metadata().addressOf_z() != positionCollectionView.metadata().addressOf_z());
      REQUIRE(customCollection.view().metadata().addressOf_candidateDirection() !=
              pcaCollectionView.metadata().addressOf_candidateDirection());

      PortableHostCollection<SoAPosition> positionHostCollection(elems, queue);
      PortableHostCollection<SoAPCA> pcaHostCollection(elems, queue);

      alpaka::memcpy(queue, positionHostCollection.buffer(), positionCollection.buffer());
      alpaka::memcpy(queue, pcaHostCollection.buffer(), pcaCollection.buffer());

      alpaka::wait(queue);

      const CustomizedSoAConstView& customizedViewCollection = customCollection.const_view();
      const SoAPositionConstView& positionViewHostCollection = positionHostCollection.const_view();
      const SoAPCAConstView& pcaViewHostCollection = pcaHostCollection.const_view();

      for (size_t i = 0; i < elems; i++) {
        REQUIRE(customizedViewCollection[i].x() == positionViewHostCollection[i].x());
        REQUIRE(customizedViewCollection[i].y() == positionViewHostCollection[i].y());
        REQUIRE(customizedViewCollection[i].z() == positionViewHostCollection[i].z());
        REQUIRE(customizedViewCollection[i].candidateDirection()(0) == pcaViewHostCollection[i].candidateDirection()(0));
        REQUIRE(customizedViewCollection[i].candidateDirection()(1) == pcaViewHostCollection[i].candidateDirection()(1));
        REQUIRE(customizedViewCollection[i].candidateDirection()(2) == pcaViewHostCollection[i].candidateDirection()(2));
      }
    }

    SECTION("Aggregate the View host to device") {
      PortableHostCollection<SoAPosition> positionHostCollection(elems, queue);
      PortableHostCollection<SoAPCA> pcaHostCollection(elems, queue);

      alpaka::memcpy(queue, positionHostCollection.buffer(), positionCollection.buffer());
      alpaka::memcpy(queue, pcaHostCollection.buffer(), pcaCollection.buffer());

      const SoAPositionConstView& positionViewHostCollection = positionHostCollection.const_view();
      const SoAPCAConstView& pcaViewHostCollection = pcaHostCollection.const_view();

      // addresses and size of the SoA columns
      const auto posRecs = positionViewHostCollection.records();
      const auto pcaRecs = pcaViewHostCollection.records();

      // building the View with runtime check for the size
      CustomizedSoAConstView customConstView(posRecs.x(), posRecs.y(), posRecs.z(), pcaRecs.candidateDirection());

      // Check for equality of memory addresses
      REQUIRE(customConstView.metadata().addressOf_x() == positionViewHostCollection.metadata().addressOf_x());
      REQUIRE(customConstView.metadata().addressOf_y() == positionViewHostCollection.metadata().addressOf_y());
      REQUIRE(customConstView.metadata().addressOf_z() == positionViewHostCollection.metadata().addressOf_z());
      REQUIRE(customConstView.metadata().addressOf_candidateDirection() ==
              pcaViewHostCollection.metadata().addressOf_candidateDirection());

      // PortableCollection that will host the aggregated columns
      PortableCollection<CustomizedSoA, Device> customCollection(elems, queue);
      CustomizedSoA::ConstDescriptor descriptor(customConstView);
      customCollection.deepCopy(descriptor, queue);

      // Check for inequality of memory addresses
      REQUIRE(customCollection.view().metadata().addressOf_x() != positionViewHostCollection.metadata().addressOf_x());
      REQUIRE(customCollection.view().metadata().addressOf_y() != positionViewHostCollection.metadata().addressOf_y());
      REQUIRE(customCollection.view().metadata().addressOf_z() != positionViewHostCollection.metadata().addressOf_z());
      REQUIRE(customCollection.view().metadata().addressOf_candidateDirection() !=
              pcaViewHostCollection.metadata().addressOf_candidateDirection());

      PortableHostCollection<CustomizedSoA> customHostCollection(elems, queue);

      alpaka::memcpy(queue, customHostCollection.buffer(), customCollection.buffer());

      alpaka::wait(queue);

      const CustomizedSoAConstView& customizedViewHostCollection = customHostCollection.const_view();

      for (size_t i = 0; i < elems; i++) {
        REQUIRE(customizedViewHostCollection[i].x() == positionViewHostCollection[i].x());
        REQUIRE(customizedViewHostCollection[i].y() == positionViewHostCollection[i].y());
        REQUIRE(customizedViewHostCollection[i].z() == positionViewHostCollection[i].z());
        REQUIRE(customizedViewHostCollection[i].candidateDirection()(0) == pcaViewHostCollection[i].candidateDirection()(0));
        REQUIRE(customizedViewHostCollection[i].candidateDirection()(1) == pcaViewHostCollection[i].candidateDirection()(1));
        REQUIRE(customizedViewHostCollection[i].candidateDirection()(2) == pcaViewHostCollection[i].candidateDirection()(2));
      }
    }
  }
}