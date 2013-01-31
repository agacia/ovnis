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
#include "myEnergy-tag.h"
#include "ns3/log.h"
NS_LOG_COMPONENT_DEFINE ("MyEnergyTag");


namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (MyEnergyTag);

	TypeId 
MyEnergyTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MyEnergyTag")
    .SetParent<Tag> ()
	.AddConstructor<MyEnergyTag> ()
    ;
  return tid;
}
uint32_t 
MyEnergyTag::GetSerializedSize (void) const
{
	return sizeof(double);
}
void 
MyEnergyTag::Serialize (TagBuffer i) const
{
i.WriteDouble (m_flowIdDouble);
}
void 
MyEnergyTag::Deserialize (TagBuffer i)
{
	m_flowIdDouble = i.ReadDouble();
}
void 
MyEnergyTag::Print (std::ostream &os) const	
{
	os << "TagId=" <<m_flowId;
}

MyEnergyTag::MyEnergyTag ()
: Tag (),m_flowId(0)
{
	
}
MyEnergyTag::~MyEnergyTag ()

{}
TypeId 
MyEnergyTag::GetInstanceTypeId (void) const
{
	return GetTypeId ();
}
void 
MyEnergyTag::SetTag (uint32_t value)
{
	m_flowId = value;

}
void
MyEnergyTag::SetTagDouble (double value)
{
	m_flowIdDouble = value;
}
uint32_t  
MyEnergyTag::GetTag ()
{

	return m_flowId;
}
double
MyEnergyTag::GetTagDouble ()
{

	return m_flowIdDouble;
}


} // namespace ns3
