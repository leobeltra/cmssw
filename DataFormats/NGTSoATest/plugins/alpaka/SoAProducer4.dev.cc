// Necessary includes
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/Framework/interface/global/EDProducer.h"
#include "DataFormats/NGTSoATest/interface/PortableCollectionSoATest.h"
#include "DataFormats/NGTSoATest/interface/alpaka/NGTSoACollection.h"
#include "HeterogeneousCore/AlpakaCore/interface/alpaka/global/EDProducer.h"
#include "HeterogeneousCore/AlpakaCore/interface/alpaka/Event.h"
#include "HeterogeneousCore/AlpakaCore/interface/alpaka/EventSetup.h"
#include "HeterogeneousCore/AlpakaCore/interface/alpaka/EDPutToken.h"
#include "HeterogeneousCore/AlpakaCore/interface/alpaka/EDGetToken.h"
#include "HeterogeneousCore/AlpakaCore/interface/alpaka/stream/SynchronizingEDProducer.h"
#include "HeterogeneousCore/AlpakaInterface/interface/config.h"
#include <iostream>

namespace ALPAKA_ACCELERATOR_NAMESPACE {

class SoAProducer4 : public stream::SynchronizingEDProducer<> {

public:
// Constructor
SoAProducer4(edm::ParameterSet const& iConfig) : inputToken_{consumes<CombinedPhysicsObjectCollection>(iConfig.getParameter<edm::InputTag>("soaInput_2"))}, outputToken_{produces()} {}

// Method to produce in SoAProducer4
void produce(device::Event& event, device::EventSetup const&) override {
    auto const& PortableCollection_0 = event.getHandle(inputToken_); // Combined Collection

    size_t elems = PortableCollection_0->view().metadata().size();

    event.emplace(outputToken_, NGTSoACollection{elems, event.queue()});
}

void acquire(device::Event const& event, device::EventSetup const& setup) override {}

static void fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
    edm::ParameterSetDescription desc;
    desc.add<edm::InputTag>("soaInput_2", edm::InputTag{"CombinedObjCollection"});
    descriptions.addWithDefaultLabel(desc);
    }

private:
    edm::EDGetTokenT<CombinedPhysicsObjectCollection> inputToken_;
    device::EDPutToken<NGTSoACollection> outputToken_;
};
}

#include "HeterogeneousCore/AlpakaCore/interface/alpaka/MakerMacros.h"
DEFINE_FWK_ALPAKA_MODULE(SoAProducer4);