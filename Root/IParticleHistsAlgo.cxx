#include <EventLoop/Job.h>
#include <EventLoop/StatusCode.h>
#include <EventLoop/Worker.h>

#include <xAODBase/IParticleContainer.h>
#include <xAODEventInfo/EventInfo.h>
#include <AthContainers/ConstDataVector.h>

#include <xAODAnaHelpers/IParticleHistsAlgo.h>
#include <xAODAnaHelpers/HelperFunctions.h>
#include <xAODAnaHelpers/HelperClasses.h>

#pragma GCC diagnostic push // ignore compiler warnings
#pragma GCC diagnostic ignored "-Wunused-result"

// this is needed to distribute the algorithm to the workers
ClassImp(IParticleHistsAlgo)

IParticleHistsAlgo :: IParticleHistsAlgo (std::string className) :
    Algorithm(className)
{
}

EL::StatusCode IParticleHistsAlgo :: setupJob (EL::Job& job)
{
  job.useXAOD();
  xAOD::Init("IParticleHistsAlgo").ignore();

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode IParticleHistsAlgo :: histInitialize ()
{

  ANA_MSG_INFO( m_name );
  ANA_CHECK( xAH::Algorithm::algInitialize());
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode IParticleHistsAlgo::AddHists( std::string name ) {
  std::string fullname(m_name);
  fullname += name; // add systematic
  IParticleHists* particleHists = new IParticleHists( fullname, m_detailStr, m_histPrefix, m_histTitle ); // add systematic
  particleHists->m_debug = msgLvl(MSG::DEBUG);
  ANA_CHECK( particleHists->initialize());
  particleHists->record( wk() );
  m_plots[name] = particleHists;

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode IParticleHistsAlgo :: fileExecute () { return EL::StatusCode::SUCCESS; }
EL::StatusCode IParticleHistsAlgo :: changeInput (bool /*firstFile*/) { return EL::StatusCode::SUCCESS; }

EL::StatusCode IParticleHistsAlgo :: initialize ()
{
  ANA_MSG_DEBUG( m_name);

  // in case anything was missing or blank...
  if( m_inContainerName.empty() || m_detailStr.empty() ){
    ANA_MSG_ERROR( "One or more required configuration values are empty");
    return EL::StatusCode::FAILURE;
  }


  // only running 1 collection
  if(m_inputAlgo.empty()) { AddHists( "" ); }
  m_event = wk()->xaodEvent();
  m_store = wk()->xaodStore();
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode IParticleHistsAlgo :: execute ()
{
  return execute<IParticleHists, xAOD::IParticleContainer>();
}

EL::StatusCode IParticleHistsAlgo :: postExecute () { return EL::StatusCode::SUCCESS; }

EL::StatusCode IParticleHistsAlgo :: finalize () {
  ANA_MSG_DEBUG( m_name );
  for( auto plots : m_plots ) {
    if(plots.second){
      plots.second->finalize();
      delete plots.second;
    }
  }
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode IParticleHistsAlgo :: histFinalize () {
  ANA_CHECK( xAH::Algorithm::algFinalize());
  return EL::StatusCode::SUCCESS;
}

#pragma GCC diagnostic pop
