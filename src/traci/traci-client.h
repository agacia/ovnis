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
 * @file Traci_SimpleRouteChange.h
 * @date Mar 31, 2010
 *
 * @author Yoann Pigné
 */

#ifndef TRACI_CLIENT_H_
#define TRACI_CLIENT_H_

#include <string>
#include <sstream>
#include <vector>

//#include <foreign/tcpip/storage.h>
//#include <foreign/tcpip/socket.h>
#include "traci/storage.h"
#include "traci/socket.h"
#include "ns3/ptr.h"
#include "ns3/core-module.h"
#include "ns3/object.h"

#include <traci-server/TraCIConstants.h>

#include "ovnis-constants.h"

namespace traciclient
{
  struct Position2D
  {
	double x;
	double y;
  };

  struct Position3D
  {
	double x;
	double y;
	double z;
  };

  struct PositionRoadMap
  {
    std::string roadId;
    double pos;
    int laneId;
  };

  struct BoundingBox
  {
    Position2D lowerLeft;
    Position2D upperRight;
  };

  typedef std::vector<Position2D> Polygon;

  struct TLPhase
  {
    std::string precRoadId;
    std::string succRoadId;
    int phase;
  };

  typedef std::vector<TLPhase> TLPhaseList;


  /**
   * Client interface to the TraCI server that allow controlling SUMO from a remote host/process.
   */
  class TraciClient: public ns3::Object
  {

  public:

    TraciClient();

    ~TraciClient();

    void
    errorMsg(std::stringstream&);

    /**
     * Sends a close command to the server so as the simulation ends.
     */
    void
    commandClose();

    /**
     * Actually closes the communication socket.
     * @return True if it is okay.
     */
    bool
    close();

    /**
     * Report errors in the response of the server if any.
     * @param
     * @param
     * @param ignoreCommandId
     * @return
     */
    void validateSubscribeSimulationResponse(tcpip::Storage & inMsg);

    bool
    reportResultState(tcpip::Storage&, int, bool ignoreCommandId = false);
    bool reportExtendedResultState(tcpip::Storage & inMsg, int command, bool ignoreCommandId = false);
    /**
     * Connect the client to a running instance of SUMO on given host and port.
     *
     * @param host The hostname of the machine SUMO is running on.
     * @param port The port of the machine SUMO is running on.
     * @return True if the connection is successful.
     */
    bool
    connect(std::string host = "localhost", int port = 1234);

    /**
     * Register a submission of receiving lists of injected and removed vehicles, and also the current time.
     *
     * @param start The date at which the submission starts.
     * @param stop The date at which the submission stops.
     */

    void
    submission(int start, int stop, u_int8_t dom = CMD_SUBSCRIBE_SIM_VARIABLE, std::vector<u_int8_t> variables = std::vector<u_int8_t>());

    /**
     * Read the result of a command and outputs it in the logger.
     * @param the response from the server.
     * @param the type of data awaited.
     * @return if the format of the answer is OK.
     */
    bool
    readAndReportTypeDependent(tcpip::Storage &, int);

    /**
     * Ask SUMO for a simulation step (1 second
     * @param targetTime
     * @param time
     * @param in
     * @param out
     * @return
     */
    bool
    simulationStep(int targetTime, int & time, std::vector<std::string> & in, std::vector<std::string> & out);

    void responseVehicleSubscriptionHandle(tcpip::Storage & inMsg, int time);

    void responseSimulationSubscriptionHandle(tcpip::Storage & inMsg, int & time, std::vector<std::string> & in, std::vector<std::string> & out);

    bool
    getStringList(u_int8_t dom, u_int8_t cmd, const std::string & node, std::vector<std::string> &);

    bool
    getString(u_int8_t dom, u_int8_t cmd, const std::string &, std::string &);

    double
    getDouble(u_int8_t dom, u_int8_t cmd, const std::string & node);

    int
    getInt(u_int8_t dom, u_int8_t cmd, const std::string & inMsg);

    bool extractCommandStatus(tcpip::Storage & inMsg, u_int8_t cmd, std::string & description);

    Position2D
    getPosition2D(std::string & nodeId);

    /**
     * Simulate the crash of a vehicle, setting it's speed to 0 and changing it's color to black.
     * @param nodeId Vehicle's id
     */
   void
    crash(std::string & nodeId);

    /**
     * Change the color of a vehicle with r,g,b values (0..255).
     * @param nodeId Vehicle's id
     * @param r Red component of the color
     * @param g Green component of the color
     * @param b Blue component of the color
     */
    void changeColor(std::string & nodeId, int,int,int);

    /**
     * All-in-one methods that changes a road (edge) travel time for a given vehicle; then it forces the re-computation of a route, and it finally ask for the new route.
     * @param nodeId
     * @param roadId
     * @param travelTime
     */
    void
   changeRoad(std::string nodeId, std::string roadId, double travelTime);

    // XXXXXXXXXXXXXXXXXXXXXX
    void
    changeRoute(std::string nodeId, std::vector<std::string> stringList);


    void closeRoad(std::string roadId, int begin, int end);

    bool getEdgeInfo(u_int8_t dom, u_int8_t cmd, const std::string & node, std::string & value);

    void closeLane(std::string laneId);

  private:
    tcpip::Socket * socket;

  };

}
#endif /* TRACI_CLIENT_H_ */
