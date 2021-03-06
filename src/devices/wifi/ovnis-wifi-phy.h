/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006 INRIA
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

/**
 *
 * Copyright (c) 2010 University of Luxembourg
 *
 * @file ovnis-wifi-phy.h
 * @date Jun 28, 2010
 *
 * @author Yoann Pigné
 */

#ifndef OVNIS_WIFI_PHY_H
#define OVNIS_WIFI_PHY_H

#include <stdint.h>
#include "ns3/callback.h"
#include "ns3/event-id.h"
#include "ns3/packet.h"
#include "ns3/object.h"
#include "ns3/traced-callback.h"
#include "ns3/nstime.h"
#include "ns3/ptr.h"
//#include "ns3/random-variable.h"
#include "ns3/random-variable-stream.h"
//#include "devices/wifi/wifi-phy.h"
//#include "devices/wifi/wifi-mode.h"
//#include "devices/wifi/wifi-preamble.h"
//#include "devices/wifi/wifi-phy-standard.h"
//#include "devices/wifi/interference-helper.h"
#include "ns3/wifi-phy.h"
#include "ns3/wifi-mode.h"
#include "ns3/wifi-preamble.h"
#include "ns3/wifi-phy-standard.h"
#include "ns3/interference-helper.h"

#include "channel-cell.h"

#define HT_PHY 127

namespace ns3 {

class RandomUniform;
class RxEvent;
class OvnisWifiChannel;
class WifiPhyStateHelper;
//class ChannelCell;

/**
 * \brief 802.11 PHY layer model
 *
 * This PHY implements a model of 802.11a. The model
 * implemented here is based on the model described
 * in "Yet Another Network Simulator", 
 * (http://cutebugs.net/files/wns2-yans.pdf).
 *
 *
 * This PHY model depends on a channel loss and delay
 * model as provided by the ns3::PropagationLossModel
 * and ns3::PropagationDelayModel classes, both of which are
 * members of the ns3::YansWifiChannel class.
 */
class OvnisWifiPhy : public WifiPhy
{
public:
  Ptr<ChannelCell> cell;
  static TypeId GetTypeId (void);

  OvnisWifiPhy ();
  virtual ~OvnisWifiPhy ();

  void SetChannel (Ptr<OvnisWifiChannel> channel);
  
  /** 
   * \brief Set channel number. 
   * 
   * Channel center frequency = Channel starting frequency + 5 MHz * (nch - 1)
   *
   * where Starting channel frequency is standard-dependent, see SetStandard()
   * as defined in IEEE 802.11-2007 17.3.8.3.2.
   *
   * OvnisWifiPhy can switch among different channels. Basically, OvnisWifiPhy
   * has a private attribute m_channelNumber that identifies the channel the 
   * PHY operates on. Channel switching cannot interrupt an ongoing transmission.
   * When PHY is in TX state, the channel switching is postponed until the end
   * of the current transmission. When the PHY is in RX state, the channel 
   * switching causes the drop of the synchronized packet. 
   */ 
  void SetChannelNumber (uint16_t id);
  /// Return current channel number, see SetChannelNumber()
  uint16_t GetChannelNumber () const;
  /// Return current center channel frequency in MHz, see SetСhannelNumber()
  double GetChannelFrequencyMhz() const;
  
  void StartReceivePacket (Ptr<Packet> packet,
                           double rxPowerDbm,
                           WifiTxVector txVector,
                           WifiPreamble preamble);

  void SetRxNoiseFigure (double noiseFigureDb);
  void SetTxPowerStart (double start);
  void SetTxPowerEnd (double end);
  void SetNTxPower (uint32_t n);
  void SetTxGain (double gain);
  void SetRxGain (double gain);
  void SetEdThreshold (double threshold);
  void SetCcaMode1Threshold (double threshold);
  void SetErrorRateModel (Ptr<ErrorRateModel> rate);
  void SetDevice (Ptr<Object> device);
  void SetMobility (Ptr<Object> mobility);
  double GetRxNoiseFigure (void) const;
  double GetTxGain (void) const;
  double GetRxGain (void) const;
  double GetEdThreshold (void) const;
  double GetCcaMode1Threshold (void) const;
  Ptr<ErrorRateModel> GetErrorRateModel (void) const;
  Ptr<Object> GetDevice (void) const;
  Ptr<Object> GetMobility (void);
  
