/**
 *
 * Copyright (c) 2010 University of Luxembourg
 *
 * @file channel-cell.h
 * @date Jun 30, 2010
 *
 * @author Yoann Pigné
 */

#ifndef CHANNELCELL_H_
#define CHANNELCELL_H_

/**
 *
 */

#include <ns3/ptr.h>
#include "ns3/object.h"

namespace ns3
{

class OvnisWifiPhy;

  class ChannelCell : public ns3::Object
   {
   public:
     static TypeId GetTypeId (void);

     int i;
     int j;
     std::vector<Ptr<OvnisWifiPhy> > content;
     ChannelCell();
     virtual ~ChannelCell();
   };



}

#endif /* CHANNELCELL_H_ */
