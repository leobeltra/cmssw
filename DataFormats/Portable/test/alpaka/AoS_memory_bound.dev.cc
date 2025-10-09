// SoABlocks benchmark (memory-bound, no helper function)
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

// --------- util -----------------
static inline std::size_t parse_or_default(const char* s, std::size_t fallback) {
  if (!s) return fallback;
  char* end = nullptr;
  errno = 0;
  unsigned long v = std::strtoul(s, &end, 10);
  if (errno != 0 || end == s || *end != '\0' || v == 0) return fallback;
  return static_cast<std::size_t>(v);
}

// --------- layout ----------------
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

// --------- kernels (semplici, memory-bound) ---------------
struct Init {
  template <typename TAcc, typename V>
  ALPAKA_FN_ACC void operator()(TAcc const& acc, V v) const {
    if (cms::alpakatools::once_per_grid(acc)) {
      v.detectorType() = 1;
      v.charge() = -1;
    }
    for (auto i : cms::alpakatools::uniform_elements(acc, v.metadata().size())) {
      v[i].x() = static_cast<float>(i);
      v[i].y() = static_cast<float>(i) * 2.0f;
      v[i].z() = static_cast<float>(i) * 3.0f;
      v[i].vx() = 0.f; v[i].vy() = 0.f; v[i].vz() = 0.f;
    }
  }
};

struct SaxpyX { float a; template <typename TAcc, typename V>
  ALPAKA_FN_ACC void operator()(TAcc const& acc, V v) const {
    for (auto i : cms::alpakatools::uniform_elements(acc, v.metadata().size())) {
      v[i].vx() = a * v[i].x() + v[i].vx();
    }
  }
};
struct SaxpyY { float a; template <typename TAcc, typename V>
  ALPAKA_FN_ACC void operator()(TAcc const& acc, V v) const {
    for (auto i : cms::alpakatools::uniform_elements(acc, v.metadata().size())) {
      v[i].vy() = a * v[i].y() + v[i].vy();
    }
  }
};
struct SaxpyZ { float a; template <typename TAcc, typename V>
  ALPAKA_FN_ACC void operator()(TAcc const& acc, V v) const {
    for (auto i : cms::alpakatools::uniform_elements(acc, v.metadata().size())) {
      v[i].vz() = a * v[i].z() + v[i].vz();
    }
  }
};

struct CopyXfromVX {
  template <typename TAcc, typename V>
  ALPAKA_FN_ACC void operator()(TAcc const& acc, V v) const {
    for (auto i : cms::alpakatools::uniform_elements(acc, v.metadata().size())) {
      v[i].x() = v[i].vx();
    }
  }
};
struct CopyYfromVY {
  template <typename TAcc, typename V>
  ALPAKA_FN_ACC void operator()(TAcc const& acc, V v) const {
    for (auto i : cms::alpakatools::uniform_elements(acc, v.metadata().size())) {
      v[i].y() = v[i].vy();
    }
  }
};
struct CopyZfromVZ {
  template <typename TAcc, typename V>
  ALPAKA_FN_ACC void operator()(TAcc const& acc, V v) const {
    for (auto i : cms::alpakatools::uniform_elements(acc, v.metadata().size())) {
      v[i].z() = v[i].vz();
    }
  }
};

