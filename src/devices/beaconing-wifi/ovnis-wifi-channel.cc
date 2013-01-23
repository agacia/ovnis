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
 * @file ovnis-wifi-channel.cpp
 * @date Jun 28, 2010
 *
 * @author Yoann Pigné
 */
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/mobility-model.h"
#include "ns3/net-device.h"
#include "ns3/node.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/object-factory.h"
#include "ovnis-wifi-channel.h"
#include "ovnis-wifi-phy.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/propagation-delay-model.h"
#include "devices/wifi/channel-cell.h"
NS_LOG_COMPONENT_DEFINE ("OvnisWifiChannel");

using namespace std;
namespace ns3
{
  NS_OBJECT_ENSURE_REGISTERED (OvnisWifiChannel);

  TypeId
  OvnisWifiChannel::GetTypeId(void)
  {
    static TypeId
        tid =
            TypeId("ns3::OvnisWifiChannel") .SetParent<WifiChannel> () .AddConstructor<OvnisWifiChannel> () .AddAttribute(
                "PropagationLossModel", "A pointer to the propagation loss model attached to this channel.",
                PointerValue(), MakePointerAccessor(&OvnisWifiChannel::m_loss),
                MakePointerChecker<PropagationLossModel> ()) .AddAttribute("PropagationDelayModel",
                "A pointer to the propagation delay model attached to this channel.", PointerValue(),
                MakePointerAccessor(&OvnisWifiChannel::m_delay), MakePointerChecker<PropagationDelayModel> ());
    return tid;
  }

  OvnisWifiChannel::OvnisWifiChannel()
  {
    NS_LOG_FUNCTION_NOARGS();
    area_x = 1;
    area_y = 1;
    range = 1;
    cells.push_back(std::vector<Ptr<ChannelCell> > ());
    cells[0].push_back(CreateObject<ChannelCell> ());

  }
  OvnisWifiChannel::~OvnisWifiChannel()
  {
    NS_LOG_FUNCTION_NOARGS ();
    m_phyList.clear();
  }

  void
  OvnisWifiChannel::SetPropagationLossModel(Ptr<PropagationLossModel> loss)
  {
    m_loss = loss;
  }
  void
  OvnisWifiChannel::SetPropagationDelayModel(Ptr<PropagationDelayModel> delay)
  {
    m_delay = delay;
  }

  void
  OvnisWifiChannel::Send(Ptr<OvnisWifiPhy> sender, Ptr<const Packet> packet, double txPowerDbm, WifiMode wifiMode,
      WifiPreamble preamble) const
  {
    //NS_LOG_FUNCTION_NOARGS();

    Ptr<MobilityModel> senderMobility = sender->GetMobility()->GetObject<MobilityModel> ();
    NS_ASSERT (senderMobility != 0);
    Ptr<ChannelCell> senderCell = sender->cell;//GetObject<ChannelCell> ();
    NS_ASSERT (senderCell != 0);
    int nbcells=0;
    int i, j;
    for (i = (senderCell->i - 1) > 0 ? (senderCell->i - 1) : 0; i
        <= ((senderCell->i + 1) < cells.size() ? (senderCell->i + 1) : cells.size()-1); i++)
    {
      for (j = (((senderCell->j - 1) > 0) ? (senderCell->j - 1) : 0); j
          <= ((senderCell->j + 1) < cells[0].size() ? (senderCell->j + 1) : cells[0].size()-1); j++)
      {
        Ptr<ChannelCell> thatCell = cells[i][j];
        nbcells++;

        for (vector<Ptr<OvnisWifiPhy> >::iterator it = thatCell->content.begin(); it != thatCell->content.end(); it++)
        {
          if (sender != (*it))
          {
            // For now don't account for inter channel interference
            if ((*it)->GetChannelNumber() != sender->GetChannelNumber())
              continue;

            Ptr<MobilityModel> receiverMobility = (*it)->GetMobility()->GetObject<MobilityModel> ();
            Time delay = m_delay->GetDelay(senderMobility, receiverMobility);
            double rxPowerDbm = m_loss->CalcRxPower(txPowerDbm, senderMobility, receiverMobility);
            //if (senderMobility->GetDistanceFrom (receiverMobility)>1400)
            //{
            NS_LOG_DEBUG ("propagation: txPower="<<txPowerDbm<<"dbm, rxPower="<<rxPowerDbm<<"dbm, "<<
               "distance="<<senderMobility->GetDistanceFrom (receiverMobility)<<"m, delay="<<delay);
              //}
            Ptr<Packet> copy = packet->Copy();
            Ptr<Object> dstNetDevice = (*it)->GetDevice();
            uint32_t dstNode;
            if (dstNetDevice == 0)
            {
              dstNode = 0xffffffff;
            }
            else
            {
              dstNode = dstNetDevice->GetObject<NetDevice> ()->GetNode()->GetId();
            }
            Simulator::ScheduleWithContext(dstNode, delay, &OvnisWifiChannel::Receive, this, (*it), copy, rxPowerDbm,
                wifiMode, preamble);
          }
        }
      }
    }
//    cout<<"Send into "<<nbcells<<" different cells."<<endl;
  }

  void
  OvnisWifiChannel::Receive(Ptr<OvnisWifiPhy> i, Ptr<Packet> packet, double rxPowerDbm, WifiMode txMode,
      WifiPreamble preamble) const
  {
   // NS_LOG_FUNCTION_NOARGS();

    i->StartReceivePacket(packet, rxPowerDbm, txMode, preamble);
  }

