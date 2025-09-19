// SoABlocks benchmark
#include <chrono>
#include <cstdlib>
#include <cerrno>
#include <Eigen/Core>
#include <Eigen/Dense>

#include <alpaka/alpaka.hpp>

#include "DataFormats/SoATemplate/interface/SoABlocks.h"
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

GENERATE_SOA_LAYOUT(SoAPositionTemplate,
                    SOA_COLUMN(float, x),
                    SOA_COLUMN(float, y),
                    SOA_COLUMN(float, z),
                    SOA_SCALAR(int, detectorType))

using SoAPosition = SoAPositionTemplate<>;
using SoAPositionView = SoAPosition::View;
using SoAPositionConstView = SoAPosition::ConstView;

GENERATE_SOA_LAYOUT(SoAVelocityTemplate,
                    SOA_COLUMN(float, vx),
                    SOA_COLUMN(float, vy),
                    SOA_COLUMN(float, vz),
                    SOA_SCALAR(int, charge))

using SoAVelocity = SoAVelocityTemplate<>;
using SoAVelocityView = SoAVelocity::View;
using SoAVelocityConstView = SoAVelocity::ConstView;

GENERATE_SOA_LAYOUT(SoAPCATemplate,
                    SOA_COLUMN(float, eigenvector_1),
                    SOA_COLUMN(float, eigenvector_2),
                    SOA_COLUMN(float, eigenvector_3),
                    SOA_EIGEN_COLUMN(Eigen::Vector3d, candidateDirection))

using SoAPCA = SoAPCATemplate<>;
using SoAPCAView = SoAPCA::View;
using SoAPCAConstView = SoAPCA::ConstView;

GENERATE_SOA_BLOCKS(SoABlocksTemplate,
                    SOA_BLOCK(position, SoAPositionTemplate),
                    SOA_BLOCK(velocity, SoAVelocityTemplate),
                    SOA_VIEW_METHODS( SOA_HOST_DEVICE void compute_velocity() {
                      for (std::size_t i = 0; i < position().metadata().size(); ++i) {
                        velocity().vx(i) = position().x(i) * TIME;
                        velocity().vy(i) = position().y(i) * TIME;
                        velocity().vz(i) = position().z(i) * TIME;
                      }
                    }),
                    SOA_BLOCK(pca, SoAPCATemplate))

using SoABlocks = SoABlocksTemplate<>;
using SoABlocksView = SoABlocks::View;
using SoABlocksConstView = SoABlocks::ConstView;

// Kernel for filling the SoAs
struct FillSoAs {
  template <typename TAcc, typename PositionView, typename VelocityView, typename PCAView>
  ALPAKA_FN_ACC void operator()(TAcc const& acc, PositionView positionView, VelocityView velocityView, PCAView pcaView) const {
    constexpr float interval = 0.01f;
    if (cms::alpakatools::once_per_grid(acc)) {
      positionView.detectorType() = 1;
      velocityView.charge() = -1;
    }
       
    for (auto local_idx : cms::alpakatools::uniform_elements(acc, pcaView.metadata().size())) {
      positionView[local_idx].x() = static_cast<float>(local_idx);
      positionView[local_idx].y() = static_cast<float>(local_idx) * 2.0f;
      positionView[local_idx].z() = static_cast<float>(local_idx) * 3.0f;

      velocityView[local_idx].vx() = positionView[local_idx].x() * TIME;
      velocityView[local_idx].vy() = positionView[local_idx].y() * TIME;
      velocityView[local_idx].vz() = positionView[local_idx].z() * TIME;

      pcaView[local_idx].eigenvector_1() = positionView[local_idx].x() / interval;
      pcaView[local_idx].eigenvector_2() = positionView[local_idx].y() / interval;
      pcaView[local_idx].eigenvector_3() = positionView[local_idx].z() / interval;
      pcaView[local_idx].candidateDirection()(0) = positionView[local_idx].x() / interval;
      pcaView[local_idx].candidateDirection()(1) = positionView[local_idx].y() / interval;
      pcaView[local_idx].candidateDirection()(2) = positionView[local_idx].z() / interval;
    }
  }
};

