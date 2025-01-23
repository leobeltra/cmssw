#include <alpaka/alpaka.hpp>

#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/Framework/interface/global/EDProducer.h"
#include "DataFormats/Portable/interface/PortableCollection.h"
#include "DataFormats/NGTSoATest/interface/SoALayoutTest.h"
#include "DataFormats/NGTSoATest/interface/PortableCollectionSoATest.h"
#include "DataFormats/NGTSoATest/interface/alpaka/NGTSoACollection.h"
#include "HeterogeneousCore/AlpakaCore/interface/alpaka/global/EDProducer.h"
#include "HeterogeneousCore/AlpakaCore/interface/alpaka/Event.h"
#include "HeterogeneousCore/AlpakaCore/interface/alpaka/EventSetup.h"
#include "HeterogeneousCore/AlpakaCore/interface/alpaka/EDPutToken.h"
#include "HeterogeneousCore/AlpakaCore/interface/alpaka/EDGetToken.h"
#include "HeterogeneousCore/AlpakaCore/interface/alpaka/stream/SynchronizingEDProducer.h"
#include "HeterogeneousCore/AlpakaInterface/interface/config.h"
#include "HeterogeneousCore/AlpakaCore/interface/alpaka/MakerMacros.h"
#include <iostream>

namespace ALPAKA_ACCELERATOR_NAMESPACE {

  class SoAProducer4 : public stream::SynchronizingEDProducer<> {
  public:
    // Constructor
    SoAProducer4(edm::ParameterSet const& iConfig)
        : inputToken_{consumes<CombinedPhysicsObjectCollection>(iConfig.getParameter<edm::InputTag>("soaInput_2"))},
          outputToken_{produces()} {}

    // Method to produce in SoAProducer4
    void produce(device::Event& event, device::EventSetup const&) override {
      auto const& aggregated_collection = event.getHandle(inputToken_);  // Combined Collection

      int elems = aggregated_collection->view().metadata().size();
      std::cout << "SoAProducer4: Number of elements in the input collection: " << elems << std::endl;

      auto& queue = event.queue();

      // std::cout << typeid(decltype(event.queue())).name() << std::endl;

      auto deviceCollection = std::make_unique<NGTSoACollection>(elems, queue);
      // Copia i dati dalla collezione host al dispositivo
      // auto deviceData = cms::alpakatools::CopyToDevice<CombinedPhysicsObjectCollection>::copyAsync(queue, *aggregated_collection);      

      assert(deviceCollection->buffer().data() != nullptr);
      assert(aggregated_collection->buffer().data() != nullptr);

      assert(aggregated_collection->view().metadata().size() == deviceCollection->view().metadata().size());
      // auto device = alpaka::getDev(queue);
      // std::cout << "The device is: " << device << std::endl;

      alpaka::memcpy(queue, deviceCollection -> buffer(), aggregated_collection -> buffer());

      alpaka::wait(queue);

      // printSoAView(deviceCollection -> view());    // It works only if backend is serial_sync  

      event.emplace(outputToken_, std::move(*deviceCollection));
    }

    void acquire(device::Event const& event, device::EventSetup const& setup) override {}

    static void fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
      edm::ParameterSetDescription desc;
      desc.add<edm::InputTag>("soaInput_2", edm::InputTag{"CombinedObjCollection"});
      descriptions.addWithDefaultLabel(desc);
    }

  private:
    const edm::EDGetTokenT<CombinedPhysicsObjectCollection> inputToken_;
    const device::EDPutToken<NGTSoACollection> outputToken_;
  };
}  // namespace ALPAKA_ACCELERATOR_NAMESPACE

DEFINE_FWK_ALPAKA_MODULE(SoAProducer4);