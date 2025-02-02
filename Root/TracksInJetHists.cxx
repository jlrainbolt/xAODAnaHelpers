#include "xAODAnaHelpers/TracksInJetHists.h"
#include <math.h>
#include <xAODTracking/TrackParticle.h>
#include "xAODAnaHelpers/HelperFunctions.h"

#pragma GCC diagnostic push // ignore compiler warnings
#pragma GCC diagnostic ignored "-Wunused-result"

ANA_MSG_SOURCE(msgTracksInJetHists, "TracksInJetHists")

TracksInJetHists :: TracksInJetHists (std::string name, std::string detailStr) :
  HistogramManager(name, detailStr)
{
}

TracksInJetHists :: ~TracksInJetHists () {}

StatusCode TracksInJetHists::initialize() {

  //
  // TrackHists
  //
  m_trkPlots = new TrackHists(m_name, "IPDetails HitCounts TPErrors Chi2Details Debugging vsLumiBlock");
  m_trkPlots -> initialize();

  //
  //  d0
  //
  m_trk_d0        = book(m_name, "d0_signed",          "d0[mm] (signed)",   100,  -2.0, 2.0 );
  m_trk_d0Sig     = book(m_name, "d0Sig_signed",       "d0Sig (signed)",   240,   -20.0, 40.0 );
  m_trk_d0SigPDF  = book(m_name, "d0Sig_signed_pdf",   "d0Sig (signed)",   42,    -21.0, 21.0 );
  m_trk_z0sinTd0  = book(m_name, "z0sinTsigned_vs_d0signed",
			 "z0xsin0[mm](signed)", 50, -2.0, 2.0,
			 "d0[mm](signed);",     50, -2.0, 2.0 );


  //
  //  z0
  //
  m_trk_z0_signed        = book(m_name, "z0_signed"   ,     "z0[mm] (signed)",               100,  -5.0, 5.0 );
  m_trk_z0sinT_signed    = book(m_name, "z0sinT_signed",    "z0xsin(#theta)[mm] (signed)",   100,  -2.0, 2.0 );
  m_trk_z0Sig_signed     = book(m_name, "z0Sig_signed",     "z0 significance (signed)",      100,  -25.0, 25.0 );
  m_trk_z0Sig_signed_pdf = book(m_name, "z0Sig_signed_pdf", "z0 significance (signed)",       42,  -21.0, 20.0 );
  m_trk_z0SigsinT_signed = book(m_name, "z0SigsinT_signed", "z0 significance x sin(#theta)", 100,  -25.0, 25.0 );

  //
  //  wrt Jet
  //
  m_trk_jetdPhi = book(m_name, "jetdPhi",  "jetdPhi",  100,  -0.5, 0.5 );
  m_trk_jetdEta = book(m_name, "jetdEta",  "jetdEta",  100,  -0.5, 0.5 );
  m_trk_jetdR      = book(m_name, "jetdR" ,   "jetdR",    300,  -0.1, 0.5 );
  m_trk_jetdR_l    = book(m_name, "jetdR_l",  "jetdR",    300,  -0.1, 1.5 );

  // if worker is passed to the class add histograms to the output
  return StatusCode::SUCCESS;
}


void TracksInJetHists::record(EL::IWorker* wk) {
  HistogramManager::record(wk);
  m_trkPlots -> record( wk );
}



StatusCode TracksInJetHists::execute( const xAOD::TrackParticle* trk, const xAOD::Jet* jet,  const xAOD::Vertex *pvx, float eventWeight,  const xAOD::EventInfo* eventInfo ) {
  using namespace msgTracksInJetHists;
  //
  //  Fill track hists
  //
  ANA_CHECK( m_trkPlots   ->execute(trk, pvx, eventWeight, eventInfo));

  // d0
  float sign         = getD0Sign(trk, jet);
  float d0_wrtPV     = trk->d0();
  float signedD0     = fabs(d0_wrtPV)*sign;
  float d0Err_wrtPV  = sqrt((trk->definingParametersCovMatrixVec().at(0)));
  float d0Sig_wrtPV  = d0Err_wrtPV ? d0_wrtPV/d0Err_wrtPV : -1;
  float d0SigSigned  = sign*fabs(d0Sig_wrtPV);
  m_trk_d0       ->Fill(signedD0,    eventWeight);
  m_trk_d0Sig    ->Fill(d0SigSigned, eventWeight);
  m_trk_d0SigPDF ->Fill(d0SigSigned, eventWeight);

  //
  // Signed Z0
  //
  float signZ0           = getZ0Sign(trk, jet, pvx);

  float z0               = trk->z0() + trk->vz() - HelperFunctions::getPrimaryVertexZ(pvx);

  float z0_wrtPV_signed  = fabs(z0)*signZ0;
  float z0Err            = sqrt((trk->definingParametersCovMatrixVec().at(2)));
  float sinT             = sin(trk->theta());

  m_trk_z0_signed     ->Fill(z0_wrtPV_signed,         eventWeight);
  m_trk_z0sinT_signed ->Fill(z0_wrtPV_signed*sinT,    eventWeight);
  if(z0Err){
    m_trk_z0Sig_signed     ->Fill(z0_wrtPV_signed/z0Err,         eventWeight);
    m_trk_z0Sig_signed_pdf ->Fill(z0_wrtPV_signed/z0Err,         eventWeight);
    m_trk_z0SigsinT_signed ->Fill(z0_wrtPV_signed/z0Err*sinT,    eventWeight);
  }

  m_trk_z0sinTd0->Fill(z0_wrtPV_signed*sinT, signedD0, eventWeight);

  float dEta = trk->eta() - jet->p4().Eta();
  float dPhi = HelperFunctions::dPhi(trk->phi(), jet->p4().Phi());
  float dR   = sqrt(dPhi*dPhi + dEta*dEta);
  //float dR = trk->p4().DeltaR(jet->p4());

  m_trk_jetdPhi ->Fill(HelperFunctions::dPhi(trk->phi(),jet->p4().Phi()), eventWeight);
  m_trk_jetdEta ->Fill(trk->eta() - jet->eta(),       eventWeight);
  m_trk_jetdR   ->Fill(dR,   eventWeight);
  m_trk_jetdR_l ->Fill(dR,   eventWeight);

  return StatusCode::SUCCESS;
}




float TracksInJetHists::getD0Sign(const xAOD::TrackParticle* trk, const xAOD::Jet* jet){
  float trk_phi    = trk->phi();
  float jet_phi    = jet->phi();
  float prod       = cos(trk_phi)*sin(jet_phi) - sin(trk_phi)*cos(jet_phi);
  float sign_prod  = prod > 0 ? 1 : -1;
  float d0_wrtPV   = trk->d0();
  float sign_d0    = d0_wrtPV > 0 ? 1 : -1;
  float sign       = sign_d0 * sign_prod;
  return sign;
}


float TracksInJetHists::getZ0Sign(const xAOD::TrackParticle* trk, const xAOD::Jet* jet, const xAOD::Vertex *pvx){
  float trk_eta  = trk->eta();
  float jetEta   = jet->eta();
  float dEta     = jetEta - trk_eta;

  float trk_z0_wrtPV = trk->z0() + trk->vz() - HelperFunctions::getPrimaryVertexZ(pvx);

  float signZ0 = (trk_z0_wrtPV*dEta) > 0 ? 1.0 : -1.0;
  return signZ0;
}

#pragma GCC diagnostic pop
