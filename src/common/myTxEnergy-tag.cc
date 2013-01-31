/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 INRIA
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
#include "myTxEnergy-tag.h"
#include "ns3/log.h"
NS_LOG_COMPONENT_DEFINE ("MyTxEnergyTag");


namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (MyTxEnergyTag);

	TypeId 
MyTxEnergyTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MyTxEnergyTag")
    .SetParent<Tag> ()
	.AddConstructor<MyTxEnergyTag> ()
    ;
  return tid;
}
uint32_t 
MyTxEnergyTag::GetSerializedSize (void) const
{
	return sizeof(double);
}
void 
MyTxEnergyTag::Serialize (TagBuffer i) const
{
//	i.WriteU32 (m_flowId);
	i.WriteDouble (m_flowIdDouble);
}
void 
MyTxEnergyTag::Deserialize (TagBuffer i)
{
	//m_flowId = i.ReadU32 ();
	m_flowIdDouble = i.ReadDouble();
}
void 
MyTxEnergyTag::Print (std::ostream &os) const	
{
	os << "TagId=" <<m_flowId;
}

MyTxEnergyTag::MyTxEnergyTag ()
: Tag (),m_flowId(0),m_flowIdDouble(0)
{
	
}
MyTxEnergyTag::~MyTxEnergyTag ()

{}
TypeId 
MyTxEnergyTag::GetInstanceTypeId (void) const
{
	return GetTypeId ();
}
void 
MyTxEnergyTag::SetTag (uint32_t value)
{
	m_flowId = value;

}
void
MyTxEnergyTag::SetTagDouble (double value)
{
	m_flowIdDouble = value;
}
uint32_t  
MyTxEnergyTag::GetTag ()
{

	return m_flowId;
}
double
MyTxEnergyTag::GetTagDouble ()
{

	return m_flowIdDouble;
}


} // namespace ns3
