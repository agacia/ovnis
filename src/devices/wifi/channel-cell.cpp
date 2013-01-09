/**
 *
 * Copyright (c) 2010-2011 University of Luxembourg
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

  TypeId ChannelCell::GetTypeId(void)
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