// Kernel for filling the SoAs
struct FillSoABlocks {
  template <typename TAcc, typename SoABlocksView>
  ALPAKA_FN_ACC void operator()(TAcc const& acc, SoABlocksView view) const {
    constexpr float interval = 0.01f;
    if (cms::alpakatools::once_per_grid(acc)) {
      view.position().detectorType() = 1;
      view.velocity().charge() = -1;
    }  

    for (auto local_idx : cms::alpakatools::uniform_elements(acc, view.pca().metadata().size())) {
      view.position()[local_idx].x() = static_cast<float>(local_idx);
      view.position()[local_idx].y() = static_cast<float>(local_idx) * 2.0f;
      view.position()[local_idx].z() = static_cast<float>(local_idx) * 3.0f;

      view.velocity()[local_idx].vx() = view.position()[local_idx].x() * TIME;
      view.velocity()[local_idx].vy() = view.position()[local_idx].y() * TIME;
      view.velocity()[local_idx].vz() = view.position()[local_idx].z() * TIME;

      view.pca()[local_idx].eigenvector_1() = view.position()[local_idx].x() / interval;
      view.pca()[local_idx].eigenvector_2() = view.position()[local_idx].y() / interval;
      view.pca()[local_idx].eigenvector_3() = view.position()[local_idx].z() / interval;
      view.pca()[local_idx].candidateDirection()(0) = view.position()[local_idx].x() / interval;
      view.pca()[local_idx].candidateDirection()(1) = view.position()[local_idx].y() / interval;
      view.pca()[local_idx].candidateDirection()(2) = view.position()[local_idx].z() / interval;
    }
  }
};

// Kernel for compute microbenchmarking on SoAs
struct ComputeBenchmarkSoAs {
  template <typename TAcc, typename PositionView, typename VelocityView, typename PCAView>
  ALPAKA_FN_ACC void operator()(TAcc const& acc,
                                PositionView positionView,
                                VelocityView velocityView,
                                PCAView     pcaView) const {
    using alpaka::math::cos;
    using alpaka::math::sin;
    using alpaka::math::sqrt;
    using alpaka::math::fma;

    constexpr float interval = 0.01f;
    constexpr int iters = 100;
    constexpr float dt = 1e-3f;

    if (cms::alpakatools::once_per_grid(acc)) {
      positionView.detectorType() = 1;
      velocityView.charge()       = -1;
    }

    for (auto local_idx :
         cms::alpakatools::uniform_elements(acc, pcaView.metadata().size())) {

      float px = positionView[local_idx].x();
      float py = positionView[local_idx].y();
      float pz = positionView[local_idx].z();

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
      velocityView[local_idx].vx() = vx;
      velocityView[local_idx].vy() = vy;
      velocityView[local_idx].vz() = vz;

      pcaView[local_idx].eigenvector_1()       = vx / interval;
      pcaView[local_idx].eigenvector_2()       = vy / interval;
      pcaView[local_idx].eigenvector_3()       = vz / interval;
      pcaView[local_idx].candidateDirection()(0) = ax;
      pcaView[local_idx].candidateDirection()(1) = ay;
      pcaView[local_idx].candidateDirection()(2) = az;
    }
  }
};

