// Necessary includes
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/global/EDProducer.h"
#include "DataFormats/NGTSoATest/interface/PortableCollectionSoATest.h"
#include <iostream>

class SoAProducer3 : public edm::global::EDProducer<> {

public:
    explicit SoAProducer3(const edm::ParameterSet&);
    ~SoAProducer3();

    void produce(edm::StreamID, edm::Event&, const edm::EventSetup&) const override;

private:
    edm::EDGetTokenT<PhysicsObjCollection> inputToken_0_;
    edm::EDGetTokenT<PhysicsObjExtraCollection> inputToken_1_;
};

// Constructor
SoAProducer3::SoAProducer3(const edm::ParameterSet& iConfig) : inputToken_0_(consumes<PhysicsObjCollection>(iConfig.getParameter<edm::InputTag>("soaInput_0"))), 
                                                               inputToken_1_(consumes<PhysicsObjExtraCollection>(iConfig.getParameter<edm::InputTag>("soaInput_1")))
{
    produces<CombinedPhysicsObjectCollection>("SoAProduct3");
}

// Destructor
SoAProducer3::~SoAProducer3() {}

// Macro to label the module
DEFINE_FWK_MODULE(SoAProducer3);

// Method to produce in SoAProducer3
void SoAProducer3::produce(edm::StreamID iID, edm::Event& event, const edm::EventSetup& iSetup) const {
    // Retrieve the first and second collections (position and velocity)
    auto const& PortableCollection_0 = event.getHandle(inputToken_0_); // Position Collection
    auto const& PortableCollection_1 = event.getHandle(inputToken_1_); // Velocity Collection

    // Creating a PortableCollection for the output (memory not allocated here)
    // int elems = PortableCollection_0->view().metadata().size();

    // auto CombinedPhysicsObjSoAColl = std::make_unique<CombinedPhysicsObjectCollection>(elems, cms::alpakatools::host());

    auto physicsObj = PortableCollection_0 -> layout();
    auto physicsObjExtra = PortableCollection_1 -> layout();

    const auto phObj = physicsObj.records();
    const auto phObjExtra = physicsObjExtra.records();

    CombinedPhysicsObjectView combinedPhysicsObjSoAView(phObj.x(),
                                                        phObj.y(),
                                                        phObj.z(),
                                                        phObjExtra.candidateDirection());

    auto combinedPhysicsObjSoAColl = std::make_unique<CombinedPhysicsObjectCollection>(
                     combinedPhysicsObjSoAView.metadata().size(), cms::alpakatools::host());

    combinedPhysicsObjSoAColl -> aggregate(combinedPhysicsObjSoAView);

    std::cout << "Hey, I modified SoA for second time!" << std::endl;
    // std::cout << "Memory address of the aggregate collection: " << combinedPhysicsObjSoAColl -> view().metadata().addressOf_x() << std::endl;
    std::cout << (PortableCollection_0 -> view().metadata().addressOf_x() == combinedPhysicsObjSoAColl -> view().metadata().addressOf_x() ? "the addresses are the same" : "the addresses are not the same") << std::endl;
    printSoAView(combinedPhysicsObjSoAColl -> view());

    event.put(std::move(combinedPhysicsObjSoAColl), "SoAProduct3");
    //printSoAView(modifiedView);
}