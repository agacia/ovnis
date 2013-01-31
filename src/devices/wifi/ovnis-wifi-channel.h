/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2006,2007 INRIA
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
 * Author: Mathieu Lacage, <mathieu.lacage@sophia.inria.fr>
 */

/**
 *
 * Copyright (c) 2010 University of Luxembourg
 *
 * @file ovnis-wifi-channel.h
 * @date Jun 30, 2010
 *
 * @author Yoann Pigné
 */
#ifndef OVNIS_WIFI_CHANNEL_H
#define OVNIS_WIFI_CHANNEL_H

#include <vector>
#include <stdint.h>
#include "ns3/packet.h"
#include "ns3/wifi-channel.h"
#include "ns3/wifi-mode.h"
#include "ns3/wifi-preamble.h"
#include "ovnis-wifi-phy.h"
namespace ns3 {

class NetDevice;
class PropagationLossModel;
class PropagationDelayModel;
class OvnisWifiPhy;
class ChannelCell;

/**
 * \brief A Yans wifi channel
 *
 * This wifi channel implements the propagation model described in
 * "Yet Another Network Simulator", (http://cutebugs.net/files/wns2-yans.pdf).
 *
 * This class is expected to be used in tandem with the ns3::OvnisWifiPhy
 * class and contains a ns3::PropagationLossModel and a ns3::PropagationDelayModel.
 * By default, no propagation models are set so, it is the caller's responsability
 * to set them before using the channel.
 */
class OvnisWifiChannel : public WifiChannel
{
public:
  static TypeId GetTypeId (void);

  OvnisWifiChannel ();
  virtual ~OvnisWifiChannel ();

  // inherited from Channel.
  virtual uint32_t GetNDevices (void) const;
  virtual Ptr<NetDevice> GetDevice (uint32_t i) const;

  virtual void Add (Ptr<OvnisWifiPhy> phy);

  virtual void Remove (Ptr<OvnisWifiPhy> phy);

  /**
   * \param loss the new propagation loss model.
   */
  void SetPropagationLossModel (Ptr<PropagationLossModel> loss);
  /**
   * \param delay the new propagation delay model.
   */
  void SetPropagationDelayModel (Ptr<PropagationDelayModel> delay);

  /**
   * \param sender the device from which the packet is originating.
   * \param packet the packet to send
   * \param txPowerDbm the tx power associated to the packet
   * \param wifiMode the tx mode associated to the packet
   * \param preamble the preamble associated to the packet
   *
   * This method should not be invoked by normal users. It is 
   * currently invoked only from WifiPhy::Send. YansWifiChannel 
   * delivers packets only between PHYs with the same m_channelNumber,
   * e.g. PHYs that are operating on the same channel.
   */
  virtual void Send (Ptr<OvnisWifiPhy> sender, Ptr<const Packet> packet, double txPowerDbm,
             WifiMode wifiMode, WifiPreamble preamble) const;

  void
  updateArea(double x, double y, double r);

  void
  updatePhy(Ptr<OvnisWifiPhy> );





protected:
  typedef std::vector<Ptr<OvnisWifiPhy> > PhyList;

  void Receive ( Ptr<OvnisWifiPhy> i, Ptr<Packet> packet, double rxPowerDbm,
                 WifiMode txMode, WifiPreamble preamble) const;
 double area_x;
  double area_y;
  double range;
  std::vector<std::vector<Ptr<ChannelCell> > > cells;

  // void Receive (uint32_t i, Ptr<Packet> packet, double rxPowerDbm,
  //              WifiMode txMode, WifiPreamble preamble) const;


  PhyList m_phyList;
  Ptr<PropagationLossModel> m_loss;
  Ptr<PropagationDelayModel> m_delay;
};

} // namespace ns3


#endif /* OVNIS_WIFI_CHANNEL_H */
