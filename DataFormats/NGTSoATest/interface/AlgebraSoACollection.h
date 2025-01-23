#ifndef Algebra_SoA_Collection_h
#define Algebra_SoA_Collection_h

#include "DataFormats/NGTSoATest/interface/AlgebraSoA.h"
#include "DataFormats/Portable/interface/PortableHostCollection.h"

using AlgebraSoACollection = PortableHostCollection<AlgebraSoA>;
using AlgebraSoACollectionView = PortableHostCollection<AlgebraSoA>::View;
using AlgebraSoACollectionConstView = PortableHostCollection<AlgebraSoA>::ConstView;

#endif