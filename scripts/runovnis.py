#!/usr/bin/python

"""A Python program 
  usage: 
  python '/media/sf_shared/runovnis.py' \
  --sumoConfig=scenario_accident_const_1111.sumocfg \
  --scenarioFolder="/home/agata/Documents/workshop/ovnis/scenarios/Kirchberg/" \
  --outputFolder="/home/agata/Documents/workshop/ovnis/scenarios/Kirchberg/1111" \
  --routingStrategiesProbabilities="1,0,0,0" \
  --startTime=0 \
  --stopTime=100 \
  --ttl=120

  python '/media/sf_shared/runovnis.py' \
  --sumoConfig=scenario_accident_const_1112.sumocfg \
  --scenarioFolder="/home/agata/Documents/workshop/ovnis/scenarios/Kirchberg/" \
  --outputFolder="/home/agata/Documents/workshop/ovnis/scenarios/Kirchberg/1112" \
  --routingStrategiesProbabilities="0,1,0,0" \
  --startTime=0 \
  --stopTime=150 \
  --ttl=180
  
"""
import sys
import os
from optparse import OptionParser 


# Define a main() function.
def main():
  # print sys.argv
  parser = OptionParser()
  parser.add_option('--sumoConfig', help=("sumoConfig."), type="string", dest="sumoConfig")
  parser.add_option('--sumoPath', help=("sumoPath."), type="string", dest="sumoPath")
  parser.add_option('--sumoPort', help=("sumoPort"), type="int", dest='sumoPort')
  parser.add_option('--scenarioFolder', help=("scenarioFolder."), type="string", dest="scenarioFolder")
  parser.add_option('--outputFolder', help=("outputFolder."), type="string", dest="outputFolder")
  parser.add_option('--routingStrategies', help=("routingStrategies."), type="string", dest="routingStrategies")
  parser.add_option('--routingStrategiesProbabilities', help=("routingStrategiesProbabilities."), type="string", dest="routingStrategiesProbabilities")
  parser.add_option('--startTime', help=("Start time"), type="int", dest='startTime')
  parser.add_option('--stopTime', help=("End time"), type="int", dest='stopTime')
  parser.add_option('--accidentStartTime', help=("accidentStartTime"), type="int", dest='accidentStartTime')
  parser.add_option('--accidentStopTime', help=("accidentStopTime"), type="int", dest='accidentStopTime')
  parser.add_option('--penetrationRate', help=("penetrationRate"), type="int", dest='penetrationRate')
  parser.add_option('--knowledgePenetrationRate', help=("knowledgePenetrationRate"), type="int", dest='knowledgePenetrationRate')
  parser.add_option('--vanetDisseminationPenetrationRate', help=("vanetDisseminationPenetrationRate"), type="int", dest='vanetDisseminationPenetrationRate')
  parser.add_option('--cheatersRatio', help=("cheatersRatio"), type="int", dest='cheatersRatio')
  parser.add_option('--usePerfect', help=("usePerfect"), type="string", dest='usePerfect')
  parser.add_option('--ttl', help=("ttl"), type="int", dest='ttl')
  parser.add_option('--timeEstimationMethod', help=("timeEstimationMethod"), type="string", dest='timeEstimationMethod')
  parser.add_option('--decayFactor', help=("decayFactor"), type="float", dest='decayFactor')
  parser.add_option('--cluster', help=("cluster"), action="store_false", dest='cluster')
  
  (options, args) = parser.parse_args()
  print options

  # default options
  ns3dir = "~/Documents/workshop/ns-3.18/build/"
  ovnisdir = "~/Documents/workshop/ovnis"
  sumodir = "~/Documents/workshop/sumo-0.18.0/bin/"
  cluster = options.cluster or False
  if cluster: 
    ns3dir="/home/users/agrzybek/ovnis/repos/ns-allinone-3.18/ns-3.18/build/"
    ovnisdir="/home/users/agrzybek/ovnis/ovnis/"


  ovnisapp = ovnisdir+"/test/ovnisSample_static"
  sumoConfig = "scenario_accident_const.sumocfg"
  scenarioFolder = ovnisdir+"/scenarios/Kirchberg/"
  outputFolder = scenarioFolder+"1111"

  print "cluster? ", cluster
  print "ns3 dir " , ns3dir
  print "ovnis dir", ovnisdir

  # overwrite with command line options
  sumoConfig = options.sumoConfig or sumoConfig
  scenarioFolder = options.scenarioFolder or scenarioFolder
  outputFolder = options.outputFolder or outputFolder
  routingStrategiesProbabilities = options.routingStrategiesProbabilities or "1,0,0,0"
  startTime = options.startTime or 0
  stopTime = options.stopTime or 10
  ttl = options.ttl or 60
  timeEstimationMethod = options.timeEstimationMethod or "last"
  decayFactor = options.decayFactor or 0.5

  # run ovnis   
  args = " --sumoConfig=%s --scenarioFolder=%s --outputFolder=%s --routingStrategiesProbabilities=%s --startTime=%d --stopTime=%d --ttl=%d --timeEstimationMethod=%s --decayFactor=%f" % (sumoConfig, scenarioFolder, outputFolder, routingStrategiesProbabilities, startTime, stopTime, ttl, timeEstimationMethod, decayFactor)
  call = ovnisapp + args
  print "running ", call
  os.system(call)

  # add headers to the output file
  filename = "output_log_routing_end"
  routing_end_filepath = os.path.join(outputFolder, filename)
  headers = "arrival\trouteId\tvehicleId\tstartReroute\ttravelTime\tstartEdgeId\tendEdgeId\tvehiclesOnRoute\tisCheater\tselfishExpectedTravelTime\texpectedTravelTime\twasCongested\tdelayTime\troutingStrategy\tstart\tduration\tstaticCost\n"
  output_file = open(routing_end_filepath)
  first_line = output_file.readline()
  if first_line != headers:
    call = "printf \"%s$( cat %s )\" > %s" % (headers, routing_end_filepath, routing_end_filepath)
    print "running ", call 
    os.system(call)
  else:
    print "header line already is there" 
  
  # analyse output file
  script_filepath = os.path.join(ovnisdir, "python", "analyse.py")
  call = "python %s --inputFile %s --outputDir %s" %(script_filepath, routing_end_filepath, outputFolder+"/")
  print "running ", call
  os.system(call)

# This is the standard boilerplate that calls the main() function.
if __name__ == '__main__':
  main()



