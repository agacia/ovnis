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
 * @author Agata Grzybek
 */

#ifndef OVNIS_CONSTANTS_H_
#define OVNIS_CONSTANTS_H_

#define PI 3.14159265

// NS-3 RELATED CONSTANTS
#define MAX_COMMUNICATION_RANGE 560

#define TX_POWER_START 21.0206
#define TX_POWER_END 21.0206
//#define TX_POWER_START 16.02
//#define TX_POWER_END 16.02
#define TX_POWER_LEVELS 1
#define TX_GAIN 0
#define RX_GAIN 0
#define ENERGY_DETECTION_THRESHOLD -96.0
#define CCA_MODEL_THRESHOLD -99
#define PROPAGATION_LOSS_MODEL "ns3::NakagamiPropagationLossModel"
#define PROPAGATION_DELAY_MODEL "ns3::ConstantSpeedPropagationDelayModel"
#define WIFI_PHY_STANDARD WIFI_PHY_STANDARD_80211_10MHZ
#define PHY_MODE "OfdmRate6MbpsBW10MHz"
#define REMOTE_STATION_MANAGER "ns3::ConstantRateWifiManager"
#define MAC_TYPE "ns3::BeaconingAdhocWifiMac"
#define BASE_NETWORK_ADDRESS "10.0.0.0"
#define NETWORK_MASK "255.0.0.0"

// APLICATION PARAMETERS

#define SIMULATION_STEP_INTERVAL  1 // simulation step interval time
#define SIMULATION_TIME_UNIT 1000
#define TRAFFIC_INFORMATION_SENDING_INTERVAL  1

//#define PACKET_TTL  600 // 10 min
#define PACKET_TTL  120 // 2 min
#define CENTRALISED_INFORMATION_TTL  120

#define STATE_PACKET_ID 1
#define TRAFFICINFO_PACKET_ID 2
#define TRAVELTIME_PACKET_ID 3
#define TRAVELTIME_ROUTE_PACKET_ID 4
#define TRAVELTIME_EDGE_PACKET_ID 5
#define WARNING_PACKET_ID 6
#define CHANGED_EDGE_PACKET_ID 7

#define LOCAL_MEMORY_SIZE 10

#define BROADCASTING_DISTANCE_THRESHOLD 60
#define RESEND_INTERVAL 1

#define CHEATER_RATE 0 // % of drivers choosing selfish route
#define CONGESTION_THRESHOLD 0.56 // Ratio of expected_travel_time/actual_travel time. For highways when speed limit = 115 kmph , traffic is considered congested when speed < 64 kmph
#define DENSITY_THRESHOLD 0.77 // For highways when speed limit = 115 kmph , traffic is considered dense when speed < 88 kmph
//#define DENSITY_THRESHOLD 0.8 // For highways when speed limit = 115 kmph , traffic is considered dense when speed < 88 kmph
#define PENETRATION_RATE 1
#define IS_VANET true
#define ACCIDENT_START_TIME 300
#define ACCIDENT_END_TIME 1300
#define NETWORK_ID "Highway" // "Kirchberg"
#define ROUTING_STRATEGY "UE" // "noRouting" "SO" "hybrid"
#define COST_FUNCTION "travelTime" // "congestionLength" "delayTime"

// TRAFFIC CONDITION EVALUATION
#define SPEED_THRESHOLD  0.2 // ratio of max speed under which I consider it's a JAM
#define JAMMED_TIME_THRESHOLD 5 // number of seconds after which a vehicle that has speed less than SPEED_THRESHOLD*maxLaneSpeed i sconsidered to be jammed
#define SPEED_SENSIVITY 5 // m/s after which change a vehicle will notify 5ms=18kmh

// SUMO
#define SUMO_PATH "/opt/sumo/bin/sumo" // The system path where the SUMO executable is located
#define SUMO_HOST "localhost"
#define SUMO_PORT 1239

  // TRACI
#define COMMAND_HEADER_SIZE 2
#define COMMAND_HEADER_EXTENDED_SIZE 6
#define VAR_WAITING_VEHICLES_NUMBER 0x7d

// LOG
typedef enum _VariableType {
  	VEHICLES_DEPARTURED = 1,
  	VEHICLES_ARRIVED = 2,
  	VEHICLES_LOADED = 3,
  	VEHICLES_WAITING = 4,
  	VEHICLES_RUNNING = 5,
  	VEHICLES_CONNECTED = 6
  } VariableType;

#endif /* OVNIS_CONSTANTS_H_ */
