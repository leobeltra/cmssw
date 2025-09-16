// Device custom methods benchmark
#include <chrono>
#include <cstdlib>
#include <cerrno>
#include <Eigen/Core>
#include <Eigen/Dense>

#include <alpaka/alpaka.hpp>

#include "DataFormats/SoATemplate/interface/SoALayout.h"
#include "DataFormats/Portable/interface/PortableCollection.h"
#include "HeterogeneousCore/AlpakaInterface/interface/config.h"
#include "HeterogeneousCore/AlpakaInterface/interface/memory.h"
#include "HeterogeneousCore/AlpakaInterface/interface/workdivision.h"

using namespace ALPAKA_ACCELERATOR_NAMESPACE;

// Safe parser: returns fallback if s is null, invalid, or <= 0
static inline std::size_t parse_or_default(const char* s, std::size_t fallback) {
  if (!s) return fallback;
  char* end = nullptr;
  errno = 0;
  unsigned long v = std::strtoul(s, &end, 10);
  if (errno != 0 || end == s || *end != '\0' || v == 0) return fallback;
  return static_cast<std::size_t>(v);
}

GENERATE_SOA_LAYOUT(SoAPositionTemplate,
                    SOA_COLUMN(float, x),
                    SOA_COLUMN(float, y),
                    SOA_COLUMN(float, z),

                    SOA_ELEMENT_METHODS(

                        SOA_HOST_DEVICE void normalise() {
                          float norm_position = square_norm_position();
                          if (norm_position > 0.0f) {
                            x() /= norm_position;
                            y() /= norm_position;
                            z() /= norm_position;
                          }};
                    ),

                    SOA_CONST_ELEMENT_METHODS(
                        SOA_HOST_DEVICE float square_norm_position() const { return sqrt(x() * x() + y() * y() + z() * z()); };
                    ),

                    SOA_SCALAR(int, detectorType))

using SoAPosition = SoAPositionTemplate<>;
using SoAPositionView = SoAPosition::View;
using SoAPositionConstView = SoAPosition::ConstView;

// Kernel for filling the SoA
struct FillSoA {
    template <typename TAcc, typename PositionView>
    ALPAKA_FN_ACC void operator()(TAcc const& acc, PositionView positionView) const {
      if (cms::alpakatools::once_per_grid(acc))
        positionView.detectorType() = 1;
  
      for (auto local_idx : cms::alpakatools::uniform_elements(acc, positionView.metadata().size())) {
        positionView[local_idx].x() = static_cast<float>(local_idx) + 1.f;
        positionView[local_idx].y() = (static_cast<float>(local_idx) + 1.f) * 2.0f;
        positionView[local_idx].z() = (static_cast<float>(local_idx) + 1.f) * 3.0f;
      }
    }
  };

// Kernel for normalising the positions
struct NormalisePositions {
    template <typename TAcc, typename PositionView>
    ALPAKA_FN_ACC void operator()(TAcc const& acc, PositionView positionView) const {
      for (auto local_idx : cms::alpakatools::uniform_elements(acc, positionView.metadata().size())) {
        positionView[local_idx].normalise();
      }
    }
  };  
  

int main(int argc, char** argv) {

        int i=0;
        std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
        std::vector<double> inner_repetitions(10);
        double sum, average;

        auto const& devices = cms::alpakatools::devices<Platform>();
        if (devices.empty()) {
        std::cout << "No devices available for the " <<  EDM_STRINGIZE(ALPAKA_ACCELERATOR_NAMESPACE) << " backend, "
            "the test will be skipped." << std::endl;
        }
    
        auto devHost = alpaka::getDevByIdx(alpaka::PlatformCpu{}, 0u);
    
        for (auto const& device : cms::alpakatools::devices<Platform>()) {  
        std::cout << "Running on " << alpaka::getName(device) << std::endl;
    
        Queue queue(device);
    
        // common number of elements for the SoAs
        const std::size_t elems = parse_or_default(argc > 1 ? argv[1] : nullptr, );

        // Portable Collections
        PortableCollection<SoAPosition, Device> positionCollection(elems, queue);
        SoAPositionView& positionCollectionView = positionCollection.view();

        // fill up
        // 1) Block size: argv[1] if valid, else 64
        const std::size_t blockSize = parse_or_default(argc > 2 ? argv[2] : nullptr, 64);

        // 2) Default blocks: cover all elements for the chosen block size
        const std::size_t defaultBlocks = cms::alpakatools::divide_up_by(elems, blockSize);

        // 3) Number of blocks: argv[2] if valid (>0), else defaultBlocks
        std::size_t numberOfBlocks = parse_or_default(argc > 3 ? argv[3] : nullptr, defaultBlocks);

        // (Optional) guard: never let it be 0
        if (numberOfBlocks == 0) numberOfBlocks = defaultBlocks;
        
        const auto workDiv = cms::alpakatools::make_workdiv<Acc1D>(numberOfBlocks, blockSize);
        
        alpaka::exec<Acc1D>(queue, workDiv, FillSoA{}, positionCollectionView);
        alpaka::wait(queue);

        for(i=0; i<11; i++) {

          start = std::chrono::high_resolution_clock::now();

          // normalise
          alpaka::exec<Acc1D>(queue, workDiv, NormalisePositions{}, positionCollectionView);  
          alpaka::wait(queue);

          alpaka::exec<Acc1D>(queue, workDiv, FillSoA{}, positionCollectionView);
          alpaka::wait(queue);

          end = std::chrono::high_resolution_clock::now();

          std::chrono::duration<double> elapsed = (end - start) * 1000;

          if (i > 0)
            inner_repetitions[i-1] = elapsed.count();
        }

        // Calculate the sum of all elements
        sum = std::accumulate(inner_repetitions.begin(), inner_repetitions.end(), 0.0);

        // Calculate the average
        average = sum / inner_repetitions.size();
        
        std::cout << "Average execution time: " << average << " ms\n";

        PortableHostCollection<SoAPosition> positionHostCollection(elems, queue);
        alpaka::memcpy(queue, positionHostCollection.buffer(), positionCollection.buffer());
        alpaka::wait(queue);

        // check norm == 1
        // const SoAPositionConstView& positionViewHostCollection = positionHostCollection.const_view();
        // for (size_t i = 0; i < elems; i++) {
        //     float norm = positionViewHostCollection[i].square_norm_position();
        //     if (std::abs(norm - 1.0f) > 1.e-5f) {
        //     std::cout << "Error in normalisation at element " << i << " : " << norm << std::endl;
        //     }
        // }

        std::cout << "Normalisation check completed" << std::endl;

        }

    return 0;

  }  
