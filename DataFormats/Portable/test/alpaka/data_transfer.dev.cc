// Data transfer benchmark
#include <Eigen/Core>
#include <Eigen/Dense>

#include <alpaka/alpaka.hpp>

#include "DataFormats/SoATemplate/interface/SoALayout.h"
#include "DataFormats/Portable/interface/PortableCollection.h"
#include "HeterogeneousCore/AlpakaInterface/interface/config.h"
#include "HeterogeneousCore/AlpakaInterface/interface/memory.h"
#include "HeterogeneousCore/AlpakaInterface/interface/workdivision.h"

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

GENERATE_SOA_LAYOUT(GenericSoATemplate,
                    SOA_COLUMN(float, x),
                    SOA_COLUMN(float, y),
                    SOA_COLUMN(float, z),
                    SOA_EIGEN_COLUMN(Eigen::Vector3d, candidateDirection))

using GenericSoA = GenericSoATemplate<cms::soa::CacheLineSize::IntelCPU>;
using GenericSoAView = GenericSoA::View;
using GenericSoAConstView = GenericSoA::ConstView;

// Kernel for filling the SoA
struct FillSoA {
  template <typename TAcc, typename PositionView, typename PCAView>
  ALPAKA_FN_ACC void operator()(TAcc const& acc, PositionView positionView, PCAView pcaView) const {
    constexpr float interval = 0.01f;
    if (cms::alpakatools::once_per_grid(acc))
      positionView.detectorType() = 1;

    for (auto local_idx : cms::alpakatools::uniform_elements(acc, positionView.metadata().size())) {
      positionView[local_idx].x() = static_cast<float>(local_idx);
      positionView[local_idx].y() = static_cast<float>(local_idx) * 2.0f;
      positionView[local_idx].z() = static_cast<float>(local_idx) * 3.0f;

      pcaView[local_idx].eigenvector_1() = positionView[local_idx].x() / interval;
      pcaView[local_idx].eigenvector_2() = positionView[local_idx].y() / interval;
      pcaView[local_idx].eigenvector_3() = positionView[local_idx].z() / interval;
      pcaView[local_idx].candidateDirection()(0) = positionView[local_idx].x() / interval;
      pcaView[local_idx].candidateDirection()(1) = positionView[local_idx].y() / interval;
      pcaView[local_idx].candidateDirection()(2) = positionView[local_idx].z() / interval;
    }
  }
};

int main(int argc, char** argv) {
    
}
