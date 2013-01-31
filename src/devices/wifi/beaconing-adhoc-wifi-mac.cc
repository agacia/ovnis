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
 * Author:Patricia Ruiz
 */

#include "beaconing-adhoc-wifi-mac.h"
#include "ns3/random-variable.h"
#include "ns3/pointer.h"
#include "ns3/log.h"
#include "ns3/string.h"
#include "ns3/boolean.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/simulator.h"

#include "ns3/qos-tag.h"
//#include "my-mac-low.h"
#include "ns3/mac-low.h"
#include "ns3/dcf-manager.h"
#include "ns3/mac-rx-middle.h"
#include "ns3/msdu-aggregator.h"
#include "ns3/amsdu-subframe-header.h"
#include "ns3/mgt-headers.h"
#include "common/myEnergy-tag.h"

#include "log.h"

/***************DEFINITION AREA*****************/

/// Beacon time interval.
#define BEACON_INTERVAL	1
#define BEACON_LOST_RATIO	4  //Como si fueran 3 intervalos de tiempo

/***************END DEFINITION AREA*************/

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (BeaconingAdhocWifiMac);

TypeId
BeaconingAdhocWifiMac::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::BeaconingAdhocWifiMac")
    .SetParent<RegularWifiMac> ()
    .AddConstructor<BeaconingAdhocWifiMac> ()
	.AddAttribute ("BeaconLost", "The number of beacon lost to loose a neighbor",
                   UintegerValue (BEACON_LOST_RATIO),
                   MakeUintegerAccessor (&BeaconingAdhocWifiMac::m_numberBeaconLost),
                   MakeUintegerChecker<uint32_t> (1))
//%    .AddAttribute ("BeaconInterval", "Delay between two beacons",
//                       TimeValue (Seconds (1)),
//                       MakeTimeAccessor (&BeaconingAdhocWifiMac::GetBeaconInterval,
//                                         &BeaconingAdhocWifiMac::SetBeaconInterval),
//                       MakeTimeChecker ())
//    .AddAttribute ("BeaconGeneration", "Whether or not beacons are generated.",
//                       BooleanValue (true),
//                       MakeBooleanAccessor (&BeaconingAdhocWifiMac::SetBeaconGeneration,
//                                            &BeaconingAdhocWifiMac::GetBeaconGeneration),
//                       MakeBooleanChecker ())
	 .AddTraceSource ("NeighborLost", "A neighbor is lost",
										 MakeTraceSourceAccessor (&BeaconingAdhocWifiMac::m_neighborLostTraceSource))
      .AddTraceSource ("NewNeighbor", "A neighbor is found",
                   					 MakeTraceSourceAccessor (&BeaconingAdhocWifiMac::m_newNeighborTraceSource))
  ;
  return tid;
}

BeaconingAdhocWifiMac::BeaconingAdhocWifiMac ()
{
  NS_LOG_FUNCTION (this);

  // Let the lower layers know that we are acting in an IBSS
  SetTypeOfStation (ADHOC_STA);
  Time t1 = Seconds(BEACON_INTERVAL);// m_beaconInterval;
  Time t2;
  // Random value to start the devices at different moments
  UniformVariable randomRange (0.01, 0.2);
  double randomRangeDouble = randomRange.GetValue();
  t2 = Seconds (randomRangeDouble);
  Time t3 = t1 + t2;

//  m_beaconEvent = Simulator::Schedule (t3, &BeaconingAdhocWifiMac::SendOneBeacon, this);

}

BeaconingAdhocWifiMac::~BeaconingAdhocWifiMac ()
{
  NS_LOG_FUNCTION (this);
  m_beaconEvent.Cancel ();
}

void
BeaconingAdhocWifiMac::SetAddress (Mac48Address address)
{
  // In an IBSS, the BSSID is supposed to be generated per Section
  // 11.1.3 of IEEE 802.11. We don't currently do this - instead we
  // make an IBSS STA a bit like an AP, with the BSSID for frames
  // transmitted by each STA set to that STA's address.
  //
  // This is why we're overriding this method.
  RegularWifiMac::SetAddress (address);
  RegularWifiMac::SetBssid (address);
}

NS_LOG_COMPONENT_DEFINE ("BeaconingAdhocWifiMac");

