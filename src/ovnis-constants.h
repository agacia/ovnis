/**
 *
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
 * @file ovnis-constants.h
 * @date Apr 21, 2010
 *
 * @author Yoann Pigné
 */

#ifndef OVNIS_CONSTANTS_H_
#define OVNIS_CONSTANTS_H_

//#define PI 3.14159265

#define MAX_COMMUNICATION_RANGE 600

/// interval of time between 2 active decisions about JAMEs
#define PROACTIVE_INTERVAL  5

#define BEACONING_INTERVAL  1

/// simulation step interval time
#define SIMULATION_STEP_INTERVAL  1

#define STATE_PACKET_ID 1
#define TRAVELTIME_PACKET_ID 2
#define WARNING_PACKET_ID 3

#define BROADCASTING_DISTANCE_THRESHOLD 60

#define LOCAL_MEMORY_SIZE 10

#define RESEND_INTERVAL 1 // interval between two packet re-sending
#define PACKET_TTL  300 // Time is seconds a packet is allowed to be forward
#define SPEED_THRESHOLD  0.2 // ratio of max speed under which I consider it's a JAM
#define JAMMED_TIME_THRESHOLD 5 // number of seconds after which a vehicle that has speed less than SPEED_THRESHOLD*maxLaneSpeed i sconsidered to be jammed
#define SPEED_SENSIVITY 5 // m/s after which change a vehicle will notify 5ms=18kmh
#define PENETRATION_RATE 0.9

// bounding box (get: simulation)
#define VAR_WAITING_VEHICLES_NUMBER 0x7d

/// The system path where the SUMO executable is located
#define SUMO_PATH "/opt/sumo/bin/sumo"

#define SUMO_HOST "localhost"

#define SUMO_PORT 1239

typedef enum _VariableType {
  	VEHICLES_DEPARTURED = 1,
  	VEHICLES_ARRIVED = 2,
  	VEHICLES_LOADED = 3,
  	VEHICLES_WAITING = 4,
  	VEHICLES_RUNNING = 5
  } VariableType;

#define COMMAND_HEADER_SIZE 2

#define COMMAND_HEADER_EXTENDED_SIZE 6

#define SIMULATION_TIME_UNIT 1000



#endif /* OVNIS_CONSTANTS_H_ */