  // Added by Patricia Ruiz (eg of received messages)
  void SetRxPowerDBm (double level);
  double GetRxPowerDBm ();


  virtual double GetTxPowerStart (void) const;
  virtual double GetTxPowerEnd (void) const;
  virtual uint32_t GetNTxPower (void) const;
  virtual void SetReceiveOkCallback (WifiPhy::RxOkCallback callback);
  virtual void SetReceiveErrorCallback (WifiPhy::RxErrorCallback callback);
//  virtual void SendPacket (Ptr<const Packet> packet, WifiMode mode, enum WifiPreamble preamble, uint8_t txPowerLevel);
  virtual void SendPacket (Ptr<const Packet> packet, WifiMode txMode, WifiPreamble preamble, WifiTxVector txVector);
  virtual void RegisterListener (WifiPhyListener *listener);
  virtual bool IsStateCcaBusy (void);
  virtual bool IsStateIdle (void);
  virtual bool IsStateBusy (void);
  virtual bool IsStateRx (void);
  virtual bool IsStateTx (void);
  virtual bool IsStateSwitching (void); 
  virtual Time GetStateDuration (void);
  virtual Time GetDelayUntilIdle (void);
  virtual Time GetLastRxStartTime (void) const;
//  virtual Time CalculateTxDuration (uint32_t size, WifiMode payloadMode, enum WifiPreamble preamble) const;
  virtual uint32_t GetNModes (void) const;
  virtual WifiMode GetMode (uint32_t mode) const;
  virtual double CalculateSnr (WifiMode txMode, double ber) const;
  virtual Ptr<WifiChannel> GetChannel (void) const;
  virtual void ConfigureStandard (enum WifiPhyStandard standard);
  virtual int64_t AssignStreams (int64_t stream);
  virtual uint32_t GetNBssMembershipSelectors (void) const;
   virtual uint32_t GetBssMembershipSelector (uint32_t selector) const;
   virtual WifiModeList GetMembershipSelectorModes(uint32_t selector);
   /**
    * The WifiPhy::GetNMcs() and  WifiPhy::GetMcs() methods are used
    * (e.g., by a WifiRemoteStationManager) to determine the set of
    * transmission/reception MCS indexes that this WifiPhy(-derived class)
    * can support - a set of Mcs indexes which we call the
    * DeviceMcsSet, and which is stored as WifiPhy::m_deviceMcsSet.
    *
    * This was introduced with 11n
    *
    * \param Mcs index in array of supported Mcs
    * \returns the Mcs index whose index is specified.
    *
    * \sa WifiPhy::GetNMcs()
    */
   virtual uint8_t GetNMcs (void) const;
   virtual uint8_t GetMcs (uint8_t mcs) const;

   /* Converts from DataRate to MCS index and vice versa */
   virtual uint32_t WifiModeToMcs (WifiMode mode);
   virtual WifiMode McsToWifiMode (uint8_t mcs);
   /**
      * \param the operating frequency on this node.
      */
     virtual void SetFrequency (uint32_t freq);
     virtual uint32_t GetFrequency (void) const;
     /**
      * \param the number of transmitters on this node.
      */
     virtual void SetNumberOfTransmitAntennas (uint32_t tx);

     virtual uint32_t GetNumberOfTransmitAntennas (void) const;
      /**
      * \param the number of recievers on this node.
      */
     virtual void SetNumberOfReceiveAntennas (uint32_t rx) ;
     /**
      * \returns the number of recievers on this node.
      */
     virtual uint32_t GetNumberOfReceiveAntennas (void) const;
     /**
      * \paramif short guard interval is supported or not
      */
      virtual void SetGuardInterval (bool GuardInterval);
      /**
      *  \returns if short guard interval is supported or not
      */
     virtual bool GetGuardInterval (void) const;
     /**
      * \paramif LDPC is supported or not
      */
     virtual void SetLdpc (bool Ldpc);
     /**
      * \returns if LDPC is supported or not
      */
     virtual bool GetLdpc (void) const;
     /**
      * \paramif STBC is supported or not
      */
     virtual void SetStbc (bool stbc);
     /**
      *  \returns if STBC is supported or not
      */
     virtual bool GetStbc (void) const;

