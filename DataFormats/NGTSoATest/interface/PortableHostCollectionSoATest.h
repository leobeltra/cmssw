#ifndef DataFormats_NGTSoATest_interface_PortableHostCollectionSoATest_h
#define DataFormats_NGTSoATest_interface_PortableHostCollectionSoATest_h

#include <cstdint>
#include <alpaka/alpaka.hpp>

#include "DataFormats/Portable/interface/PortableHostCollection.h"
#include "DataFormats/NGTSoATest/interface/SoALayoutTest.h"
// #include "HeterogeneousCore/AlpakaInterface/interface/CopyToHost.h"
#include "HeterogeneousCore/AlpakaInterface/interface/config.h"

// TODO: The class is created via inheritance of the PortableCollection.
// This is generally discouraged, and should be done via composition.
// See: https://github.com/cms-sw/cmssw/pull/40465#discussion_r1067364306
class PortableHostCollectionSoATest : public PortableHostCollection<DeviceLayout> {
public:
  static constexpr size_t elems = 16;
  PortableHostCollectionSoATest() = default;

  using PortableHostCollection<DeviceLayout>::view;
  using PortableHostCollection<DeviceLayout>::const_view;
  using PortableHostCollection<DeviceLayout>::buffer;

  template <typename TQueue>
  explicit PortableHostCollectionSoATest(size_t maxModules, TQueue queue)
      : PortableHostCollection<DeviceLayout>(maxModules + 1, queue) {}

    // Constructor which specifies the SoA size
  template <typename TQueue>
  explicit PortableHostCollectionSoATest(TQueue& queue)
      : PortableHostCollection<DeviceLayout>(elems, queue) {}

  // Constructor which specifies the DevHost
  explicit PortableHostCollectionSoATest(size_t maxModules, alpaka_common::DevHost const& host)
      : PortableHostCollection<DeviceLayout>(maxModules + 1, host) {}      

};

#endif  // DataFormats_NGTSoATest_interface_PortableHostCollectionSoATest_h