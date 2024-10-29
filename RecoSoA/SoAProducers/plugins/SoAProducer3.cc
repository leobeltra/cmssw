// // Necessary includes
// #include "FWCore/MessageLogger/interface/MessageLogger.h"
// #include "FWCore/Framework/interface/Event.h"
// #include "FWCore/Framework/interface/MakerMacros.h"
// #include "FWCore/Framework/interface/global/EDProducer.h"
// #include "RecoSoA/SoAProducers/interface/PortableCollectionSoATest.h"
// #include <iostream>

// using ColumnXType = SoAHostPositionCollectionView::template element_type<0>;

// class SoAProducer3 : public edm::global::EDProducer<> {

// public:
//     explicit SoAProducer3(const edm::ParameterSet&);
//     ~SoAProducer3();

//     void produce(edm::StreamID, edm::Event&, const edm::EventSetup&) const override;

// private:
//     edm::EDGetTokenT<SoAHostPositionCollection> inputToken_0_;
//     edm::EDGetTokenT<SoAHostVelocityCollection> inputToken_1_;
// };

// // Constructor
// SoAProducer3::SoAProducer3(const edm::ParameterSet& iConfig) : inputToken_0_(consumes<SoAHostPositionCollection>(iConfig.getParameter<edm::InputTag>("soaInput_0"))), 
//                                                                inputToken_1_(consumes<SoAHostVelocityCollection>(iConfig.getParameter<edm::InputTag>("soaInput_1")))
// {
//     produces<SoAHostXYCoordinatesView>("SoAProduct3");
// }

// // Destructor
// SoAProducer3::~SoAProducer3() {}

// // Macro to label the module
// DEFINE_FWK_MODULE(SoAProducer3);

// // Method to produce in SoAProducer3
// void SoAProducer3::produce(edm::StreamID iID, edm::Event& event, const edm::EventSetup& iSetup) const {
//     // Retrieve the first and second collections (position and velocity)
//     auto const& PortableCollection_0 = event.getHandle(inputToken_0_); // Position Collection
//     auto const& PortableCollection_1 = event.getHandle(inputToken_1_); // Velocity Collection

//     // Creating a PortableCollection for the output (memory not allocated here)
//     std::size_t elems = PortableCollection_0->view().metadata().size();





//     auto myView = std::make_unique<SoAHostXYCoordinatesView>
//                     (PortableCollection_0, {"y"}, PortableCollection_1, {"v_y"}, elems);




//     //auto myView = std::make_unique<SoAHostXYCoordinatesView>(PortableCollection_0, PortableCollection_1);
//     // Dynamically allocate columns
//     // myView.addColumns(PortableCollection_1, "v_y");
//     //event.put(std::move(modifiedSoA), "SoAProduct3");

//     std::cout << "Hey, I modified SoA for second time!" << std::endl;
//     //printSoAView(view);
//     //printSoAView(modifiedView);
// }