     /**
      * \paramif GreenField is supported or not
      */
     virtual void SetGreenfield (bool greenfield);
     /**
      *  \returns if Green field is supported or not
      */
     virtual bool GetGreenfield (void) const;
     /**
      * \paramif channel bonding 40 MHz is supported or not
      */
     virtual bool GetChannelBonding (void) const;
     /**
      *  \returns if channel bonding is supported or not
      */
     virtual void SetChannelBonding (bool channelbonding) ;



private:
  typedef std::vector<WifiMode> Modes;

private:
  OvnisWifiPhy (const OvnisWifiPhy &o);
  virtual void DoDispose (void);
  void Configure80211a (void);
  void Configure80211b (void);
  void Configure80211g (void);
  void Configure80211_10Mhz (void);
  void Configure80211_5Mhz ();
  void ConfigureHolland (void);
  void Configure80211p_CCH (void);
  void Configure80211p_SCH (void);
  double GetEdThresholdW (void) const;
  double DbmToW (double dbm) const;
  double DbToRatio (double db) const;
  double WToDbm (double w) const;
  double RatioToDb (double ratio) const;
  double GetPowerDbm (uint8_t power) const;
  void EndReceive (Ptr<Packet> packet, Ptr<InterferenceHelper::Event> event);

private:
  double   m_edThresholdW;
  double   m_ccaMode1ThresholdW;
  double   m_txGainDb;
  double   m_rxGainDb;
  double   m_txPowerBaseDbm;
  double   m_txPowerEndDbm;
  uint32_t m_nTxPower;
  std::vector<uint32_t> m_bssMembershipSelectorSet;
  // number of transmitters
    uint32_t m_numberOfTransmitters;
    // number of recievers
    uint32_t m_numberOfReceivers;
    //if true use LDPC
    bool     m_ldpc;
    // True if STBC is used
    bool     m_stbc;
    //True if GreenField format is supported
    bool     m_greenfield;
    //True is short guard interval is used
    bool     m_guardInterval;
    //True if channel bonding is used
    bool     m_channelBonding;


  Ptr<OvnisWifiChannel> m_channel;
  uint16_t m_channelNumber;
  Ptr<Object> m_device;
  Ptr<Object> m_mobility;

  // Added by Patricia Ruiz (eg of received messages)
  double m_rxPowerDbm;

  /**
   * This vector holds the set of transmission modes that this
   * WifiPhy(-derived class) can support. In conversation we call this
   * the DeviceRateSet (not a term you'll find in the standard), and
   * it is a superset of standard-defined parameters such as the
   * OperationalRateSet, and the BSSBasicRateSet (which, themselves,
   * have a superset/subset relationship).
   *
   * Mandatory rates relevant to this WifiPhy can be found by
   * iterating over this vector looking for WifiMode objects for which
   * WifiMode::IsMandatory() is true.
   *
   * A quick note is appropriate here (well, here is as good a place
   * as any I can find)...
   *
   * In the standard there is no text that explicitly precludes
   * production of a device that does not support some rates that are
   * mandatory (according to the standard) for PHYs that the device
   * happens to fully or partially support.
   *
   * This approach is taken by some devices which choose to only support,
   * for example, 6 and 9 Mbps ERP-OFDM rates for cost and power
   * consumption reasons (i.e., these devices don't need to be designed
   * for and waste current on the increased linearity requirement of
   * higher-order constellations when 6 and 9 Mbps more than meet their
   * data requirements). The wording of the standard allows such devices
   * to have an OperationalRateSet which includes 6 and 9 Mbps ERP-OFDM
   * rates, despite 12 and 24 Mbps being "mandatory" rates for the
   * ERP-OFDM PHY.
   *
   * Now this doesn't actually have any impact on code, yet. It is,
   * however, something that we need to keep in mind for the
   * future. Basically, the key point is that we can't be making
   * assumptions like "the Operational Rate Set will contain all the
   * mandatory rates".
   */
  WifiModeList m_deviceRateSet;

  std::vector<uint8_t> m_deviceMcsSet;
  EventId m_endRxEvent;
  Ptr<UniformRandomVariable> m_random;
  /// Standard-dependent center frequency of 0-th channel, MHz 
  double m_channelStartingFrequency;
  Ptr<WifiPhyStateHelper> m_state;
  InterferenceHelper m_interference;
  Time m_channelSwitchDelay;


};

} // namespace ns3


#endif /* OVNIS_WIFI_PHY_H */
