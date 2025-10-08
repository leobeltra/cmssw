// SoABlocks benchmark
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

#define TIME 0.1f

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

GENERATE_SOA_LAYOUT(SoATemplate,
                    SOA_COLUMN(float, x),
                    SOA_COLUMN(float, y),
                    SOA_COLUMN(float, z),
                    SOA_SCALAR(int, detectorType),
                    SOA_COLUMN(float, vx),
                    SOA_COLUMN(float, vy),
                    SOA_COLUMN(float, vz),
                    SOA_SCALAR(int, charge),
                    SOA_COLUMN(float, eigenvector_1),
                    SOA_COLUMN(float, eigenvector_2),
                    SOA_COLUMN(float, eigenvector_3),
                    SOA_EIGEN_COLUMN(Eigen::Vector3d, candidateDirection))

using SoA = SoATemplate<>;
using AoS = SoA::AoSWrapper;
using SoAView = SoA::View;
using AoSView = AoS::View;
using SoAConstView = SoA::ConstView;
using AoSConstView = AoS::ConstView;

// Kernel for filling the AoS
// struct FillAoS {
//     template <typename TAcc, typename AoSView>
//     ALPAKA_FN_ACC void operator()(TAcc const& acc, AoSView view) const {
//       constexpr float interval = 0.01f;
//       if (cms::alpakatools::once_per_grid(acc)) {
//         view.detectorType() = 1;
//         view.charge() = -1;
//       }  
  
//       for (auto local_idx : cms::alpakatools::uniform_elements(acc, view.pca().metadata().size())) {
//         view[local_idx].x() = static_cast<float>(local_idx);
//         view[local_idx].y() = static_cast<float>(local_idx) * 2.0f;
//         view[local_idx].z() = static_cast<float>(local_idx) * 3.0f;
  
//         view[local_idx].vx() = view[local_idx].x() * TIME;
//         view[local_idx].vy() = view[local_idx].y() * TIME;
//         view[local_idx].vz() = view[local_idx].z() * TIME;
  
//         view[local_idx].eigenvector_1() = view[local_idx].x() / interval;
//         view[local_idx].eigenvector_2() = view[local_idx].y() / interval;
//         view[local_idx].eigenvector_3() = view[local_idx].z() / interval;
//         view[local_idx].candidateDirection()(0) = view[local_idx].x() / interval;
//         view[local_idx].candidateDirection()(1) = view[local_idx].y() / interval;
//         view[local_idx].candidateDirection()(2) = view[local_idx].z() / interval;
//       }
//     }
//   };
  

// Kernel for filling the SoA
struct Fill {
  template <typename TAcc, typename SoAView>
  ALPAKA_FN_ACC void operator()(TAcc const& acc, SoAView view) const {
    constexpr float interval = 0.01f;
    if (cms::alpakatools::once_per_grid(acc)) {
      view.detectorType() = 1;
      view.charge() = -1;
    }  

    for (auto local_idx : cms::alpakatools::uniform_elements(acc, view.metadata().size())) {
      view[local_idx].x() = static_cast<float>(local_idx);
      view[local_idx].y() = static_cast<float>(local_idx) * 2.0f;
      view[local_idx].z() = static_cast<float>(local_idx) * 3.0f;

      view[local_idx].vx() = view[local_idx].x() * TIME;
      view[local_idx].vy() = view[local_idx].y() * TIME;
      view[local_idx].vz() = view[local_idx].z() * TIME;

      view[local_idx].eigenvector_1() = view[local_idx].x() / interval;
      view[local_idx].eigenvector_2() = view[local_idx].y() / interval;
      view[local_idx].eigenvector_3() = view[local_idx].z() / interval;
      view[local_idx].candidateDirection()(0) = view[local_idx].x() / interval;
      view[local_idx].candidateDirection()(1) = view[local_idx].y() / interval;
      view[local_idx].candidateDirection()(2) = view[local_idx].z() / interval;
    }
  }
};

