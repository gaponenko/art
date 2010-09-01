# Unit test configuration file for MessageLogger service:
# LogVerbatim and LogTrace

import FWCore.ParameterSet.python.Config as cms

process = cms.Process("TEST")

import FWCore.Framework.python.test.cmsExceptionsFatal_cff
process.options = FWCore.Framework.python.test.cmsExceptionsFatal_cff.options

process.load("FWCore.MessageService.python.test.Services_cff")

process.MessageLogger = cms.Service("MessageLogger",
    u13_infos = cms.untracked.PSet(
        threshold = cms.untracked.string('INFO'),
        noTimeStamps = cms.untracked.bool(True),
        FwkReport = cms.untracked.PSet(
            limit = cms.untracked.int32(0)
        ),
        preEventProcessing = cms.untracked.PSet(
            limit = cms.untracked.int32(0)
        ),
        FwkJob = cms.untracked.PSet(
            limit = cms.untracked.int32(0)
        )
    ),
    debugModules = cms.untracked.vstring('*'),
    u13_debugs = cms.untracked.PSet(
        threshold = cms.untracked.string('DEBUG'),
        noTimeStamps = cms.untracked.bool(True),
        FwkReport = cms.untracked.PSet(
            limit = cms.untracked.int32(0)
        ),
        preEventProcessing = cms.untracked.PSet(
            limit = cms.untracked.int32(0)
        ),
        FwkJob = cms.untracked.PSet(
            limit = cms.untracked.int32(0)
        )
    ),
    categories = cms.untracked.vstring('preEventProcessing',
        'FwkReport',
        'FwkJob'),
    destinations = cms.untracked.vstring('u13_infos',
        'u13_debugs')
)

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(2)
)

process.source = cms.Source("EmptySource")

process.sendSomeMessages = cms.EDAnalyzer("UnitTestClient_H")

process.p = cms.Path(process.sendSomeMessages)


