/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005 INRIA
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as 
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#include "ovnis-adhoc-wifi-mac.h"
#include "ns3/dca-txop.h"
#include "ns3/mac-low.h"
#include "ns3/mac-rx-middle.h"
#include "ns3/wifi-phy.h"
#include "ns3/dcf-manager.h"
#include "ns3/adhoc-wifi-mac.h"
#include "ns3/pointer.h"
#include "ns3/packet.h"
#include "ns3/log.h"
#include "ns3/trace-source-accessor.h"

NS_LOG_COMPONENT_DEFINE ("OvnisAdhocWifiMac");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (OvnisAdhocWifiMac);

#undef NS_LOG_APPEND_CONTEXT
#define NS_LOG_APPEND_CONTEXT if (m_low != 0) {std::clog << "[mac=" << m_low->GetAddress () << "] ";}

TypeId 
OvnisAdhocWifiMac::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::OvnisAdhocWifiMac")
    .SetParent<WifiMac> ()
    .AddConstructor<OvnisAdhocWifiMac> ()
    .AddAttribute ("DcaTxop", "The DcaTxop object",
                   PointerValue (),
                   MakePointerAccessor (&OvnisAdhocWifiMac::GetDcaTxop),
                   MakePointerChecker<DcaTxop> ()) 
    .AddTraceSource ( "TxOkHeader",
                      "The header of successfully transmitted packet",
                      MakeTraceSourceAccessor (&OvnisAdhocWifiMac::m_txOkCallback))
    .AddTraceSource ("TxErrHeader",
                     "The header of unsuccessfully transmitted packet",
                     MakeTraceSourceAccessor (&OvnisAdhocWifiMac::m_txErrCallback))
    ;
  return tid;
}

OvnisAdhocWifiMac::OvnisAdhocWifiMac ()
{
  m_rxMiddle = new MacRxMiddle ();
  m_rxMiddle->SetForwardCallback (MakeCallback (&OvnisAdhocWifiMac::ForwardUp, this));

  m_low = CreateObject<MacLow> ();
  m_low->SetRxCallback (MakeCallback (&MacRxMiddle::Receive, m_rxMiddle));
  m_low->SetBssid (GetBssid ());

  m_dcfManager = new DcfManager ();
  m_dcfManager->SetupLowListener (m_low);

  m_dca = CreateObject<DcaTxop> ();
  m_dca->SetLow (m_low);
  m_dca->SetManager (m_dcfManager);
  m_dca->SetTxFailedCallback (MakeCallback (&OvnisAdhocWifiMac::TxFailed, this));
}
OvnisAdhocWifiMac::~OvnisAdhocWifiMac ()
{}

void
OvnisAdhocWifiMac::DoDispose (void)
{
  delete m_rxMiddle;
  delete m_dcfManager;
  m_rxMiddle = 0;
  m_dcfManager = 0;
  m_low = 0;
  m_phy = 0;
  m_dca = 0;
  m_stationManager = 0;
  WifiMac::DoDispose ();
}

