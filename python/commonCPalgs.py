#from AnaAlgorithm.AlgSequence import AlgSequence
from AnaAlgorithm.AnaAlgSequence import AnaAlgSequence
from AnaAlgorithm.DualUseConfig import createAlgorithm
from AnaAlgorithm.DualUseConfig import createService
import ROOT

def makeSequence (dataType) :

    #algSeq = AlgSequence()
    algSeq = AnaAlgSequence()
    
    sysService = createService( 'CP::SystematicsSvc', 'SystematicsSvc', sequence = algSeq )
    #sysService.systematicsList= ['NOSYS']
    sysService.sigmaRecommended = 1
    
    #  Include, and then set up the pileup analysis sequence:
    from AsgAnalysisAlgorithms.PileupAnalysisSequence import makePileupAnalysisSequence
    pileupSequence = makePileupAnalysisSequence( dataType )
    pileupSequence.configure( inputName = 'EventInfo', outputName = 'EventInfo_%SYS%' )
    
    #  Add the pileup sequence to the job:
    algSeq += pileupSequence
    
    from MuonAnalysisAlgorithms.MuonAnalysisSequence import makeMuonAnalysisSequence
    muonSequenceLoose = makeMuonAnalysisSequence( dataType, deepCopyOutput = True, shallowViewOutput = False,
                                                   workingPoint = 'Loose.NonIso', postfix = 'loose' )
    muonSequenceLoose.configure( inputName = 'Muons', outputName = 'AnalysisMuons_%SYS%' )
    algSeq += muonSequenceLoose
    
    from EgammaAnalysisAlgorithms.ElectronAnalysisSequence import  makeElectronAnalysisSequence
    electronSequence = makeElectronAnalysisSequence( dataType, 'LooseLHElectron.NonIso', shallowViewOutput = False, deepCopyOutput = True )
    electronSequence.configure( inputName = 'Electrons', outputName = 'AnalysisElectrons_%SYS%' )
    #print( electronSequence ) # For debugging
    algSeq += electronSequence
    
    from EgammaAnalysisAlgorithms.PhotonAnalysisSequence import  makePhotonAnalysisSequence
    photonSequence = makePhotonAnalysisSequence( dataType, 'Loose.Undefined', deepCopyOutput = False, recomputeIsEM=False )
    photonSequence.configure( inputName = 'Photons', outputName = 'AnalysisPhotons_%SYS%' )
    #print( photonSequence ) # For debugging
    algSeq += photonSequence
    
    from TauAnalysisAlgorithms.TauAnalysisSequence import makeTauAnalysisSequence
    tauSequence = makeTauAnalysisSequence( dataType, 'Baseline', shallowViewOutput = False, rerunTruthMatching = False, deepCopyOutput = True )
    tauSequence.configure( inputName = 'TauJets', outputName = 'AnalysisTauJets_%SYS%' )
    algSeq += tauSequence

    print( tauSequence ) # For debugging                                                                               
    
    jetContainer = 'AntiKt4EMPFlowJets'
    from JetAnalysisAlgorithms.JetAnalysisSequence import makeJetAnalysisSequence
    jetSequence = makeJetAnalysisSequence( dataType, jetContainer, deepCopyOutput = True, shallowViewOutput = False, runGhostMuonAssociation = False, runFJvtUpdate = False, runFJvtSelection = False, runJvtSelection = False)
    
    from FTagAnalysisAlgorithms.FTagAnalysisSequence import makeFTagAnalysisSequence
    ftagSequence = makeFTagAnalysisSequence( jetSequence, dataType, jetContainer,
                              btagWP = "FixedCutBEff_77",
                              btagger = "DL1r",
                              postfix = "",
                              preselection=None,
                              kinematicSelection = False,
                              noEfficiency = False,
                              legacyRecommendations = True,
                              enableCutflow = False,
                              minPt = 20000 )    
    
    jetSequence.configure( inputName = jetContainer, outputName = 'AnalysisJetsBTAG_%SYS%')
    #print( jetSequence ) # For debugging
    algSeq += jetSequence
    
    from MetAnalysisAlgorithms.MetAnalysisSequence import makeMetAnalysisSequence
    metSequence = makeMetAnalysisSequence( dataType, metSuffix = "AntiKt4EMPFlow")
    metSequence.configure( inputName = { 'jets'      : 'AnalysisJetsBTAG_%SYS%',
                                         'muons'     : 'AnalysisMuons_%SYS%',
                                         'taus'      : 'AnalysisTauJets_%SYS%',
                                         'electrons' : 'AnalysisElectrons_%SYS%',
                                         'photons'   : 'AnalysisPhotons_%SYS%'
                                       },
                           outputName = 'AnalysisMET_%SYS%' )

    # Add the sequence to the job:
    algSeq += metSequence
    
    
    #Add an ntuple dumper algorithm:
    """
    treeMaker = createAlgorithm( 'CP::TreeMakerAlg', 'TreeMaker' )
    treeMaker.TreeName = 'tree'
    algSeq += treeMaker
    ntupleMaker = createAlgorithm( 'CP::AsgxAODNTupleMakerAlg', 'NTupleMakerEventInfo' )
    ntupleMaker.TreeName = 'tree'
    ntupleMaker.Branches = [ 'EventInfo.runNumber     -> runNumber',
                             'EventInfo.eventNumber   -> eventNumber', ]
    ntupleMaker.Branches += [ 'AnalysisMuons_%SYS%.pt  -> mu_%SYS%_pt',
                             #'AnalysisMuons_NOSYS.pt  -> mu_pt',
                             #'AnalysisElectrons_NOSYS.eta -> el_eta',
                             #'AnalysisElectrons_NOSYS.phi -> el_phi',
                             #'AnalysisElectrons_NOSYS.pt  -> el_pt',
                             #'AnalysisTauJets_NOSYS.eta -> tau_eta',
                             #'AnalysisTauJets_NOSYS.phi -> tau_phi',
                             #'AnalysisTauJets_NOSYS.pt  -> tau_pt',
                             #'AnalysisJets_NOSYS.eta -> jet_eta',
                             #'AnalysisJets_NOSYS.phi -> jet_phi',
                             #'AnalysisJets_NOSYS.pt  -> jet_pt' 
                           ]
    #ntupleMaker.systematicsRegex = '(^MUON_.*)'
    algSeq += ntupleMaker
    treeFiller = createAlgorithm( 'CP::TreeFillerAlg', 'TreeFiller' )
    treeFiller.TreeName = 'tree'
    algSeq += treeFiller
    """
    
    return algSeq


