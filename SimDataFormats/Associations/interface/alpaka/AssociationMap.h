
#pragma once

#include <vector>
#include <utility>
#include <algorithm>
#include <stdexcept>
#include <type_traits>
#include <iostream>
#include <cassert>

#include <limits>

#include <alpaka/alpaka.hpp>

// CMSSW specific includes
#include "DataFormats/Common/interface/Ref.h"
#include "DataFormats/Common/interface/RefProd.h"
#include "DataFormats/Portable/interface/PortableCollection.h"
#include "FWCore/Framework/interface/Event.h"
#include "DataFormats/SoATemplate/interface/SoALayout.h"
#include "DataFormats/SoATemplate/interface/SoAView.h"
#include "HeterogeneousCore/AlpakaInterface/interface/config.h"
#include "HeterogeneousCore/AlpakaInterface/interface/memory.h"
#include "HeterogeneousCore/AlpakaInterface/interface/workdivision.h"
#include "HeterogeneousCore/AlpakaInterface/interface/prefixScan.h"

namespace ALPAKA_ACCELERATOR_NAMESPACE {

  using namespace cms::alpakatools;

  // Define wrapper types to differentiate between fraction and shared energy
  struct FractionType {
    float value;
    FractionType(float v = 0.0f) : value(v) {}
    FractionType& operator+=(float v) {
      value += v;
      return *this;
    }
  };

  struct SharedEnergyType {
    float value;
    SharedEnergyType(float v = 0.0f) : value(v) {}
    SharedEnergyType& operator+=(float v) {
      value += v;
      return *this;
    }
  };

  template <typename ValueType, typename Score>
  struct AssociationElementsSoAStruct {
    GENERATE_SOA_LAYOUT(Layout, SOA_COLUMN(ValueType, values), SOA_COLUMN(Score, scores), SOA_COLUMN(int, indexes))
  };

  template <typename ValueType>
  struct AssociationElementsSoAStruct<ValueType, void> {
    GENERATE_SOA_LAYOUT(Layout, SOA_COLUMN(ValueType, values), SOA_COLUMN(int, indexes))
  };

  template <typename ValueType, typename Score>
  using AssociationElementsSoA = typename AssociationElementsSoAStruct<ValueType, Score>::template Layout<>;
  template <typename ValueType, typename Score>
  using AssociationElementsSoAView = typename AssociationElementsSoAStruct<ValueType, Score>::template Layout<>::View;

  template <typename TDev, typename V, typename Score = void, typename = std::enable_if_t<alpaka::isDevice<TDev>>>
  class AssociationElements {
  private:
    PortableCollection<AssociationElementsSoA<V, Score>, TDev> m_data;

  public:
    using value_type = V;
    using score_type = std::enable_if_t<!std::is_void_v<Score>, Score>;
    static constexpr bool has_score = std::is_void_v<Score>;

    AssociationElements(size_t size, const TDev& dev) : m_data(size, dev) {}

    ALPAKA_FN_HOST_ACC bool isValid(size_t i) {
      if constexpr (has_score) {
        return m_data.view()->scores(i) >= 0.f;
      } else {
        return m_data.view()->values(i) >= 0.f;
      }
    }

    // Enable fraction() if ValueType is FractionType
    template <typename T = V, typename std::enable_if_t<std::is_same_v<T, FractionType>, int> = 0>
    ALPAKA_FN_HOST_ACC float fraction(size_t i) const {}

    // Enable sharedEnergy() if ValueType is SharedEnergyType
    template <typename T = V, typename std::enable_if_t<std::is_same_v<T, SharedEnergyType>, int> = 0>
    ALPAKA_FN_HOST_ACC float sharedEnergy(size_t i) const {}

    template <typename T = Score, typename std::enable_if_t<std::is_void_v<T>, int> = 0>
    ALPAKA_FN_HOST_ACC float score(size_t i) const {}

    // Method to accumulate values
    /*
    void accumulate(const V& other_value) {
      if constexpr (std::is_same_v<V, FractionType> || std::is_same_v<V, SharedEnergyType>) {
        value_.value += other_value.value;
      } else if constexpr (std::is_same_v<V, std::pair<FractionType, float>> ||
                           std::is_same_v<V, std::pair<SharedEnergyType, float>>) {
        value_.first.value += other_value.first.value;
        value_.second += other_value.second;
      }
    }
    bool operator==(const AssociationElement& other) const {
      return index_ == other.index_ && value_.value == other.value_.value;
    }

    bool operator!=(const AssociationElement& other) const { return !(*this == other); }
    */
  };

  template <typename TDev,
            typename V,
            typename Score = void,
            typename Collection1 = void,
            typename Collection2 = void,
            typename = std::enable_if_t<alpaka::isDevice<TDev>>>
  class AssociationMap {
  private:
    AssociationElements<V, Score> m_associations;
    device_buffer<TDev, int[]> m_offsets;
    size_t m_size;  // std::span?

