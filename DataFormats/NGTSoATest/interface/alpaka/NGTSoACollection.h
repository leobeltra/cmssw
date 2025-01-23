#ifndef DataFormats_NGTSoATest_interface_alpaka_NGTSoACollection_h
#define DataFormats_NGTSoATest_interface_alpaka_NGTSoACollection_h

#include <cstdint>

#include "DataFormats/Portable/interface/PortableHostCollection.h"
#include "DataFormats/Portable/interface/alpaka/PortableCollection.h"
#include "DataFormats/NGTSoATest/interface/NGTSoAHostCollection.h"

#include "DataFormats/NGTSoATest/interface/SoALayoutTest.h"
#include "HeterogeneousCore/AlpakaInterface/interface/config.h"

namespace ALPAKA_ACCELERATOR_NAMESPACE {
  using ::NGTSoAHostCollection;
  using NGTSoACollection = PortableCollection<NGTSoA>;

}  // namespace ALPAKA_ACCELERATOR_NAMESPACE

// namespace cms::alpakatools {
//   template <typename TDevice>
//   struct CopyToHost<PortableDeviceCollection<NGTSoA, TDevice>> {
//     template <typename TQueue>
//     static auto copyAsync(TQueue& queue, PortableDeviceCollection<NGTSoA, TDevice> const& deviceData) {
//       PortableHostCollection<NGTSoA> hostData(deviceData.view().metadata().size() - 1, queue);
//       alpaka::memcpy(queue, hostData.buffer(), deviceData.buffer());
//       printf("TracksSoACollection: I'm copying to host.\n");
//       return hostData;
//     }
//   };
// }  // namespace cms::alpakatools

ASSERT_DEVICE_MATCHES_HOST_COLLECTION(NGTSoACollection, NGTSoAHostCollection);
#endif  // DataFormats_NGTSoATest_interface_alpaka_NGTSoACollection_h