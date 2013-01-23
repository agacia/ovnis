/**
 *
 * Copyright (c) 2010 University of Luxembourg
 *
 * @file channel-cell.cpp
 * @date Jun 30, 2010
 *
 * @author Yoann Pigné
 */

#include "channel-cell.h"
#include "ovnis-wifi-phy.h"

namespace ns3
{

  NS_OBJECT_ENSURE_REGISTERED (ChannelCell);

  TypeId
  ChannelCell::GetTypeId(void)
  {
    static TypeId tid = TypeId("ns3::ChannelCell") .SetParent<Object> ().AddConstructor<ChannelCell> ();
    return tid;
  }


  ChannelCell::~ChannelCell()
  {
    // TODO Auto-generated destructor stub
  }


  ChannelCell::ChannelCell() :
    i(0), j(0), content()
  {
  }

}
