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

#include "myMessageID-header.h"
#include "ns3/address-utils.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (MyMessageIDHeader);

/* The magic values below are used only for debugging.
 * They can be used to easily detect memory corruption
 * problems so you can see the patterns in memory.
 */
MyMessageIDHeader::MyMessageIDHeader ()
	:
	 m_IdMessage (0)
{

}
	
MyMessageIDHeader::~MyMessageIDHeader ()
{
   m_IdMessage = 0xfffe;

  
}
	 
void 
MyMessageIDHeader::SetIdMessage (uint32_t  level)
{
    m_IdMessage = level;
}
uint32_t  
MyMessageIDHeader::GetIdMessage () 
{
	return  m_IdMessage;
}
TypeId 
MyMessageIDHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MyMessageIDHeader")
    .SetParent<Header> ()
    .AddConstructor<MyMessageIDHeader> ()
    ;
  return tid;
}
TypeId 
MyMessageIDHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}
void 
MyMessageIDHeader::Print (std::ostream &os) const
{
			os << "length: " << GetSerializedSize ()
			<< " " 
			<< "type: Energy";
}

uint32_t 
MyMessageIDHeader::GetSerializedSize (void) const
{
  return 4; 
}
//
void
MyMessageIDHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  i.WriteHtonU32 (m_IdMessage);
}
uint32_t
MyMessageIDHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  m_IdMessage = i.ReadNtohU32 ();


  return GetSerializedSize ();
}
//

}; // namespace ns3
