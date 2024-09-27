#ifndef Portable_Collection_SoA_Test_h
#define Portable_Collection_SoA_Test_h

#include "RecoSoA/SoAProducers/interface/SoALayoutTest.h"
#include "DataFormats/Portable/interface/PortableHostCollection.h"

using SoAHostCollection = PortableHostCollection<SoAHostLayout>;
using SoAHostCollectionView = PortableHostCollection<SoAHostLayout>::View;
using SoAHostCollectionConstView = PortableHostCollection<SoAHostLayout>::ConstView;

#endif
