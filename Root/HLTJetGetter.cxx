/******************************************
 *
 * This class gets HLT jets from the TDT and can be expanded to get other features
 *
 * Merlin Davies (merlin.davies@cern.ch)
 * Caterina Doglioni (caterina.doglioni@cern.ch)
 * John Alison (john.alison@cern.ch)
 *
 *
 ******************************************/

// c++ include(s):
#include <iostream>
#include <vector>

// EL include(s):
#include <EventLoop/Job.h>
#include <EventLoop/StatusCode.h>
#include <EventLoop/Worker.h>

// EDM include(s):
#include "xAODEventInfo/EventInfo.h"
#include "xAODJet/JetContainer.h"
#include "xAODJet/JetAuxContainer.h"
#include "xAODJet/Jet.h"

// package include(s):
#include "xAODAnaHelpers/HelperFunctions.h"
#include "xAODAnaHelpers/HLTJetGetter.h"
#include "TrigConfxAOD/xAODConfigTool.h"
#include "TrigDecisionTool/TrigDecisionTool.h"

#pragma GCC diagnostic push // ignore compiler warnings
#pragma GCC diagnostic ignored "-Wunused-result"

// this is needed to distribute the algorithm to the workers
ClassImp(HLTJetGetter)

HLTJetGetter :: HLTJetGetter () :
Algorithm("HLTJetGetter")
{
}


EL::StatusCode HLTJetGetter :: setupJob (EL::Job& job)
{
    ANA_MSG_INFO( "Calling setupJob");
    job.useXAOD ();
    xAOD::Init( "HLTJetGetter" ).ignore(); // call before opening first file
    return EL::StatusCode::SUCCESS;
}



EL::StatusCode HLTJetGetter :: histInitialize ()
{
    ANA_CHECK( xAH::Algorithm::algInitialize());
    return EL::StatusCode::SUCCESS;
}



EL::StatusCode HLTJetGetter :: fileExecute ()
{
    return EL::StatusCode::SUCCESS;
}



EL::StatusCode HLTJetGetter :: changeInput (bool /*firstFile*/)
{
    return EL::StatusCode::SUCCESS;
}

EL::StatusCode HLTJetGetter :: initialize ()
{


    ANA_MSG_INFO( "Initializing HLTJetGetter Interface... ");

    m_event = wk()->xaodEvent();
    m_store = wk()->xaodStore();

    //
    // Grab the TrigDecTool from the ToolStore
    //

    /*
    if ( asg::ToolStore::contains<Trig::TrigDecisionTool>( "TrigDecisionTool" ) ) {
        m_trigDecTool = asg::ToolStore::get<Trig::TrigDecisionTool>("TrigDecisionTool");
    } else {
        Info ("Initialize()", "the Trigger Decision Tool is not yet initialized...[%s]. Doing so now.", m_name.c_str());
        m_ownTDTAndTCT = true;

        m_trigConfTool = new TrigConf::xAODConfigTool( "xAODConfigTool" );
        ANA_CHECK( m_trigConfTool->initialize());
        ToolHandle< TrigConf::ITrigConfigTool > configHandle( m_trigConfTool );

        m_trigDecTool = new Trig::TrigDecisionTool( "TrigDecisionTool" );
        ANA_CHECK( m_trigDecTool->setProperty( "ConfigTool", configHandle ));
        ANA_CHECK( m_trigDecTool->setProperty( "TrigDecisionKey", "xTrigDecision" ));
        ANA_CHECK( m_trigDecTool->setProperty( "OutputLevel", MSG::ERROR));
        ANA_CHECK( m_trigDecTool->initialize());
        ANA_MSG_INFO( "Successfully configured Trig::TrigDecisionTool!");
    }
    */

    if(!m_trigDecTool_handle.isUserConfigured()){
      ANA_MSG_FATAL("A configured " << m_trigDecTool_handle.typeAndName() << " must have been previously created! Are you creating one in xAH::BasicEventSelection?" );
      return EL::StatusCode::FAILURE;
    }

    // If there is no InputContainer we must stop
    if ( m_inContainerName.empty() ) {
        ANA_MSG_ERROR( "InputContainer is empty!");
        return EL::StatusCode::FAILURE;
    }

    return EL::StatusCode::SUCCESS;
}


EL::StatusCode HLTJetGetter :: execute ()
{
    ANA_MSG_DEBUG( "Getting HLT jets... ");

    //
    // Create the new container and its auxiliary store.
    //
    xAOD::JetContainer*     hltJets    = new xAOD::JetContainer();
    xAOD::JetAuxContainer*  hltJetsAux = new xAOD::JetAuxContainer();
    hltJets->setStore( hltJetsAux ); //< Connect the two

    //Retrieving jets via trigger decision tool:
    const Trig::ChainGroup * chainGroup = m_trigDecTool_handle->getChainGroup(m_triggerList); //Trigger list:
    
    std::vector<std::string> trigger_list = chainGroup->getListOfTriggers();

    auto chainFeatures = chainGroup->features(); //Gets features associated to chain defined above

    auto JetFeatureContainers = chainFeatures.containerFeature<xAOD::JetContainer>(m_inContainerName.c_str());

    ANA_CHECK( m_store->record( hltJets,    m_outContainerName));
    ANA_CHECK( m_store->record( hltJetsAux, m_outContainerName+"Aux."));

    for( auto fContainer : JetFeatureContainers ) {
        for( auto trigJet : *fContainer.cptr() ) {
            xAOD::Jet *Jet = new xAOD::Jet();
            *Jet = *trigJet;
            hltJets->push_back( Jet );
        }//end trigJet loop
    }//end feature container loop
    
    if(msgLvl(MSG::VERBOSE)) m_store->print();

    return EL::StatusCode::SUCCESS;
}



EL::StatusCode HLTJetGetter :: postExecute ()
{
    ANA_MSG_DEBUG( "Calling postExecute");
    return EL::StatusCode::SUCCESS;
}



EL::StatusCode HLTJetGetter :: finalize ()
{
    ANA_MSG_INFO( "Deleting tool instances...");

    // this is necessary because in most cases the pointer will be set to null
    // after deletion in BasicEventSelection, but it will not propagate here
    if ( m_ownTDTAndTCT ) {
      //if ( m_trigDecTool_handle )  { delete m_trigDecTool_handle; m_trigDecTool_handle = nullptr;  }
      if ( m_trigDecTool_handle.isInitialized() )  m_trigDecTool_handle->finalize();
      if ( m_trigConfTool ) {  delete m_trigConfTool; m_trigConfTool = nullptr; }
    }

    return EL::StatusCode::SUCCESS;
}



EL::StatusCode HLTJetGetter :: histFinalize ()
{
    ANA_MSG_INFO( "Calling histFinalize");
    ANA_CHECK( xAH::Algorithm::algFinalize());
    return EL::StatusCode::SUCCESS;
}

#pragma GCC diagnostic pop