  void
  OvnisWifiChannel::updatePhy(Ptr<OvnisWifiPhy> phy)
  {
    NS_LOG_FUNCTION_NOARGS();
    Ptr<MobilityModel> mob = phy->GetMobility()->GetObject<MobilityModel> ();
    // Ptr<ChannelCell> cell = phy->cell;//GetObject<ChannelCell> ();

    if (phy->cell == 0)
    { // a new one...
      // try to find position of node
      if (mob == 0)
      {
        // no position
        cells[0][0]->content.push_back(phy);
        phy->AggregateObject(cells[0][0]);

      }
      else
      {
        Vector v = mob->GetPosition();
        int nx = (int) (v.x / range);
        int ny = (int) (v.y / range);
        cells[nx][ny]->content.push_back(phy);
        //phy->AggregateObject(cells[nx][ny]);
        phy->cell = cells[nx][ny];
      }
    }
    else
    {
      if (mob == 0)
      {
        //Ptr<ChannelCell> cell = phy->GetObject<ChannelCell> ();
        if (phy->cell != cells[0][0])
        {

          std::vector<Ptr<OvnisWifiPhy> >::iterator it = std::find(phy->cell->content.begin(), phy->cell->content.end(),
              phy);
          if (it != phy->cell->content.end())
          {
            phy->cell->content.erase(it);
          }
          cells[0][0]->content.push_back(phy);
          //phy->AggregateObject(cells[0][0]);
          phy->cell = cells[0][0];
        }

      }
      else
      {
        Vector v = mob->GetPosition();
        int nx = (int) (v.x / range);
        int ny = (int) (v.y / range);

        if (phy->cell != cells[nx][ny])
        {
          std::vector<Ptr<OvnisWifiPhy> >::iterator it = std::find(phy->cell->content.begin(), phy->cell->content.end(),
              phy);
          if (it != phy->cell->content.end())
          {
            phy->cell->content.erase(it);
          }
          cells[nx][ny]->content.push_back(phy);
          //phy->AggregateObject(cells[nx][ny]);
          NS_LOG_DEBUG ("phy moving from cell ("<<phy->cell->i<<","<<phy->cell->j<<") to cell ("<<nx<<","<<ny<<")");
          phy->cell = cells[nx][ny];


        }
      }
    }
  }

  void
OvnisWifiChannel::updateArea(double x, double y, double r)
  {
    NS_LOG_FUNCTION_NOARGS();
    double nb_x_cells = ceil(area_x / range);
    double new_nb_x_cells = ceil(x / r);
    double nb_y_cells = ceil(area_y / range);
    double new_nb_y_cells = ceil(y / r);
    NS_LOG_DEBUG ("area = "<<x<<"x"<<y<<" range = "<<r<<"new nb cells x = "<<new_nb_x_cells<<" new nb cells x="<<new_nb_y_cells);
    int i, j;
    for (i = 0; i < nb_x_cells; i++)
    {
      for (j = nb_y_cells; j < new_nb_y_cells; j++)
      {
        Ptr<ChannelCell> c = CreateObject<ChannelCell> ();
        c->i = i;
        c->j = j;
        cells[i].push_back(c);
      }
    }
    for (i = nb_x_cells; i < new_nb_x_cells; i++)
    {
      cells.push_back(vector<Ptr<ChannelCell> > ());
      for (j = 0; j < new_nb_y_cells; j++)
      {
        Ptr<ChannelCell> c = CreateObject<ChannelCell> ();
        c->i = i;
        c->j = j;
        cells[i].push_back(c);
      }
    }
    area_x = x;
    area_y = y;
    range = r;

    // update nodes;
    for (PhyList::const_iterator i = m_phyList.begin(); i != m_phyList.end(); i++)
    {
      updatePhy(*i);
    }

    // Too much cells, perhaps
    int ii, jj;
    for (ii = 0; ii < i; ii++)
    {
      jj = cells[ii].size() - j;
      for (int jjj = 0; jjj < jj; jjj++)
      {
        if (cells[ii].back()->content.size() != 0)
        {
          NS_LOG_ERROR("Removing Cell "<<cells[ii].back()->i<< " "<<cells[ii].back()->j<<" that is NOT empty !!!!");
        }
        cells[ii].pop_back();
      }
    }
    ii = cells.size();
    for (int iii = ii - 1; iii >= i; iii--)
    {
      for (vector<Ptr<ChannelCell> >::iterator it = cells[iii].begin(); it != cells[iii].end(); it++)
      {
        if ((*it)->content.size() != 0)
        {
          NS_LOG_ERROR("Removing Cell "<<(*it)->i<< " "<<(*it)->j<<" that is NOT empty !!!!");
        }
      }
      cells[iii].clear();
      cells.pop_back();
    }

  }

  uint32_t
  OvnisWifiChannel::GetNDevices(void) const
  {
    return m_phyList.size();
  }
  Ptr<NetDevice>
  OvnisWifiChannel::GetDevice(uint32_t i) const
  {
    return m_phyList[i]->GetDevice()->GetObject<NetDevice> ();
  }

  void
  OvnisWifiChannel::Add(Ptr<OvnisWifiPhy> phy)
  {
    NS_LOG_FUNCTION_NOARGS();
    m_phyList.push_back(phy);
    updatePhy(phy);

  }
  void
  OvnisWifiChannel::Remove(Ptr<OvnisWifiPhy> phy)
  {
    NS_LOG_FUNCTION_NOARGS();

    if (phy->cell == 0)
    {
      NS_LOG_ERROR("No cell !!");
    }
    else
    {
      vector<Ptr<OvnisWifiPhy> >::iterator i = std::find(phy->cell->content.begin(), phy->cell->content.end(), phy);
      if (i != phy->cell->content.end())
        phy->cell->content.erase(i);
    }
    PhyList::iterator i = std::find(m_phyList.begin(), m_phyList.end(), phy);
    if (i != m_phyList.end())
      m_phyList.erase(i);
  }

} // namespace ns3
