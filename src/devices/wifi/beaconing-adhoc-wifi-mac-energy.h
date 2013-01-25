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
 * Author: Patricia
 *
 */
#ifndef BEACONING_ADHOC_WIFI_MAC_ORIGINAL_H
#define BEACONING_ADHOC_WIFI_MAC_ORIGINAL_H

#include "ns3/regular-wifi-mac.h"
#include "ns3/supported-rates.h"
#include "ns3/amsdu-subframe-header.h"
#include "ns3/mac-tx-middle.h"
#include "ns3/mac-low.h"


namespace ns3 {

Ptr<MacLow> m_low;

class BeaconingAdhocWifiMacOriginal : public RegularWifiMac
{
public:
  static TypeId GetTypeId (void);

  BeaconingAdhocWifiMacOriginal ();
  virtual ~BeaconingAdhocWifiMacOriginal ();

  /**
   * \param address the current address of this MAC layer.
   */
  virtual void SetAddress (Mac48Address address);

  /**
   * \param linkUp the callback to invoke when the link becomes up.
   */
  virtual void SetLinkUpCallback (Callback<void> linkUp);

  /**
   * \param packet the packet to send.
   * \param to the address to which the packet should be sent.
   *
   * The packet should be enqueued in a tx queue, and should be
   * dequeued as soon as the channel access function determines that
   * access is granted to this MAC.
   */
  virtual void Enqueue (Ptr<const Packet> packet, Mac48Address to);

  //Patricia ruiz
  /**
   * \returns the interval between two beacon transmissions.
   */
  Time GetBeaconInterval (void) const;
  /**
   * \param interval the interval between two beacon transmissions.
   */
  void SetBeaconInterval (Time interval);

private:
  virtual void Receive (Ptr<Packet> packet, const WifiMacHeader *hdr);

  // My own vbles (Patricia Ruiz)
    void ProcessBeacon( Ptr<Packet> packet, Mac48Address addrFrom);
    void DecreaseBeaconCount();
    void SendOneBeacon (void);
    SupportedRates GetSupportedRates (void) const;
    EventId m_beaconEvent;
    double m_rxPwDbm;

//    Ptr<DcaTxop> m_beaconDca;
//    Time m_beaconInterval;


    typedef	std::map <Mac48Address,uint16_t> MacAddrMap;
    typedef   std::map<Mac48Address, uint16_t>::iterator MacAddrMapIterator;
    uint16_t      m_numberBeaconLost;
    MacAddrMap    m_neighborList;
    Callback<void,Ptr<Packet>, Mac48Address, double> m_neighborCallback;
    Callback<void,Ptr<Packet>, Mac48Address> m_neighborLostCallback;

    TracedCallback<Ptr<const Packet>, Mac48Address > m_neighborLostTraceSource;
    TracedCallback<Ptr<const Packet>, Mac48Address,double  > m_newNeighborTraceSource;


    void SetBeaconGeneration (bool enable);
    bool GetBeaconGeneration (void) const;


};

} // namespace ns3

#endif /* BEACONING_ADHOC_WIFI_MAC_H */
