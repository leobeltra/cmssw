import FWCore.ParameterSet.Config as cms

process = cms.Process("SoAProducer")

process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = 1

# Number of events
process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(10))

# Empty source
process.source = cms.Source("EmptySource")

# Add the producer
process.soaproducer = cms.EDProducer("SoAProducer", soaParameter = cms.int32(42))

# Add to process path
process.p = cms.Path(process.soaproducer)