void 
OvnisAdhocWifiMac::SetSlot (Time slotTime)
{
  m_dcfManager->SetSlot (slotTime);
  m_low->SetSlotTime (slotTime);
}
void 
OvnisAdhocWifiMac::SetSifs (Time sifs)
{
  m_dcfManager->SetSifs (sifs);
  m_low->SetSifs (sifs);
}
void 
OvnisAdhocWifiMac::SetEifsNoDifs (Time eifsNoDifs)
{
  m_dcfManager->SetEifsNoDifs (eifsNoDifs);
}
void 
OvnisAdhocWifiMac::SetAckTimeout (Time ackTimeout)
{
  m_low->SetAckTimeout (ackTimeout);
}
void 
OvnisAdhocWifiMac::SetCtsTimeout (Time ctsTimeout)
{
  m_low->SetCtsTimeout (ctsTimeout);
}
void 
OvnisAdhocWifiMac::SetPifs (Time pifs)
{
  m_low->SetPifs (pifs);
}
Time 
OvnisAdhocWifiMac::GetSlot (void) const
{
  return m_low->GetSlotTime ();
}
Time 
OvnisAdhocWifiMac::GetSifs (void) const
{
  return m_low->GetSifs ();
}
Time 
OvnisAdhocWifiMac::GetEifsNoDifs (void) const
{
  return m_dcfManager->GetEifsNoDifs ();
}
Time 
OvnisAdhocWifiMac::GetAckTimeout (void) const
{
  return m_low->GetAckTimeout ();
}
Time 
OvnisAdhocWifiMac::GetCtsTimeout (void) const
{
  return m_low->GetCtsTimeout ();
}
Time 
OvnisAdhocWifiMac::GetPifs (void) const
{
  return m_low->GetPifs ();
}
void 
OvnisAdhocWifiMac::SetWifiPhy (Ptr<WifiPhy> phy)
{
  m_phy = phy;
  m_dcfManager->SetupPhyListener (phy);
  m_low->SetPhy (phy);
}
void 
OvnisAdhocWifiMac::SetWifiRemoteStationManager (Ptr<WifiRemoteStationManager> stationManager)
{
  m_stationManager = stationManager;
  m_dca->SetWifiRemoteStationManager (stationManager);
  m_low->SetWifiRemoteStationManager (stationManager);
}
void 
OvnisAdhocWifiMac::SetForwardUpCallback (Callback<void,Ptr<Packet>, Mac48Address, Mac48Address> upCallback)
{
  m_upCallback = upCallback;
}
void 
OvnisAdhocWifiMac::SetLinkUpCallback (Callback<void> linkUp)
{
  // an Adhoc network is always UP.
  linkUp ();
}
void 
OvnisAdhocWifiMac::SetLinkDownCallback (Callback<void> linkDown)
{}
Mac48Address 
OvnisAdhocWifiMac::GetAddress (void) const
{
  return m_low->GetAddress ();
}
Ssid 
OvnisAdhocWifiMac::GetSsid (void) const
{
  return m_ssid;
}
Mac48Address 
OvnisAdhocWifiMac::GetBssid (void) const
{
  return m_low->GetBssid ();
}
void 
OvnisAdhocWifiMac::SetAddress (Mac48Address address)
{
  m_low->SetAddress (address);
  m_low->SetBssid (address);
  // XXX the bssid should be generated by the procedure
  // described in ieee802.11 section 11.1.3
}
void 
OvnisAdhocWifiMac::SetSsid (Ssid ssid)
{
  // XXX: here, we should start a special adhoc network
  m_ssid = ssid;
}

void 
OvnisAdhocWifiMac::Enqueue (Ptr<const Packet> packet, Mac48Address to, Mac48Address from)
{
  NS_FATAL_ERROR ("Adhoc does not support a from != m_low->GetAddress ()");
}
void 
OvnisAdhocWifiMac::Enqueue (Ptr<const Packet> packet, Mac48Address to)
{
  NS_LOG_FUNCTION (packet->GetSize () << to);
  WifiMacHeader hdr;
  hdr.SetType (WIFI_MAC_DATA);
  hdr.SetAddr1 (to);
  hdr.SetAddr2 (m_low->GetAddress ());
  hdr.SetAddr3 (GetBssid ());
  hdr.SetDsNotFrom ();
  hdr.SetDsNotTo ();

  if (m_stationManager->IsBrandNew (to))
    {
      // in adhoc mode, we assume that every destination
      // supports all the rates we support.
      for (uint32_t i = 0; i < m_phy->GetNModes (); i++)
        {
          m_stationManager->AddSupportedMode (to, m_phy->GetMode (i));
        }
      m_stationManager->RecordDisassociated (to);
    }

  m_dca->Queue (packet, hdr);
}
bool 
OvnisAdhocWifiMac::SupportsSendFrom (void) const
{
  return false;
}

void 
OvnisAdhocWifiMac::ForwardUp (Ptr<Packet> packet, const WifiMacHeader *hdr)
{
  NS_LOG_DEBUG ("received size="<<packet->GetSize ()<<", from="<<hdr->GetAddr2 ());
  m_upCallback (packet, hdr->GetAddr2 (), hdr->GetAddr1 ());
}
Ptr<DcaTxop>
OvnisAdhocWifiMac::GetDcaTxop(void) const
{
  return m_dca;
}

void 
OvnisAdhocWifiMac::FinishConfigureStandard (enum WifiPhyStandard standard)
{
  switch (standard)
    {
    case WIFI_PHY_STANDARD_holland:
      // fall through
    case WIFI_PHY_STANDARD_80211_10MHZ:
      // fall through
    case WIFI_PHY_STANDARD_80211_5MHZ:
      // fall through
    case WIFI_PHY_STANDARD_80211a:
      ConfigureDcf (m_dca, 15, 1023, AC_BE_NQOS);
      break;
    case WIFI_PHY_STANDARD_80211b:
      ConfigureDcf (m_dca, 31, 1023, AC_BE_NQOS);
      break;
    default:
      NS_ASSERT (false);
      break;
    }
}
void
OvnisAdhocWifiMac::TxOk (const WifiMacHeader &hdr)
{
  m_txOkCallback (hdr);
}
void
OvnisAdhocWifiMac::TxFailed (const WifiMacHeader &hdr)
{
  m_txErrCallback (hdr);
}
void
OvnisAdhocWifiMac::DoStart ()
{
  m_dca->Initialize ();
  WifiMac::DoInitialize ();
}

} // namespace ns3
