// Necessary includes for an effective Producer file
#include "FWCore/MessageLogger/interface/MessageLogger.h"
//#include "FWCore/Framework/interface/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/global/EDProducer.h"

#include "RecoSoA/SoAProducers/interface/PortableCollectionSoATest.h"
#include <iostream>

class SoAProducer : public edm::global::EDProducer<> {

    public :
        explicit SoAProducer(const edm::ParameterSet&);
        ~SoAProducer();

        void produce(edm::StreamID, edm::Event&, const edm::EventSetup&) const override;

    private:
        int soaParameter_;    
};

// Constructor
SoAProducer::SoAProducer(const edm::ParameterSet& iConfig) {
    std::cout << "I am here" << std::endl;
    //soaParameter_ = iConfig.getParameter<int>("soaParameter");
    // Product produced by the Producer
    produces<SoAHostCollection>("SoAProduct");
}

// Destructor
SoAProducer::~SoAProducer() {}

// Macro to label the label the module
DEFINE_FWK_MODULE(SoAProducer);

// Method to produce
void SoAProducer::produce(edm::StreamID iID, edm::Event& event, const edm::EventSetup& iSetup) const {
    std::size_t elems = 16;
    // SoA producer input
    auto SoAProduct = std::make_unique<SoAHostCollection>(elems, cms::alpakatools::host());

    auto& view = SoAProduct -> view();

    for (size_t i = 0; i < elems; i++) {
          view.x()[i] = static_cast<double>(i);
          view.y()[i] = static_cast<double>(i) * 2.0;
          view.z()[i] = static_cast<double>(i) * 3.0;
          for (size_t j = 0; j < 3; ++j) {
                view[i].a()(j) = 0.1*i*(j+1);
                view[i].b()(j) = 0.2*i*(j+1);
                view[i].r()(j) = 0.3*i*(j+1); 
          }
    }
    view.description() = "Example SoA";
    view.someNumber() = 1; 

    // Put the product in the event
    event.put(std::move(SoAProduct), "SoAProduct");
    std::cout << "Hey I work with SoA" << std::endl;
} 