void
BeaconingAdhocWifiMac::Enqueue (Ptr<const Packet> packet, Mac48Address to)
{
  NS_LOG_FUNCTION (this << packet << to);
  if (m_stationManager->IsBrandNew (to))
    {
      // In ad hoc mode, we assume that every destination supports all
      // the rates we support.
      for (uint32_t i = 0; i < m_phy->GetNModes (); i++)
        {
          m_stationManager->AddSupportedMode (to, m_phy->GetMode (i));
        }
      m_stationManager->RecordDisassociated (to);
    }

  WifiMacHeader hdr;

  // If we are not a QoS STA then we definitely want to use AC_BE to
  // transmit the packet. A TID of zero will map to AC_BE (through \c
  // QosUtilsMapTidToAc()), so we use that as our default here.
  uint8_t tid = 0;

  // For now, a STA that supports QoS does not support non-QoS
  // associations, and vice versa. In future the STA model should fall
  // back to non-QoS if talking to a peer that is also non-QoS. At
  // that point there will need to be per-station QoS state maintained
  // by the association state machine, and consulted here.
  if (m_qosSupported)
    {
      hdr.SetType (WIFI_MAC_QOSDATA);
      hdr.SetQosAckPolicy (WifiMacHeader::NORMAL_ACK);
      hdr.SetQosNoEosp ();
      hdr.SetQosNoAmsdu ();
      // Transmission of multiple frames in the same TXOP is not
      // supported for now
      hdr.SetQosTxopLimit (0);

      // Fill in the QoS control field in the MAC header
      tid = QosUtilsGetTidForPacket (packet);
      // Any value greater than 7 is invalid and likely indicates that
      // the packet had no QoS tag, so we revert to zero, which'll
      // mean that AC_BE is used.
      if (tid >= 7)
        {
          tid = 0;
        }
      hdr.SetQosTid (tid);
    }
  else
    {
      hdr.SetTypeData ();
    }

  hdr.SetAddr1 (to);
  hdr.SetAddr2 (m_low->GetAddress ());
  hdr.SetAddr3 (GetBssid ());
  hdr.SetDsNotFrom ();
  hdr.SetDsNotTo ();

  if (m_qosSupported)
    {
      // Sanity check that the TID is valid
      NS_ASSERT (tid < 8);
      ovnis::Log::getInstance().packetSent();
//      cout << "a" << endl;
      m_edca[QosUtilsMapTidToAc (tid)]->Queue (packet, hdr);
    }
  else
    {
	  // here when I send a packet with application layer
//      cout << "b" << endl;
	  ovnis::Log::getInstance().packetSent();
      m_dca->Queue (packet, hdr);
    }
}

void
BeaconingAdhocWifiMac::SetLinkUpCallback (Callback<void> linkUp)
{
  NS_LOG_FUNCTION (this);
  RegularWifiMac::SetLinkUpCallback (linkUp);

  // The approach taken here is that, from the point of view of a STA
  // in IBSS mode, the link is always up, so we immediately invoke the
  // callback if one is set
  linkUp ();
}

void
BeaconingAdhocWifiMac::Receive (Ptr<Packet> packet, const WifiMacHeader *hdr)
{

  NS_LOG_FUNCTION (this << packet << hdr);
  NS_ASSERT (!hdr->IsCtl ());
  Mac48Address from = hdr->GetAddr2 ();
  Mac48Address to = hdr->GetAddr1 ();
  if (hdr->IsData ())
    {

      if (hdr->IsQosData () && hdr->IsQosAmsdu ())
        {
          NS_LOG_DEBUG ("Received A-MSDU from"<<from);
          DeaggregateAmsduAndForward (packet, hdr);
        }
      else
        {
          ForwardUp (packet, from, to);
        }
      return;
    }
  else if (hdr->IsBeacon ())
    {
//	  std::cout<<"Beacon received"<<std::endl;

	  ovnis::Log::getInstance().packetReceived();

	  MgtBeaconHeader beacon;
	  packet->RemoveHeader (beacon);
	  ProcessBeacon(packet,hdr->GetAddr2 ());


    }
  return;

//	        bool goodBeacon = false;
//	        if (GetSsid ().IsBroadcast () ||
//	            beacon.GetSsid ().IsEqual (GetSsid ()))
//	          {
//	            goodBeacon = true;
//	          }
//	        if ((IsWaitAssocResp () || IsAssociated ()) && hdr->GetAddr3 () != GetBssid ())
//	          {
//	            goodBeacon = false;
//	          }
//	        if (goodBeacon)
//	          {
//	            Time delay = MicroSeconds (beacon.GetBeaconIntervalUs () * m_maxMissedBeacons);
//	            RestartBeaconWatchdog (delay);
//	            SetBssid (hdr->GetAddr3 ());
//	          }
//	        if (goodBeacon && m_state == BEACON_MISSED)
//	          {
//	            SetState (WAIT_ASSOC_RESP);
//	            SendAssociationRequest ();
//	          }
//    }

  // Invoke the receive handler of our parent class to deal with any
  // other frames. Specifically, this will handle Block Ack-related
  // Management Action frames.
  RegularWifiMac::Receive (packet, hdr);
}


//Added by Patricia Ruiz for the beaconing generation

void
BeaconingAdhocWifiMac::SetBeaconGeneration (bool enable)
{
  NS_LOG_FUNCTION (this << enable);
  if (enable)
    {
      m_beaconEvent = Simulator::ScheduleNow (&BeaconingAdhocWifiMac::SendOneBeacon, this);
    }
  else
    {
      m_beaconEvent.Cancel ();
    }
}

bool
BeaconingAdhocWifiMac::GetBeaconGeneration (void) const
{
  return m_beaconEvent.IsRunning ();
}

