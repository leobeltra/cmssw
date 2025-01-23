#include "DataFormats/Portable/interface/PortableHostCollectionReadRules.h"
#include "DataFormats/NGTSoATest/interface/PortableCollectionSoATest.h"
#include "DataFormats/NGTSoATest/interface/PositionSoACollection.h"
#include "DataFormats/NGTSoATest/interface/AlgebraSoACollection.h"
#include "DataFormats/NGTSoATest/interface/NGTSoAHostCollection.h"
#include "DataFormats/NGTSoATest/interface/SoALayoutTest.h"

SET_PORTABLEHOSTCOLLECTION_READ_RULES(PositionSoACollection);
SET_PORTABLEHOSTCOLLECTION_READ_RULES(AlgebraSoACollection);
SET_PORTABLEHOSTCOLLECTION_READ_RULES(CombinedPhysicsObjectCollection);
SET_PORTABLEHOSTCOLLECTION_READ_RULES(NGTSoAHostCollection);