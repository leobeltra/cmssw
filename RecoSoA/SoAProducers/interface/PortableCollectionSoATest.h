#ifndef Portable_Collection_SoA_Test_h
#define Portable_Collection_SoA_Test_h

#include "RecoSoA/SoAProducers/interface/SoALayoutTest.h"
#include "DataFormats/Portable/interface/PortableHostCollection.h"

using SoAHostPositionCollection = PortableHostCollection<SoAHostPositionLayout>;
using SoAHostVelocityCollection = PortableHostCollection<SoAHostVelocityLayout>;

using SoAHostYCoordinateCollection = PortableHostCollection<SoAHostYCoordinateLayout>;

// using SoAHostCollection2 = PortableHostCollection2<SoAHostPositionLayout, SoAHostVelocityLayout>;
// using SoAHostCollectionView1 = PortableHostCollection2<SoAHostPositionLayout, SoAHostVelocityLayout>::View<0>;
// using SoAHostCollectionView2 = PortableHostCollection2<SoAHostPositionLayout, SoAHostVelocityLayout>::View<1>;

using SoAHostPositionCollectionView = PortableHostCollection<SoAHostPositionLayout>::View;
using SoAHostCollectionConstView = PortableHostCollection<SoAHostPositionLayout>::ConstView;

using SoAHostVelocityCollectionView = PortableHostCollection<SoAHostVelocityLayout>::View;
using SoAHostVelocityCollectionConstView = PortableHostCollection<SoAHostVelocityLayout>::ConstView;

#endif