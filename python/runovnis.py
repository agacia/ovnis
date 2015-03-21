#!/usr/bin/python

"""A Python program
  usage:
  a) run + analysis
  python '/media/sf_shared/runovnis.py' \
  --sumoConfig=scenario_accident_600_1800.sumocfg \
  --scenarioFolder="/home/agata/Documents/workshop/ovnis/scenarios/Kirchberg/" \
  --outputFolder="/home/agata/Documents/workshop/ovnis/scenarios/Kirchberg/accident_600-1800" \
  --routingStrategiesProbabilities="0,0,0,1" \
  --startTime=0 \
  --stopTime=2400 \
  --accidentStartTime=600 \
  --accidentStopTime=1800 \
  --ttl=120 \
  --runOvnis > log.txt
  b) only analysis
  python '/media/sf_shared/runovnis.py' \
  --scenarioFolder="/home/agata/Documents/workshop/ovnis/scenarios/Kirchberg/" \
  --outputFolder="/home/agata/Documents/workshop/ovnis/scenarios/Kirchberg/1112" \
  --startTime=0 \
  --stopTime=150 \
  --ttl=180

ovnispath=/home/agata/Documents/workshop/ovnis
date=19032015
scenario=Kirchberg
scenario=Highway
knowledge_type=vanet
capacity_type=const_accident
routing="000-100"  # sp-prob  # noRouting,shortest,probabilistic,hybrid
iter=1
# python "${ovnispath}/scripts/runovnis.py" \
python /media/sf_shared/runovnis.py \
  --scenarioFolder="${ovnispath}/scenarios/${scenario}/" \
  --scenario="${scenario}" \
  --outputFolder="${ovnispath}/scenarios/${scenario}/${date}/${knowledge_type}/${estimation}/${capacity_type}/${routing}/${iter}" \
  --sumoConfig=scenario_accident_const.sumocfg \
  --routingStrategiesProbabilities="0,0,1,0" \
  --startTime=0 \
  --stopTime=600 \
  --accidentStartTime=0 \
  --accidentStopTime=3600 \
  --knowledgeType=${knowledge_type} \
  --ttl=120 \
  --runOvnis > log.txt

ovnispath=/home/agata/Documents/workshop/ovnis
date=19032015
scenario=Kirchberg
scenario=Highway
knowledge_type=vanet
capacity_type=const_accident
routing="000-100"  # sp-prob  # noRouting,shortest,probabilistic,hybrid
iter=1
# python "${ovnispath}/scripts/runovnis.py" \
python /media/sf_shared/runovnis.py \
  --scenarioFolder="${ovnispath}/scenarios/Highway/" \
  --scenario="${scenario}" \
  --outputFolder="${ovnispath}/scenarios/Highway/test/" \
  --sumoConfig=scenario_accident_const.sumocfg \
  --routingStrategiesProbabilities="0,0,0,0,1" \
  --startTime=0 \
  --stopTime=3600 \
  --accidentStartTime=0 \
  --accidentStopTime=3600 \
  --knowledgeType=${knowledge_type} \
  --ttl=120 \
  --runOvnis > log.txt

  python /media/sf_shared/runovnis.py \
  --scenarioFolder="${ovnispath}/scenarios/Kirchberg/" \
  --scenario="Kirchberg" \
  --outputFolder="${ovnispath}/scenarios/Kirchberg/test/" \
  --sumoConfig=scenario_accident_const.sumocfg \
  --routingStrategiesProbabilities="0,0,1,0,0,0" \
  --startTime=0 \
  --stopTime=3600 \
  --accidentStartTime=0 \
  --accidentStopTime=3600 \
  --knowledgeType=${knowledge_type} \
  --ttl=120 \
  --runOvnis > log.txt

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
  parser.add_option('--knowledgeType', help=("knowledgeType"), type="string", dest='knowledgeType')
  parser.add_option('--ttl', help=("ttl"), type="int", dest='ttl')
  parser.add_option('--timeEstimationMethod', help=("timeEstimationMethod"), type="string", dest='timeEstimationMethod')
  parser.add_option('--decayFactor', help=("decayFactor"), type="float", dest='decayFactor')
  parser.add_option('-c', '--cluster', help=("cluster"), action="store_true", dest='cluster', default=False)
  parser.add_option('--runOvnis', help=("run ovnis"), action="store_true", dest='runOvnis', default=False)
  parser.add_option('--scenario', help=("scenario"), type="string", dest='scenario')

  (options, args) = parser.parse_args()
  print options

  # default options
  ns3dir = "~/Documents/workshop/ns-3.18/build/"
  ovnisdir = "/Users/agata/workspace/ovnis/"
  sumodir = "~/Documents/workshop/sumo-0.18.0/bin/"
  cluster = options.cluster or False

  # linux 
  ovnisdir = "/home/agata/Documents/workshop/ovnis"
  ns3dir = "/home/agata/Documents/workshop/ns-3.18/build/"
  sumodir = "/home/agata/Documents/workshop/sumo-0.18.0/bin/"

  if cluster:
    ns3dir="/home/users/agrzybek/ovnis/repos/ns-allinone-3.18/ns-3.18/build/"
    ovnisdir="/home/users/agrzybek/ovnis/ovnis/"
    sumodir = "/home/users/agrzybek/ovnis/repos/sumo-0.18.0/bin/"

  ovnisapp = ovnisdir+"/test/ovnisSample_static"
  sumoConfig = "scenario_accident_const.sumocfg"
  scenarioFolder = ovnisdir+"/scenarios/Kirchberg/"
  outputFolder = scenarioFolder+"1111"
  sumoPath = os.path.join(sumodir, "sumo")
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
  accidentStartTime = options.accidentStartTime or startTime
  accidentStopTime = options.accidentStopTime or stopTime
  ttl = options.ttl or 60
  timeEstimationMethod = options.timeEstimationMethod or "last"
  decayFactor = options.decayFactor or 0.5
  knowledgeType = options.knowledgeType or "perfect"
  runOvnis = options.runOvnis or False
  scenario = options.scenario or "Kirchberg"

  # run ovnis
  if runOvnis:
    print "running ovnis "
    args = " --sumoPath=%s --sumoConfig=%s --scenarioFolder=%s --outputFolder=%s --routingStrategiesProbabilities=%s --startTime=%d --stopTime=%d --ttl=%d --timeEstimationMethod=%s --decayFactor=%f --knowledgeType=%s --networkId=%s --accidentStartTime=%d --accidentStopTime=%d" % (sumoPath, sumoConfig, scenarioFolder, outputFolder, routingStrategiesProbabilities, startTime, stopTime, ttl, timeEstimationMethod, decayFactor, knowledgeType, scenario, accidentStartTime, accidentStopTime)
    call = ovnisapp + args
    print "running ovnis", call
    os.system(call)
    # exit(0)

  # add headers to the output file
  filename = "output_log_routing_end"
  routing_end_filepath = os.path.join(outputFolder, filename)
  headers = "arrival\trouteId\tvehicleId\tstartReroute\ttravelTime\tstartEdgeId\tendEdgeId\tvehiclesOnRoute\tisCheater\tselfishExpectedTravelTime\texpectedTravelTime\twasCongested\tdelayTime\troutingStrategy\tstart\tduration\tstaticCost\n"
  output_file = open(routing_end_filepath)
  first_line = output_file.readline()
  if first_line != headers:
    call = "printf \"%s$( cat %s )\" > %s" % (headers, routing_end_filepath, routing_end_filepath)
    print "running ", call
    print "\n"
    os.system(call)
  else:
    print "header line already is there"

  # analyse output file
  scriptpath = "/media/sf_shared"
  script_filepath = os.path.join(scriptpath, "analyse.py")
  call = "python %s --inputFile %s --outputDir %s --scenario %s" % (
      script_filepath, routing_end_filepath, outputFolder+"/",
      scenario)
  print "running ", call
  print "\n"
  os.system(call)

  # analyse error file
  filename = "output_log_edges_error"
  routing_end_filepath = os.path.join(outputFolder, filename)
  script_filepath = os.path.join(scriptpath, "analyse.py")
  call = "python %s --inputFile %s --outputDir %s --scenario %s" % (
      script_filepath, routing_end_filepath, outputFolder+"/",
      scenario)
  print "running ", call
  print "\n"
  os.system(call)

# This is the standard boilerplate that calls the main() function.
if __name__ == '__main__':
  main()



