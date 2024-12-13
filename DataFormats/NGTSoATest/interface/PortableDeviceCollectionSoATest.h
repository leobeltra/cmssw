#ifndef DataFormats_NGTSoATest_interface_PortableDeviceCollectionSoATest_h
#define DataFormats_NGTSoATest_interface_PortableDeviceCollectionSoATest_h

#include <cstdint>

#include <alpaka/alpaka.hpp>

#include "DataFormats/Portable/interface/PortableDeviceCollection.h"
#include "DataFormats/NGTSoATest/interface/SoALayoutTest.h"
#include "DataFormats/NGTSoATest/interface/PortableHostCollectionSoATest.h"
#include "DataFormats/Portable/interface/PortableCollection.h"
// #include "HeterogeneousCore/AlpakaInterface/interface/CopyToHost.h"
#include "HeterogeneousCore/AlpakaInterface/interface/config.h"

// namespace ALPAKA_ACCELERATOR_NAMESPACE {
template <typename TDev>
class PortableDeviceCollectionSoATest : public PortableDeviceCollection<DeviceLayout, TDev> {
public:
  PortableDeviceCollectionSoATest() = default;

  using PortableDeviceCollection<DeviceLayout, TDev>::view;
  using PortableDeviceCollection<DeviceLayout, TDev>::const_view;
  using PortableDeviceCollection<DeviceLayout, TDev>::buffer;

  template <typename TQueue>
  explicit PortableDeviceCollectionSoATest(size_t maxModules, TQueue const& queue)
      : PortableDeviceCollection<DeviceLayout, TDev>(maxModules + 1, queue) {}

  // Constructor which specifies the SoA size
  explicit PortableDeviceCollectionSoATest(size_t maxModules, TDev const &device)
      : PortableDeviceCollection<DeviceLayout, TDev>(maxModules + 1, device) {}   
}; 
//}

#endif  // DataFormats_NGTSoATest_interface_PortableDeviceCollectionSoATestc_h