// Kernel for compute microbenchmarking on SoA
struct ComputeBenchmark {
  template <typename TAcc, typename SoAView>
  ALPAKA_FN_ACC void operator()(TAcc const& acc,
                                SoAView view) const {
    using alpaka::math::cos;
    using alpaka::math::sin;
    using alpaka::math::sqrt;
    using alpaka::math::fma;

    constexpr float interval = 0.01f;
    constexpr int iters = 100;
    constexpr float dt = 1e-3f;

    if (cms::alpakatools::once_per_grid(acc)) {
      view.detectorType() = 1;
      view.charge()       = -1;
    }

    for (auto local_idx :
         cms::alpakatools::uniform_elements(acc, view.metadata().size())) {
 
      float px = view[local_idx].x();
      float py = view[local_idx].y();
      float pz = view[local_idx].z();

      // Seed "velocities" and "accelerations" from the position
      float vx = px * 0.5f;
      float vy = py * 0.5f;
      float vz = pz * 0.5f;

      float ax = 0.001f + 1e-6f * px;
      float ay = 0.002f + 1e-6f * py;
      float az = 0.003f + 1e-6f * pz;

      for (int k = 0; k < iters; ++k) {
        // a * t + v and write on v
        vx = fma(acc, ax, dt, vx);
        vy = fma(acc, ay, dt, vy);
        vz = fma(acc, az, dt, vz);

        // some math
        float s = sin(acc, vx) + cos(acc, vy);
        float r = sqrt(acc, vz * vz + 1.0f);

        // s * 1e-3f + a
        ax = fma(acc, s, 1e-3f, ax);
        ay = fma(acc, r, 1e-3f, ay);
        az = fma(acc, s + r, 1e-3f, az);
      }

      // Write results
      view[local_idx].vx() = vx;
      view[local_idx].vy() = vy;
      view[local_idx].vz() = vz;

      view[local_idx].eigenvector_1()       = vx / interval;
      view[local_idx].eigenvector_2()       = vy / interval;
      view[local_idx].eigenvector_3()       = vz / interval;
      view[local_idx].candidateDirection()(0) = ax;
      view[local_idx].candidateDirection()(1) = ay;
      view[local_idx].candidateDirection()(2) = az;
    }
  }
};


int main(int argc, char** argv) {

  auto const& devices = cms::alpakatools::devices<Platform>();
  if (devices.empty()) {
  std::cout << "No devices available for the " <<  EDM_STRINGIZE(ALPAKA_ACCELERATOR_NAMESPACE) << " backend, "
      "the test will be skipped." << std::endl;
  }

  auto devHost = alpaka::getDevByIdx(alpaka::PlatformCpu{}, 0u);

  std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
  std::chrono::duration<double> elapsed;

  for (auto const& device : cms::alpakatools::devices<Platform>()) {
    std::cout << "Running on " << alpaka::getName(device) << std::endl;

    Queue queue(device);

    // number of elements
    const std::size_t size = parse_or_default(argc > 1 ? argv[1] : nullptr, 1000000);
    
    // PortableCollection SoA
    PortableCollection<SoA, Device> soa(size, queue);
    SoAView& soaView = soa.view();

    // PortableCollection AoS
    PortableCollection<AoS, Device> aos(size, queue);
    AoSView& aosView = aos.view();

    // fill up
    // block size: argv[2] if valid, else 512
    const std::size_t blockSize = parse_or_default(argc > 2 ? argv[2] : nullptr, 512);

    const std::size_t defaultBlocks = cms::alpakatools::divide_up_by(size, blockSize);

    // number of blocks: argv[3] if valid (>0), else defaultBlocks
    std::size_t numberOfBlocks = parse_or_default(argc > 3 ? argv[3] : nullptr, defaultBlocks);

    // (Optional) guard: never let it be 0
    if (numberOfBlocks == 0) numberOfBlocks = defaultBlocks;
    
    const auto workDiv = cms::alpakatools::make_workdiv<Acc1D>(numberOfBlocks, blockSize);
    
    alpaka::exec<Acc1D>(queue, workDiv, Fill{}, aosView);
    alpaka::wait(queue);
    alpaka::exec<Acc1D>(queue, workDiv, Fill{}, soaView);
    alpaka::wait(queue);

    // warm-up runs (discard)
    start = std::chrono::high_resolution_clock::now();
    int warmup = 2;
    for (int w = 0; w < warmup; ++w) {
      alpaka::exec<Acc1D>(queue, workDiv, ComputeBenchmark{}, aosView);
      alpaka::wait(queue);
      alpaka::exec<Acc1D>(queue, workDiv, ComputeBenchmark{}, soaView);
      alpaka::wait(queue);
    }
    end = std::chrono::high_resolution_clock::now();

    // benchmark
    start = std::chrono::high_resolution_clock::now();
    alpaka::exec<Acc1D>(queue,
                     workDiv,
                     ComputeBenchmark{},
                     soaView);
    alpaka::wait(queue);
    end = std::chrono::high_resolution_clock::now();
    elapsed = end - start;
    std::cout << "Total execution time for soa: " << elapsed.count() * 1000 << " ms\n";

    start = std::chrono::high_resolution_clock::now();
    alpaka::exec<Acc1D>(queue,
                     workDiv,
                     ComputeBenchmark{},
                     aosView);
    alpaka::wait(queue);
    end = std::chrono::high_resolution_clock::now();
    elapsed = end - start;
    std::cout << "Total execution time for aos: " << elapsed.count() * 1000 << " ms\n";

  }

  return 0;
}
