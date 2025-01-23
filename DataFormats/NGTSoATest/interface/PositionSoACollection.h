#ifndef Position_SoA_Collection_h
#define Position_SoA_Collection_h

#include "DataFormats/NGTSoATest/interface/PositionSoA.h"
#include "DataFormats/Portable/interface/PortableHostCollection.h"

using PositionSoACollection = PortableHostCollection<PositionSoA>;
using PositionSoACollectionView = PortableHostCollection<PositionSoA>::View;
using PositionSoACollectionConstView = PortableHostCollection<PositionSoA>::ConstView;

#endif