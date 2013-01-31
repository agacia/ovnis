# additional include paths necessary to compile the C++ library
AM_CXXFLAGS = -I$(top_srcdir) @AM_CPPFLAGS@

# the library names to build (note we are building static libs only)
#lib_LIBRARIES = libovnis.a

# where to install the headers on the system
#libovnis_adir = $(includedir)

# the list of header files that belong to the library (to be installed later)
#libovnis_a_HEADERS = ./applications/ovnis-application.h ./devices/wifi/channel-cell.h ./devices/wifi/ovnis-adhoc-wifi-mac.h ./devices/wifi/ovnis-error-rate-model.h \
	./devices/wifi/ovnis-wifi-channel.h ./devices/wifi/ovnis-wifi-phy.h ./helper/ovnis-wifi-helper.h \
	./ovnis-constants.h ./ovnis.h ./traci/traci-client.h ./xml-sumo-conf-parser.h ./log.h ./vehicle.h
                         

# the sources to add to the library and to add to the source distribution
#libovnis_a_SOURCES =    \
#                       $(libmylib_a_HEADERS) \
#                        ./devices/wifi/ovnis-adhoc-wifi-mac.cc ./devices/wifi/ovnis-error-rate-model.cc ./devices/wifi/ovnis-wifi-channel.cc ./devices/wifi/ovnis-wifi-phy.cc \
						 ./helper/ovnis-wifi-helper.cc ./traci/traci-client.cc ./applications/ovnis-application.cpp ./devices/wifi/channel-cell.cpp ./ovnis.cpp ./xml-sumo-conf-parser.cpp


lib_LTLIBRARIES = libovnis.la
libovnis_ladir = $(includedir)
libovnis_la_HEADERS = ./applications/ovnis-application.h \
	./common/myEnergy-tag.h ./common/myMessageID-header.h ./common/myTxEnergy-tag.h \
	./devices/wifi/channel-cell.h ./devices/wifi/ovnis-adhoc-wifi-mac.h ./devices/wifi/ovnis-error-rate-model.h \
	./devices/wifi/ovnis-wifi-channel.h ./devices/wifi/ovnis-wifi-phy.h \
	./devices/wifi/beaconing-adhoc-wifi-mac.h \
	./helper/ovnis-wifi-helper.h \
	./ovnis-constants.h ./ovnis.h \
	./traci/traci-client.h ./traci/sumoTraciConnection.h ./traci/query/query.h ./traci/query/simStepQuery.h ./traci/traciException.h ./traci/query/subscribeQuery.h ./traci/query/vehicleQuery.h \
	./traci/query/closeQuery.h ./traci/query/edgeQuery.h ./traci/query/laneQuery.h ./traci/query/simulationQuery.h \
	./traci/protocol/command.h ./traci/protocol/status.h \
	./xml-sumo-conf-parser.h ./log.h ./vehicle.h ./scenario.h \
	./traci/storage.h ./traci/socket.h \
	./route.h ./itinerary.h ./edge.h ./edgeInfo.h \
	./recordEntry.h ./ovnisPacket.h \
	./knowledge.h ./applications/trafficInformationSystem.h   \
	./applications/dissemination/dataPacket.h ./applications/dissemination/dissemination.h \
	./common/commonHelper.h \
	./xml_writer.hpp

#libovnis_la_LIBADD = 
libovnis_la_SOURCES = \
                     ${libovnis_la_HEADERS} \
                       ./common/myEnergy-tag.cc ./common/myMessageID-header.cc ./common/myTxEnergy-tag.cc \
                        ./devices/wifi/ovnis-adhoc-wifi-mac.cc ./devices/wifi/ovnis-error-rate-model.cc \
                       ./devices/wifi/ovnis-wifi-channel.cc ./devices/wifi/ovnis-wifi-phy.cc ./devices/wifi/channel-cell.cpp \
                       ./devices/wifi/beaconing-adhoc-wifi-mac.cc  \
						./helper/ovnis-wifi-helper.cc \
                       ./traci/traci-client.cc ./traci/sumoTraciConnection.cpp ./traci/query/query.cpp ./traci/query/simStepQuery.cpp ./traci/traciException.cpp ./traci/query/subscribeQuery.cpp ./traci/query/vehicleQuery.cpp \
                       ./traci/query/closeQuery.cpp ./traci/query/edgeQuery.cpp ./traci/query/laneQuery.cpp ./traci/query/simulationQuery.cpp \
                       ./traci/protocol/command.cpp ./traci/protocol/status.cpp \
                       ./applications/ovnis-application.cpp \
                       ./ovnis.cpp ./xml-sumo-conf-parser.cpp ./log.cpp ./vehicle.cpp ./scenario.cpp \
                       ./traci/storage.cpp ./traci/socket.cpp \
                       ./route.cpp ./itinerary.cpp ./edge.cpp ./edgeInfo.cpp \
                       ./recordEntry.cpp ./ovnisPacket.cpp  \
						./knowledge.cpp ./applications/trafficInformationSystem.cpp \
						./applications/dissemination/dataPacket.cpp ./applications/dissemination/dissemination.cpp \
						./common/commonHelper.cpp \
						./xml_writer.hpp

#libovnis_la_LDFLAGS = -version-info 0:1