void
BeaconingAdhocWifiMac::SendOneBeacon (void)
{
  DecreaseBeaconCount();
  NS_LOG_FUNCTION (this);
  WifiMacHeader hdr;
  hdr.SetBeacon ();
  hdr.SetAddr1 (Mac48Address::GetBroadcast ());
  hdr.SetAddr2 (GetAddress ());
  hdr.SetAddr3 (GetAddress ());
//  hdr.SetDsNotFrom ();
//  hdr.SetDsNotTo ();
  MgtBeaconHeader beacon;
  beacon.SetSsid (GetSsid ());
  beacon.SetSupportedRates (GetSupportedRates ());
//  beacon.SetBeaconIntervalUs (m_beaconInterval.GetMicroSeconds ());


  Ptr<Packet> packet = Create<Packet> ();
  //packet->AddHeader (hdr);
  packet->AddHeader (beacon);


//  Mac48Address aaa= GetAddress();
//  std::cout<<Simulator::Now()<<" envio beacon "<<aaa<<std::endl;

//  Enqueue(packet,Mac48Address::GetBroadcast () );
  m_dca->Queue (packet, hdr);
//  cout << "c" << endl;
  ovnis::Log::getInstance().packetSent();
//  m_dca->Queue (packet, beacon);
//cout << "sending! " << endl;
  m_beaconEvent = Simulator::Schedule (Seconds(BEACON_INTERVAL), &BeaconingAdhocWifiMac::SendOneBeacon, this);


}
void BeaconingAdhocWifiMac::ProcessBeacon( Ptr<Packet> packet, Mac48Address addrFrom){

	if ( m_neighborList.find (addrFrom)== m_neighborList.end ()){
		// update the beacon index
		m_neighborList[addrFrom] = m_numberBeaconLost;
		//std::cout<<addrFrom<<" Addd en beaconing wifi mac "<<std::endl;
	}
	  //Added by Patricia Ruiz (for changing the tx power)
//	  	MyEnergyTag neighborPower=MyEnergyTag();
//	  	Ptr<Packet> packet2 = packet->Copy();
//	  	bool aux =packet2->RemovePacketTag (neighborPower);
//	  	double rxPowerDbm=-95;
//	  	if (aux){
//	  		rxPowerDbm = neighborPower.GetTagDouble();
//	  	}


	m_neighborList[addrFrom] = m_numberBeaconLost;
	//Call back every beacon to be able to update the rxEg otherwise inside the if!!
//	m_newNeighborTraceSource (packet, addrFrom,rxPowerDbm);

	m_rxPwDbm = 16.02;
	m_newNeighborTraceSource (packet, addrFrom,m_rxPwDbm);

	//std::cout<<Simulator::Now()<<GetAddress()<<" Incremento contador de  "<<addrFrom<<" a "<<m_neighborList[addrFrom]<<" con potencia "<<rxPowerDbm<<std::endl;

}

void BeaconingAdhocWifiMac::DecreaseBeaconCount(){
	//std::cout<<GetAddress()<<" Decremento contador de "<<std::endl;

	if (!m_neighborList.empty()){
		//Create a vector to store the neighbors gone
		std::vector <Mac48Address> arrayToErase;

		for (MacAddrMapIterator i = m_neighborList.begin (); i != m_neighborList.end (); i++) {

			uint16_t index = i->second;
			//Insert the decreased value of the beacon index
			if (index !=0){
				index--;
				i->second = index;
			}
			//When arrive to 0 the node consider the neighbor is lost
			else{
				//Store the neighbors to delete in a vector
				Mac48Address addrFrom = i->first;
				arrayToErase.push_back(addrFrom);
           }
		}
		if (!arrayToErase.empty()){
			// remove the neighbors gone from the list they were stored
			std::vector<Mac48Address>::iterator it;
			for ( it=arrayToErase.begin() ; it <= arrayToErase.end(); it++ ){
				Mac48Address addr = arrayToErase.back();
				arrayToErase.pop_back();
				m_neighborList.erase(addr);
				Ptr<Packet> packet =  Create<Packet> ();
			//	ForwardNeighborLostList (packet, addr);
				m_neighborLostTraceSource(packet, addr);
		    }
		}
   }

}
SupportedRates
BeaconingAdhocWifiMac::GetSupportedRates (void) const
{
  // send the set of supported rates and make sure that we indicate
  // the Basic Rate set in this set of supported rates.
  SupportedRates rates;
  for (uint32_t i = 0; i < m_phy->GetNModes (); i++)
    {
      WifiMode mode = m_phy->GetMode (i);
      rates.AddSupportedRate (mode.GetDataRate ());
    }
  // set the basic rates
  for (uint32_t j = 0; j < m_stationManager->GetNBasicModes (); j++)
    {
      WifiMode mode = m_stationManager->GetBasicMode (j);
      rates.SetBasicRate (mode.GetDataRate ());
    }
  return rates;
}

void
BeaconingAdhocWifiMac::StopBeaconing()
{
    m_beaconEvent.Cancel();
}
} // namespace ns3