// Kernel for compute microbenchmarking on SoABlocks
struct ComputeBenchmarkSoABlocks {
  template <typename TAcc, typename SoABlocksView>
  ALPAKA_FN_ACC void operator()(TAcc const& acc,
                                SoABlocksView view) const {
    using alpaka::math::cos;
    using alpaka::math::sin;
    using alpaka::math::sqrt;
    using alpaka::math::fma;

    constexpr float interval = 0.01f;
    constexpr int iters = 100;
    constexpr float dt = 1e-3f;

    if (cms::alpakatools::once_per_grid(acc)) {
      view.position().detectorType() = 1;
      view.velocity().charge()       = -1;
    }

    for (auto local_idx :
         cms::alpakatools::uniform_elements(acc, view.pca().metadata().size())) {
 
      float px = view.position()[local_idx].x();
      float py = view.position()[local_idx].y();
      float pz = view.position()[local_idx].z();

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
      view.velocity()[local_idx].vx() = vx;
      view.velocity()[local_idx].vy() = vy;
      view.velocity()[local_idx].vz() = vz;

      view.pca()[local_idx].eigenvector_1()       = vx / interval;
      view.pca()[local_idx].eigenvector_2()       = vy / interval;
      view.pca()[local_idx].eigenvector_3()       = vz / interval;
      view.pca()[local_idx].candidateDirection()(0) = ax;
      view.pca()[local_idx].candidateDirection()(1) = ay;
      view.pca()[local_idx].candidateDirection()(2) = az;
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
    const int pos_elems = size;
    const int vel_elems = size * 0.9;
    const int pca_elems = size * 0.8;

    const std::array<cms::soa::size_type, 3> sizes{{pos_elems, vel_elems, pca_elems}};

    // Portable Collections SoA
    PortableCollection<SoAPosition, Device> positionCollection(pos_elems, queue);
    SoAPositionView& positionCollectionView = positionCollection.view();
    PortableCollection<SoAVelocity, Device> velocityCollection(vel_elems, queue);
    SoAVelocityView& velocityCollectionView = velocityCollection.view();
    PortableCollection<SoAPCA, Device> pcaCollection(pca_elems, queue);
    SoAPCAView& pcaCollectionView = pcaCollection.view();
    
    // PortableCollection SoABlocks
    PortableCollection<SoABlocks, Device> blocksCollection(sizes, queue);
    SoABlocksView& blocksCollectionView = blocksCollection.view();

    // fill up
    // block size: argv[2] if valid, else 512
    const std::size_t blockSize = parse_or_default(argc > 2 ? argv[2] : nullptr, 512);

    const std::size_t defaultBlocks = cms::alpakatools::divide_up_by(pos_elems, blockSize);

    // number of blocks: argv[3] if valid (>0), else defaultBlocks
    std::size_t numberOfBlocks = parse_or_default(argc > 3 ? argv[3] : nullptr, defaultBlocks);

    // (Optional) guard: never let it be 0
    if (numberOfBlocks == 0) numberOfBlocks = defaultBlocks;
    
    const auto workDiv = cms::alpakatools::make_workdiv<Acc1D>(numberOfBlocks, blockSize);
    
    alpaka::exec<Acc1D>(queue, workDiv, FillSoAs{}, positionCollectionView, velocityCollectionView, pcaCollectionView);
    alpaka::wait(queue);
    alpaka::exec<Acc1D>(queue, workDiv, FillSoABlocks{}, blocksCollectionView);
    alpaka::wait(queue);

    // warm-up runs (discard)
    start = std::chrono::high_resolution_clock::now();
    int warmup = 2;
    for (int w = 0; w < warmup; ++w) {
      alpaka::exec<Acc1D>(queue, workDiv, ComputeBenchmarkSoAs{}, positionCollectionView, velocityCollectionView, pcaCollectionView);
      alpaka::wait(queue);
      alpaka::exec<Acc1D>(queue, workDiv, ComputeBenchmarkSoABlocks{}, blocksCollectionView);
      alpaka::wait(queue);
    }
    end = std::chrono::high_resolution_clock::now();

    // benchmark
    start = std::chrono::high_resolution_clock::now();
    alpaka::exec<Acc1D>(queue,
                     workDiv,
                     ComputeBenchmarkSoABlocks{},
                     blocksCollectionView);
    alpaka::wait(queue);
    end = std::chrono::high_resolution_clock::now();
    elapsed = end - start;
    std::cout << "Total execution time for SoABlocks: " << elapsed.count() * 1000 << " ms\n";

    start = std::chrono::high_resolution_clock::now();
    alpaka::exec<Acc1D>(queue,
                     workDiv,
                     ComputeBenchmarkSoAs{},
                     positionCollectionView,
                     velocityCollectionView,
                     pcaCollectionView);
    alpaka::wait(queue);
    end = std::chrono::high_resolution_clock::now();
    elapsed = end - start;
    std::cout << "Total execution time for SoAs: " << elapsed.count() * 1000 << " ms\n";

  }

  return 0;
}

