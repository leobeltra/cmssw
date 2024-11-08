import FWCore.ParameterSet.Config as cms

process = cms.Process("SoAProducer")

process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = 1

# Number of events
process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(4))

# Empty source
process.source = cms.Source("EmptySource")

# Add the producer
process.soaproducer = cms.EDProducer("SoAProducer", soaParameter = cms.int32(42))
process.soaproducer2 = cms.EDProducer("SoAProducer2", soaInput = cms.InputTag("soaproducer", "SoAProduct"))
process.soaproducer3 = cms.EDProducer("SoAProducer3", soaInput_0 = cms.InputTag("soaproducer", "SoAProduct"), soaInput_1 = cms.InputTag("soaproducer2", "SoAProduct2"))

# Add to process path
process.p = cms.Path(process.soaproducer * process.soaproducer2 * process.soaproducer3
                     )