    using CollectionRefProdType =
        typename std::conditional_t<std::is_void_v<Collection1> || std::is_void_v<Collection2>,
                                    std::monostate,
                                    std::pair<edm::RefProd<Collection1>, edm::RefProd<Collection2>>>;

    CollectionRefProdType collectionRefProds;

    using value_type = V;
    static constexpr bool has_score = AssociationElements<V, Score>::has_score;

    // TODO
    //using Traits = MapTraits<MapType>;
    //using AssociationElementType = typename Traits::AssociationElementType;
    //static constexpr bool is_one_to_one = Traits::is_one_to_one;

  public:
    // Constructors for generic use
    AssociationMap(size_t size, size_t nbins, const TDev& dev)
        : m_associations(size, dev),
          m_offsets{make_device_buffer<int[]>(dev, nbins)},
          m_size{nbins},
          collectionRefProds() {}

    template <typename TQueue, typename = std::enable_if_t<alpaka::isQueue<TQueue>>>
    AssociationMap(size_t size, size_t nbins, const TQueue& queue)
        : m_associations(size, queue),
          m_offsets{make_device_buffer<int[]>(queue, nbins)},
          m_size{nbins},
          collectionRefProds() {}

    // Constructor for CMSSW-specific use
    template <typename C1 = Collection1,
              typename C2 = Collection2,
              typename std::enable_if_t<!std::is_void_v<C1> && !std::is_void_v<C2>, int> = 0>
    AssociationMap(size_t size,
                   size_t nbins,
                   const TDev& dev,
                   const edm::RefProd<C1>& id1,
                   const edm::RefProd<C2>& id2,
                   const edm::Event& event)
        : m_associations(size, dev),
          m_offsets{make_device_buffer<int[]>(dev, nbins)},
          m_size{nbins},
          collectionRefProds(std::make_pair(id1, id2)) {
      //resize(event);
    }

    template <typename C1 = Collection1,
              typename C2 = Collection2,
              typename TQueue,
              typename std::enable_if_t<!std::is_void_v<C1> && !std::is_void_v<C2>, int> = 0,
              typename std::enable_if_t<alpaka::isQueue<TQueue>>>
    AssociationMap(size_t size,
                   size_t nbins,
                   const TQueue& queue,
                   const edm::RefProd<C1>& id1,
                   const edm::RefProd<C2>& id2,
                   const edm::Event& event)
        : m_associations(size, queue),
          m_offsets{make_device_buffer<int[]>(queue, nbins)},
          m_size{nbins},
          collectionRefProds(std::make_pair(id1, id2)) {
      //resize(event);
    }

    // Constructor for CMSSW-specific use
    template <typename C1 = Collection1,
              typename C2 = Collection2,
              typename TQueue,
              typename std::enable_if_t<!std::is_void_v<C1> && !std::is_void_v<C2>, int> = 0,
              typename std::enable_if_t<alpaka::isQueue<TQueue>>>
    AssociationMap(size_t size,
                   size_t nbins,
                   const TQueue& queue,
                   const edm::Handle<C1>& handle1,
                   const edm::Handle<C2>& handle2,
                   const edm::Event& event)
        : m_associations(size, queue),
          m_offsets{make_device_buffer<int[]>(queue, nbins)},
          m_size{nbins},
          collectionRefProds(std::make_pair(edm::RefProd<C1>(handle1), edm::RefProd<C2>(handle2))) {
      //resize(event);
    }

    auto size() const { return m_size; }

    device_buffer<TDev, int[]>& offsets() { return m_offsets; }

    // CMSSW-specific method to get references
    template <typename C1 = Collection1,
              typename C2 = Collection2,
              typename std::enable_if_t<!std::is_void_v<C1> && !std::is_void_v<C2>, int> = 0>
    edm::Ref<C1> getRefFirst(unsigned int index) const {
      return edm::Ref<C1>(collectionRefProds.first, index);
    }

    template <typename C1 = Collection1,
              typename C2 = Collection2,
              typename std::enable_if_t<!std::is_void_v<C1> && !std::is_void_v<C2>, int> = 0>
    edm::Ref<C2> getRefSecond(unsigned int index) const {
      return edm::Ref<C2>(collectionRefProds.second, index);
    }

    // Method to get collection IDs for CMSSW-specific use
    template <typename C1 = Collection1,
              typename C2 = Collection2,
              typename std::enable_if_t<!std::is_void_v<C1> && !std::is_void_v<C2>, int> = 0>
    std::pair<const edm::RefProd<C1>, const edm::RefProd<C2>> getCollectionIDs() const {
      return collectionRefProds;
    }

    ALPAKA_FN_ACC void insert(int offset, int index, float fraction_or_energy, float score = 0.0) {
      assert(index1 < m_size);
      if constexpr (has_score) {
        m_associations.view().values[offset] = fraction_or_energy;
        m_associations.view().scores[offset] = score;
        m_associations.view().indexes[offset] = index;
      } else {
        m_associations.view().values[offset] = fraction_or_energy;
        m_associations.view().indexes[offset] = index;
      }
    }
  };