int main(int argc, char** argv) {
  auto const& devs = cms::alpakatools::devices<Platform>();
  if (devs.empty()) {
    std::cout << "No devices available for the " << EDM_STRINGIZE(ALPAKA_ACCELERATOR_NAMESPACE)
              << " backend, the test will be skipped.\n";
    return 0;
  }

  // argv: size, blockSize, nBlocks, rounds
  const std::size_t size        = parse_or_default(argc > 1 ? argv[1] : nullptr, 4ul*1024*1024);
  const std::size_t blockSize   = parse_or_default(argc > 2 ? argv[2] : nullptr, 512);
  const std::size_t defBlocks   = cms::alpakatools::divide_up_by(size, blockSize);
  std::size_t       nBlocks     = parse_or_default(argc > 3 ? argv[3] : nullptr, defBlocks);
  if (nBlocks == 0) nBlocks = defBlocks;
  const int         rounds      = static_cast<int>(parse_or_default(argc > 4 ? argv[4] : nullptr, 10));

  for (auto const& device : devs) {
    std::cout << "Running on " << alpaka::getName(device)
              << " size=" << size << " block=" << blockSize
              << " grid=" << nBlocks << " rounds=" << rounds << "\n";

    Queue q(device);
    const auto workDiv = cms::alpakatools::make_workdiv<Acc1D>(nBlocks, blockSize);

    // Allocazioni
    PortableCollection<SoA, Device> soa(size, q);
    PortableCollection<AoS, Device> aos(size, q);
    SoAView soaView = soa.view();
    AoSView aosView = aos.view();

    // Init & warmup (stesse operazioni per uniformare cache/clock)
    alpaka::exec<Acc1D>(q, workDiv, Init{}, soaView); alpaka::wait(q);
    alpaka::exec<Acc1D>(q, workDiv, Init{}, aosView); alpaka::wait(q);

    // warmup round SoA
    alpaka::exec<Acc1D>(q, workDiv, SaxpyX{0.1f}, soaView); alpaka::wait(q);
    alpaka::exec<Acc1D>(q, workDiv, SaxpyY{0.2f}, soaView); alpaka::wait(q);
    alpaka::exec<Acc1D>(q, workDiv, SaxpyZ{0.3f}, soaView); alpaka::wait(q);
    alpaka::exec<Acc1D>(q, workDiv, CopyXfromVX{}, soaView); alpaka::wait(q);
    alpaka::exec<Acc1D>(q, workDiv, CopyYfromVY{}, soaView); alpaka::wait(q);
    alpaka::exec<Acc1D>(q, workDiv, CopyZfromVZ{}, soaView); alpaka::wait(q);

    // warmup round AoS
    alpaka::exec<Acc1D>(q, workDiv, SaxpyX{0.1f}, aosView); alpaka::wait(q);
    alpaka::exec<Acc1D>(q, workDiv, SaxpyY{0.2f}, aosView); alpaka::wait(q);
    alpaka::exec<Acc1D>(q, workDiv, SaxpyZ{0.3f}, aosView); alpaka::wait(q);
    alpaka::exec<Acc1D>(q, workDiv, CopyXfromVX{}, aosView); alpaka::wait(q);
    alpaka::exec<Acc1D>(q, workDiv, CopyYfromVY{}, aosView); alpaka::wait(q);
    alpaka::exec<Acc1D>(q, workDiv, CopyZfromVZ{}, aosView); alpaka::wait(q);

    // ---- Misura SoA (rounds x 6 kernel, con wait per forzare traffico) ----
    auto t0 = std::chrono::high_resolution_clock::now();
    for (int r = 0; r < rounds; ++r) {
      alpaka::exec<Acc1D>(q, workDiv, SaxpyX{0.1f}, soaView); alpaka::wait(q);
      alpaka::exec<Acc1D>(q, workDiv, SaxpyY{0.2f}, soaView); alpaka::wait(q);
      alpaka::exec<Acc1D>(q, workDiv, SaxpyZ{0.3f}, soaView); alpaka::wait(q);
      alpaka::exec<Acc1D>(q, workDiv, CopyXfromVX{}, soaView); alpaka::wait(q);
      alpaka::exec<Acc1D>(q, workDiv, CopyYfromVY{}, soaView); alpaka::wait(q);
      alpaka::exec<Acc1D>(q, workDiv, CopyZfromVZ{}, soaView); alpaka::wait(q);
    }
    auto t1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> soa_ms = t1 - t0;
    std::cout << "SoA time: " << (soa_ms.count() * 1000.0) << " ms\n";

    // ---- Misura AoS ----
    auto t2 = std::chrono::high_resolution_clock::now();
    for (int r = 0; r < rounds; ++r) {
      alpaka::exec<Acc1D>(q, workDiv, SaxpyX{0.1f}, aosView); alpaka::wait(q);
      alpaka::exec<Acc1D>(q, workDiv, SaxpyY{0.2f}, aosView); alpaka::wait(q);
      alpaka::exec<Acc1D>(q, workDiv, SaxpyZ{0.3f}, aosView); alpaka::wait(q);
      alpaka::exec<Acc1D>(q, workDiv, CopyXfromVX{}, aosView); alpaka::wait(q);
      alpaka::exec<Acc1D>(q, workDiv, CopyYfromVY{}, aosView); alpaka::wait(q);
      alpaka::exec<Acc1D>(q, workDiv, CopyZfromVZ{}, aosView); alpaka::wait(q);
    }
    auto t3 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> aos_ms = t3 - t2;
    std::cout << "AoS time: " << (aos_ms.count() * 1000.0) << " ms\n";
  }

  return 0;
}