  // TODO
  template <typename TFunc>
  struct KernelComputeAssociations {
    template <typename TAcc>
    ALPAKA_FN_ACC void operator()(
        const TAcc& acc, const int* indexes, size_t size, int* associations, int* nbins, const TFunc* func) const {
      auto max = 0;
      for (auto i : uniform_elements(acc, size)) {
        associations[i] = func(indexes[i]);
        if (associations[i] > max) {
          max = associations[i];
        }
      }
      *nbins = max;
    }
  };

  // Note: bad name. Find a better one.
  struct KernelComputeAssociationSizes {
    template <typename TAcc>
    ALPAKA_FN_ACC void operator()(const TAcc& acc, const int* associations, int* sizes /*tempname*/, size_t size) const {
      for (auto i : uniform_elements(acc, size)) {
        alpaka::atomicAdd(acc, &sizes[associations[i]], 1u);
      }
    }
  };

// TODO manage the score case also in CreateAssociationMap
  template <typename TDev, typename V, typename Score, typename Collection1 = void, typename Collection2 = void>
  struct KernelFillAssociationMap {
    template <typename TAcc>
    ALPAKA_FN_ACC void operator()(const TAcc& acc,
                                  AssociationMap<TDev, V, Score, Collection1, Collection2>* map,
                                  const int* bin_buffer,
                                  const V* values,
                                  int* temp_offsets,
                                  size_t size) const {
      for (auto i : uniform_elements(acc, size)) {
        const auto binId = bin_buffer[i];
        const auto position = alpaka::atomicAdd(acc, &temp_offsets[binId], 1u);
        map->template insert<V, void>(position, i, values[i], 0.0);
      }
    }
  };

  // Note: could also provide a different overload for the case where the associations have already
  // been calculated. In that case, I only need to compute the offsets and fill the map
  template <typename V, typename TFunc,  typename TDev, typename Score, typename = std::enable_if_t<alpaka::isDevice<TDev>>>
  ALPAKA_FN_HOST AssociationMap<TDev, V> CreateAssociationMap(
      const int* indexes, const V* values, size_t size, const TFunc* func, const TDev& dev) {
    auto nbins_buffer = make_device_buffer<int>(dev);
    auto bin_buffer = make_device_buffer<int[]>(dev, size);

    const auto blocksize = 512;
    const auto gridsize = divide_up_by<Acc1D>(size, blocksize);
    const auto workdiv = make_workdiv<Acc1D>(gridsize, blocksize);

    Queue queue{dev};

    alpaka::exec<Acc1D>(
        queue, workdiv, KernelComputeAssociations<TFunc>{}, indexes, size, bin_buffer.data(), nbins_buffer.data(), func);

    auto nbins = *nbins_buffer.data();
    auto sizes_buffer = make_device_buffer<int[]>(dev, nbins);
    alpaka::exec<Acc1D>(queue, workdiv, KernelComputeAssociationSizes{}, bin_buffer.data(), sizes_buffer.data(), nbins);

    // size here should be the max of the associations
    AssociationMap assoc_map(size, nbins, dev);

    // prepare for prefix scan
    auto block_counter = make_device_buffer<int32_t>(queue);
    alpaka::memset(queue, block_counter, 0);

    auto blocksize_multiblockscan = 1024;
    auto gridsize_multiblockscan =
        divide_up_by<Acc1D>(size, blocksize_multiblockscan);  // think about the size
    const auto workdiv_multiblockscan = make_workdiv<Acc1D>(gridsize_multiblockscan, blocksize_multiblockscan);
    auto warp_size = alpaka::getPreferredWarpSize(dev);
    alpaka::exec<Acc1D>(queue,
                        workdiv_multiblockscan,
                        multiBlockPrefixScan<int>{},
                        sizes_buffer.data(),
                        assoc_map.offsets().data(),
                        size,
                        gridsize_multiblockscan,
                        block_counter.data(),
                        warp_size);
    auto temp_offsets = make_device_buffer<int[]>(queue, size);
    alpaka::memcpy(queue, temp_offsets, assoc_map.offsets());
    alpaka::exec<Acc1D>(
        queue, workdiv, KernelFillAssociationMap<TDev, V, Score>{}, &assoc_map, bin_buffer.data(), values, temp_offsets.data(), size);

    return assoc_map;
  }

  // template <typename V, typename TQueue, typename = std::enable_if_t<alpaka::isQueue<TQueue>>>
  // ALPAKA_FN_HOST AssociationMap<TDev, V> CreateAssociationMap(const int* indexes,
  //                                                             const V* values,
  //                                                             size_t size,
  //                                                             const TQueue& queue) {
  // }

}  // namespace ALPAKA_ACCELERATOR_NAMESPACE
