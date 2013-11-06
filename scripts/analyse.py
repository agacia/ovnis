#!/usr/bin/python -tt
# Copyright 2010 Google Inc.
# Licensed under the Apache License, Version 2.0
# http://www.apache.org/licenses/LICENSE-2.0

# Google's Python Class
# http://code.google.com/edu/languages/google-python-class/

"""A Python program to analyse Sumo and OVNIS outpuy.
Try running this program from the command line:
  python hello.py
  python hello.py Alice
"""
import sys
from lxml import etree
from xml.sax.saxutils import XMLGenerator
from xml.sax.xmlreader import AttributesNSImpl
from optparse import OptionParser 
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.mlab import csv2rec
import re
from pylab import *
import numpy as np
from numpy import * 
import operator
from operator import itemgetter, attrgetter
from matplotlib.finance import candlestick

linestyles = ['-.', '-', '--', ':']
colors = ('b', 'g', 'r', 'm', 'c', 'y', 'k')
styles = [
  'o',
  'v',
  '^',
  'o',
    r'$\lambda$',
    r'$\bowtie$',
    r'$\circlearrowleft$',
    r'$\clubsuit$',
    r'$\checkmark$']

# Define a main() function.
def main():
  # print sys.argv
  parser = OptionParser()
  parser.add_option('--inputFile', help=("inputFile."), type="string", dest="inputFile")
  parser.add_option('--inputFiles', help=("inputFiles."), type="string", dest="inputFiles")
  parser.add_option('--inputCapacityFiles', help=("inputCapacityFiles."), type="string", dest="inputCapacityFiles")
  parser.add_option('--inputCheaterFiles', help=("inputCheaterFiles."), type="string", dest="inputCheaterFiles")
  parser.add_option('--inputAverageCheatersFile', help=("inputAverageCheatersFile."), type="string", dest="inputAverageCheatersFile")
  parser.add_option('--labels', help=("Label titles"), type="string", dest='labelTitles')
  parser.add_option('--outputFile', help=("outputFile."), type="string", dest="outputFile")
  parser.add_option('--outputDir', help=("outputDir."), type="string", dest="outputDir")
  parser.add_option('--rootTagName', help=("Root Tag name."), type="string", dest="rootTagName")
  parser.add_option('--tagName', help=("Tag name."), type="string", dest="tagName")
  parser.add_option('--startX', help=("Start time"), type="int", dest='startX')
  parser.add_option('--endX', help=("End time"), type="int", dest='endX')
  parser.add_option('--stepSize', help=("Step size"), type="int", dest='stepSize')
  (options, args) = parser.parse_args()
  print options

  args = {} 
  args["startX"] = options.startX
  args["endX"] = options.endX
  args['tagName'] = options.tagName
  args['rootTagName'] = options.rootTagName
  args['stepSize'] = options.stepSize
  args['xLabel'] = 'time'
  # args['data'] = []
  args['data'] = {}
  delimiterStr = "\t"
  avg_output_file = options.outputDir + "avg.txt"
  routing_names = ['time', 'routeId', 'vehicleId', 'startReroute', 'travelTime', 'startEdgeId', 'endEdgeId', 
      'vehiclesOnRoute', 'isCheater', 'selfishExpectedTravelTime', 'expectedTravelTime', 'wasCongested', 'delayTime',
      'routingStrategy', 'startTravelTime', 'totalTravelTime']
  num_names = [ 'travelTime', 'vehiclesOnRoute', 'isCheater', 'selfishExpectedTravelTime', 'expectedTravelTime', 
  'wasCongested', 'delayTime', 'totalTravelTime' ]
  clusterIndexes = {
        'routeId':routing_names.index('routeId'), 
        'selfishExpectedTravelTime': routing_names.index('selfishExpectedTravelTime'),
        'isCheater' : routing_names.index('isCheater')}
      
  if options.inputFile:
    inputFile = options.inputFile
    if ".tsv" in inputFile:
      data = csv2rec(inputFile, delimiter='\t', names=names)
      # print data.dtype.names
      xData = data['flow']
      yData = [data['main'], data['bypass']]
      xLabel = "Flow"
      yLabel = "Average travel time "
      yLabels = ["Main", "Bypass"]
      # plotType = "scatterplot"
      # outputFile =  options.outputDir + name + "/routing_end_traveltime_number.png"
      # plot(plotType, xData, yData, xLabel, yLabel, yLabels, outputFile)
      plotType = "line"
      outputFile =  options.outputDir + "capacity_main_bypas.png"
      plot(plotType, xData, yData, xLabel, "Average travel time", yLabels, outputFile)
   

  if options.inputFiles:
    inputFiles = options.inputFiles.split(' ')
    inputLabels = options.labelTitles.split(' ')
    if not inputFiles or not inputFiles[0]:
      return
    if "routing_end" in inputFiles[0]:
      xIndex = 1
      cheaterIndex = 8
      shortestExpectedTravelTimeIndex = 9
      expectedTravelTimeIndex = 10
      orderingColumn = 'startReroute'
       
      cutColumnName = 'time'
      xBoundaries = [300,1300]
      inputData = readData(inputFiles, inputLabels, delimiterStr, routing_names, num_names, avg_output_file, cutColumnName, xBoundaries)
      
      clusters = {}
      clusterNames = ['cheater', 'noncheater']
      for clusterName in clusterNames:
        clusters[clusterName] = {}
        clusters[clusterName]['avgTravelTime'] = []
        clusters[clusterName]['count'] = []
        clusters[clusterName]['avgTravelTime_cut'] = []
        clusters[clusterName]['count_cut'] = []
     
      for name,data in inputData.iteritems():
        cutIndexes = data['cutIndexes']
        print name, " cutIndexes: ", cutIndexes
        clusteredData = processRoutingData(data['recarray'], clusterIndexes, routing_names, num_names, orderingColumn, cutIndexes)
        data['clusteredData'] = clusteredData
        xData = []
        yData = []
        yLabels = []
        if "main" in clusteredData.keys():
          xData.append(clusteredData['main']['columns']['time'])
          yData.append(clusteredData['main']['columns']['travelTime'])
          yLabels.append("Travel time - main")
        if "bypass" in clusteredData.keys():
          xData.append(clusteredData['bypass']['columns']['time'])
          yData.append(clusteredData['bypass']['columns']['travelTime'])
          yLabels.append("Travel time - bypass")
        if "main" in clusteredData.keys():  
          yData.append(clusteredData['main']['columns']['vehiclesOnRoute'])
          yLabels.append("Nuber of vehicles - main")
        if "bypass" in clusteredData.keys():
          yData.append(clusteredData['bypass']['columns']['vehiclesOnRoute'])
          yLabels.append("Nuber of vehicles - bypass")

        xLabel = "Simulation time"
        yLabel = ""
        linesX = [300,1300]
        # plotType = "scatterplot"
        # outputFile =  options.outputDir + name + "/routing_end_traveltime_number.png"
        # plot(plotType, xData, yData, xLabel, yLabel, yLabels, outputFile)
        plotType = "line"
        outputFile =  options.outputDir + name + "/routing_end_line_traveltime_number.png"
        plot(plotType, xData, yData, xLabel, "", yLabels, outputFile, linesX)
        
        if "main" in clusteredData.keys() and "bypass" in clusteredData.keys():
          yData = [clusteredData['main']['columns']['price'], clusteredData['bypass']['columns']['price']]
          yLabels = ["Price - main", "Price - bypass"]
          outputFile =  options.outputDir + name + "/routing_end_line_price_number.png"
          plot(plotType, xData, yData, xLabel, "", yLabels, outputFile, linesX)
        
          yData = [clusteredData['main']['columns']['travelTime'], clusteredData['bypass']['columns']['travelTime'], clusteredData['main']['columns']['price'], clusteredData['bypass']['columns']['price']]
          yLabels = ["Travel time - main", "Travel time - bypass", "Price - main", "Price - bypass"]
          outputFile =  options.outputDir + name + "/routing_end_line_price_traveltime_number.png"
          plot(plotType, xData, yData, xLabel, "", yLabels, outputFile, linesX)

          yData = [clusteredData['main']['columns']['error'], clusteredData['bypass']['columns']['error']]
          yLabels = ["Error - main", "Error - bypass"]
          outputFile =  options.outputDir + name + "/routing_end_line_error_number.png"
          plot(plotType, xData, yData, xLabel, "", yLabels, outputFile, linesX)
        
        for clusterName in clusterNames:
          if clusterName in clusteredData.keys():
            clusters[clusterName]['avgTravelTime'].append(clusteredData[clusterName]['stats']['travelTime']['average'])
            clusters[clusterName]['count'].append(clusteredData[clusterName]['stats']['travelTime']['count'])
            if 'average_cut' in clusteredData[clusterName]['stats']['travelTime'].keys():
              clusters[clusterName]['avgTravelTime_cut'].append(clusteredData[clusterName]['stats']['travelTime']['average_cut'])
            if 'count_cut' in clusteredData[clusterName]['stats']['travelTime'].keys():
              clusters[clusterName]['count_cut'].append(clusteredData[clusterName]['stats']['travelTime']['count_cut'])
      
      # out = open(options.outputDir + "cheaters.txt", 'w')
      out = open(avg_output_file, 'a+')
      for clusterName in clusterNames:
        if len(clusters[clusterName]['avgTravelTime']) > 0:
          avg = np.average(clusters[clusterName]['avgTravelTime'])
          min = np.min(clusters[clusterName]['avgTravelTime'])
          max = np.max(clusters[clusterName]['avgTravelTime'])
          std = np.std(clusters[clusterName]['avgTravelTime'])
          out.write("{0}\t{1}\t{2}\t{3}\t{4}\n".format(clusterName + "TravelTime", avg, min, max, std))
        else:
          out.write("{0}\t{1}\t{2}\t{3}\t{4}\n".format(clusterName + "TravelTime", 0,0,0,0))
        if len(clusters[clusterName]['count']) > 0:
          avg = np.average(clusters[clusterName]['count'])
          min = np.min(clusters[clusterName]['count'])
          max = np.max(clusters[clusterName]['count'])
          std = np.std(clusters[clusterName]['count'])
          out.write("{0}\t{1}\t{2}\t{3}\t{4}\n".format(clusterName + "Count", avg, min, max, std))
        else:
          out.write("{0}\t{1}\t{2}\t{3}\t{4}\n".format(clusterName + "Count", 0,0,0,0))
        
        out.write("{2} cut\t{0}\t{1}\n".format(xBoundaries[0], xBoundaries[1], cutColumnName))
        if len(clusters[clusterName]['avgTravelTime_cut']) > 0:
          avg = np.average(clusters[clusterName]['avgTravelTime_cut'])
          min = np.min(clusters[clusterName]['avgTravelTime_cut'])
          max = np.max(clusters[clusterName]['avgTravelTime_cut'])
          std = np.std(clusters[clusterName]['avgTravelTime_cut'])
          out.write("{0}\t{1}\t{2}\t{3}\t{4}\n".format(clusterName + "TravelTimeCut", avg, min, max, std))
        else:
          out.write("{0}\t{1}\t{2}\t{3}\t{4}\n".format(clusterName + "TravelTimeCut", 0,0,0,0))
        if len(clusters[clusterName]['count_cut']) > 0:
          avg = np.average(clusters[clusterName]['count_cut'])
          min = np.min(clusters[clusterName]['count_cut'])
          max = np.max(clusters[clusterName]['count_cut'])
          std = np.std(clusters[clusterName]['count_cut'])
          out.write("{0}\t{1}\t{2}\t{3}\t{4}\n".format(clusterName + "CountCut", avg, min, max, std))
        else:
          out.write("{0}\t{1}\t{2}\t{3}\t{4}\n".format(clusterName + "CountCut", 0,0,0,0))
         
    
    if "global_costs" in inputFiles[0]:
      orderingColumn = 'time'
      names = ['time', 'bypass', 'main']
      num_names = [ 'travelTime', 'vehiclesOnRoute']
      inputData = readData(inputFiles, inputLabels, delimiterStr, names, num_names, avg_output_file)
      for name,data in inputData.iteritems():
        clusteredData = processGlobalCostsData(data['recarray'], ['bypass','main'], num_names, ",", orderingColumn)
        data['clusteredData'] = clusteredData
         # plot
        xData = [clusteredData['main']['columns']['time'], clusteredData['bypass']['columns']['time']]
        yData = [clusteredData['main']['columns']['travelTime'], clusteredData['bypass']['columns']['travelTime'], clusteredData['main']['columns']['vehiclesOnRoute'], clusteredData['bypass']['columns']['vehiclesOnRoute']]
        xLabel = "Simulation time"
        yLabel = ""
        yLabels = ["Travel time - main", "Travel time - bypass", "Nuber of vehicles - main", "Number of vehicless - bypass"]
        outputFile =  options.outputDir + name + "/global_costs_line_traveltime_number.png"
        plotType = "line"
        plot(plotType, xData, yData, xLabel, "", yLabels, outputFile, linesX)
     
        plotType = "scatterplot"
        outputFile =  options.outputDir + name + "/global_costs_traveltime_number.png"
        plot(plotType, xData, yData, xLabel, yLabel, yLabels, outputFile, linesX)
        yData = [clusteredData['main']['columns']['travelTime'], clusteredData['bypass']['columns']['travelTime']]
        outputFile = "{0}/{1}/plot_global_traveltime.png".format(options.outputDir,name)
        yLabel = "Travel time"
        plot(plotType, xData, yData, xLabel, yLabel, yLabels, outputFile, linesX)
    
    calculateAvg(inputData, num_names, options.outputFile)

  if options.inputCheaterFiles:
    inputFiles = options.inputCheaterFiles.split(' ')
    inputLabels = options.labelTitles.split(' ')
    if not inputFiles or not inputFiles[0]:
      return
    if len(inputFiles) != len(inputLabels):
      print "ERROR. Input files: {0}, inpute labels: {1}".format(len(inputFiles), len(inputLabels))
      return
    output_file = options.outputFile
    out = open(output_file, 'w')
    names = ['name', 'avg', 'min_avg', 'max_avg', 'std_avg', 'min', 'std_min', 'mean', 'mean_std', 'max', 'std_max', 'sum', 'std_sum']
    plot_names = ['travelTime', 'travelTimeCut', 
    'cheaterTravelTime', 'cheaterCount', 'cheaterTravelTimeCut', 'cheaterCountCut', 
    'noncheaterTravelTime', 'noncheaterCount', 'noncheaterTravelTimeCut', 'noncheaterCountCut']
    plotValues = {}
    for name in plot_names:
      plotValues[name] = {}
    cheatersPercents = []
    for label in inputLabels:
      label = label.strip()
      if (label != ""):
        for fileName in inputFiles:
          if label in fileName:
            cheaterPercent = float(re.sub(r'/', '', label))
            print "cheaterPercent", cheaterPercent
            break
        data = csv2rec(fileName, delimiter=delimiterStr, names=names)
        cheatersPercents.append(cheaterPercent)
        i = 0
        print "checking following data ", data['name']
        for name in data['name']:
          if name in plot_names:
            plotValues[name][cheaterPercent] = {}
            plotValues[name][cheaterPercent]['avg'] = data['avg'][i]
            plotValues[name][cheaterPercent]['min'] = data['min_avg'][i]
            plotValues[name][cheaterPercent]['max'] = data['max_avg'][i]
            print "percent {0}\t name {1}, avg {2}, min {3}, max {4}".format(cheaterPercent, name, 
              plotValues[name][cheaterPercent]['avg'], plotValues[name][cheaterPercent]['min'], plotValues[name][cheaterPercent]['max'])          
          i += 1  
    # write results          
    out.write("cheaters_percent")
    for name in plot_names:
      out.write("\t{0}_avg\t{0}_min\t{0}_max".format(name))
    out.write("\n")
    cheatersPercents.sort()
    for cheaterPercent in cheatersPercents:
      out.write("{0}".format(cheaterPercent))
      for name in plot_names:
        out.write("\t{0}\t{1}\t{2}".format(plotValues[name][cheaterPercent]['avg'], plotValues[name][cheaterPercent]['min'], plotValues[name][cheaterPercent]['max']))
      out.write("\n")

  if options.inputAverageCheatersFile:
    fileName = options.inputAverageCheatersFile
    names=['cheaters_percent','travelTime_avg','travelTime_min','travelTime_max',
    'travelTimeCut_avg','travelTimeCut_min','travelTimeCut_max',
    'cheaterTravelTime_avg','cheaterTravelTime_min','cheaterTravelTime_max','cheaterCount_avg','cheaterCount_min','cheaterCount_max',
    'cheaterTravelTimeCut_avg','cheaterTravelTimeCut_min','cheaterTravelTimeCut_max','cheaterCountCut_avg','cheaterCountCut_min','cheaterCountCut_max',
    'noncheaterTravelTime_avg','noncheaterTravelTime_min','noncheaterTravelTime_max','noncheaterCount_avg','noncheaterCount_min','noncheaterCount_max',
    'noncheaterTravelTimeCut_avg','noncheaterTravelTimeCut_min','noncheaterTravelTimeCut_max','noncheaterCountCut_avg','noncheaterCountCut_min','noncheaterCountCut_max']
    data = csv2rec(fileName, delimiter=delimiterStr, names=names, skiprows=1)
    xData = data["cheaters_percent"]
    plotType = "line"
    xLabel = "Precent of cheaters"
    
    plotnames = ["travelTime"]
    yData = []
    yLabel = "Average travel time"
    yLabels = []
    yMins = []
    yMaxs = []
    for plotname in plotnames:
      yData.append(data[plotname+"_avg"])
      yLabels.append(yLabel)
      yMins.append(data[plotname+"_min"])
      yMaxs.append(data[plotname+"_max"])
    outputFile =  options.outputDir + "cheaters_avg_"+plotnames[0]+".png"
    plot(plotType, xData, yData, xLabel, yLabel, yLabels, outputFile, None, yMins, yMaxs, True)
    # plot(plotType, xData, yData, xLabel, yLabel, yLabels, outputFile, None, yMins, yMaxs)

    plotnames = ["travelTime","cheaterTravelTime", "noncheaterTravelTime"]
    plottitles = ["All", "Cheaters", "TrafficEQ-hybrid"]  
    yData = []
    yLabel = "Average travel time"
    yLabels = []
    yMins = []
    yMaxs = []
    i = 0
    for plotname in plotnames:
      yData.append(data[plotname+"_avg"])
      yLabels.append(plottitles[i])
      yMins.append(data[plotname+"_min"])
      yMaxs.append(data[plotname+"_max"])
      i += 1
    outputFile =  options.outputDir + "cheaters_"+plotnames[0]+".png"
    plot(plotType, xData, yData, xLabel, yLabel, yLabels, outputFile, None, None, None, True)
    # plot(plotType, xData, yData, xLabel, yLabel, yLabels, outputFile, None, yMins, yMaxs)

    plotnames = ["travelTimeCut", "cheaterTravelTimeCut", "noncheaterTravelTimeCut"]
    plottitles = ["All", "Cheaters", "TrafficEQ-hybrid"]
    yData = []
    yLabel = "Average travel time (cut)"
    yLabels = []
    yMins = []
    yMaxs = []
    i = 0
    for plotname in plotnames:  
      yData.append(data[plotname+"_avg"])
      yLabels.append(plottitles[i])
      yMins.append(data[plotname+"_min"])
      yMaxs.append(data[plotname+"_max"])
      i += 1
    outputFile =  options.outputDir + "cheaters_"+plotnames[0]+".png"
    plot(plotType, xData, yData, xLabel, yLabel, yLabels, outputFile, None, None, None, True)
    outputFile =  options.outputDir + "cheaters_"+plotnames[0]+"_minmax.png"
    plot(plotType, xData, yData, xLabel, yLabel, yLabels, outputFile, None, yMins, yMaxs, True)


  if options.inputCapacityFiles:
    inputFiles = options.inputCapacityFiles.split(' ')
    inputLabels = options.labelTitles.split(' ')
    if not inputFiles or not inputFiles[0]:
      return
    if "routing_end" in inputFiles[0]:
      inputData = readData(inputFiles, inputLabels, delimiterStr, routing_names, num_names, avg_output_file)
      flows = []
      avgTotalTravelTimes = {}
      out = open(options.outputDir+'averageTravelTimes.txt', 'w')
      out.write("flow\tavg total travel time\n")
    
      for name,data in inputData.iteritems():
        # data['recarray']['totalTravelTime']
        recarrayData = data['recarray']
        xData = [recarrayData['time']]
        yData = [recarrayData['totalTravelTime']]
        # plotType = "scatterplot"
        # xLabel = "Simulation time"
        # yLabel = ""
        # yLabels = ["Travel time"]
        # linesX = [300,1300]
        # outputFolder = options.outputDir + name + "/"
        # outputFile =  outputFolder + "total_traveltime.png"
        # plot(plotType, xData, yData, xLabel, yLabel, yLabels, outputFile, linesX)
        avgTotalTravelTime = np.average(yData)
        flowNumber = float(name)
        flows.append(flowNumber)
        avgTotalTravelTimes[flowNumber] = avgTotalTravelTime

        # yData = [recarrayData['travelTime']]
        # outputFile =  outputFolder + "traveltime.png"
        # plot(plotType, xData, yData, xLabel, yLabel, yLabels, outputFile, linesX)

      flows.sort()
      sortedAvgTravelTimes = []
      for flow in flows:
        sortedAvgTravelTimes.append(avgTotalTravelTimes[flow])
        out.write("{0}\t{1}\n".format(flow, avgTotalTravelTimes[flow]))
      xData = flows
      yData = [sortedAvgTravelTimes]
      plotType = "line"
      xLabel = "Flow"
      yLabel = "Average travel time"
      yLabels = ["Average travel time"]
      outputFile =  options.outputDir + "traveltime_flow.png"
      plot(plotType, xData, yData, xLabel, yLabel, yLabels, outputFile)
        

  if options.inputFile:
    inputFile = options.inputFile
    # read main xml data
    if ".xml" in inputFile:
      context = etree.iterparse(inputFile, events=('end',), tag=args['tagName'])
      fastIter(context, processElem, args)
      data = args['data'].copy()
      
      array = np.array(map(float, data['meanTravelTime']))
      y = np.array(filter(lambda x: x >= 0, array))  
      out = open(options.outputFile, 'w')
      out.write("meanTravelTime\n")
      out.write("len \t{0}\n".format(len(y)))
      out.write("avg meanTravelTime\t{0}\n".format(np.average(y)))
      out.write("min meanTravelTime\t{0}\n".format(np.min(y)))
      out.write("max meanTravelTime\t{0}\n".format(np.max(y)))
      out.write("std meanTravelTime\t{0}\n".format(np.std(y)))
      y = np.array(map(float, data['loaded']))
      out.write("loaded\n")
      out.write("len\t{0}\n".format(len(y)))
      out.write("avg\t{0}\n".format(np.average(y)))
      out.write("min\t{0}\n".format(np.min(y)))
      out.write("max\t{0}\n".format(np.max(y)))
      out.write("std\t{0}\n".format(np.std(y)))
      out.write("total\t{0}\n".format(y[len(y)-1]))
      y = np.array(map(float, data['emitted']))
      out.write("emitted\n")
      out.write("len\t{0}\n".format(len(y)))
      out.write("avg\t{0}\n".format(np.average(y)))
      out.write("min\t{0}\n".format(np.min(y)))
      out.write("max\t{0}\n".format(np.max(y)))
      out.write("std\t{0}\n".format(np.std(y)))
      out.write("total\t{0}\n".format(y[len(y)-1]))
      out.write("waiting\t{0}\n".format(data['waiting'][len(data['waiting'])-1]))
      avg = np.average(y)
      min = np.min(y)

      args['data'] = {}
      plot("line", data['time'], [data['loaded'], data['emitted']], "Simulation time", "Number of vehicles", ["loaded", "emitted"], options.outputDir + "plot_loaded.png")
      plot("line", data['time'], [data['meanTravelTime']], "Simulation time", "Averag travel time", ["Average travel time"], options.outputDir + "plot_meantraveltime.png")
    

    if "routing_end" in inputFile:
      data = csv2rec(inputFile, delimiter='\t', names=routing_names)
      print clusterIndexes
      data = processRoutingData(data, clusterIndexes, routing_names, num_names)
      plotType = "scatterplot"
      xData = [data['main']['columns']['time'], data['bypass']['columns']['time']]
      yData = [data['main']['columns']['travelTime'], data['bypass']['columns']['travelTime'], data['main']['columns']['vehiclesOnRoute'], data['bypass']['columns']['vehiclesOnRoute']]
      xLabel = "Time"
      yLabel = ""
      yLabels = ["Travel time - main", "Travel time - bypass", "Nuber of vehicles - main", "Number of vehicless - bypass"]
      linesX = [300,1300]
      outputFile =  options.outputDir + "plot_traveltime_number.png"
      plot(plotType, xData, yData, xLabel, yLabel, yLabels, outputFile, linesX)

      yData = [data['main']['columns']['travelTime'], data['bypass']['columns']['travelTime']]
      outputFile = options.outputDir + "plot_traveltime.png"
      plot(plotType, xData, yData, xLabel, yLabel, yLabels, outputFile, linesX)
    

    return
    
# --------------------------------------------  

def readData(files, labels, delimiterStr, names, num_names, avg_output_file, xColumnName = None, xBoundaries = None):
  data = {}
  if len(files) != len(labels):
    print "ERROR. Input files: {0}, inpute labels: {1}".format(len(files), len(labels))
    return data
  out = open(avg_output_file, 'w')
  print "Write avrage of {0} to {1}".format(len(files), avg_output_file)
  stats = {}
  for num_name in num_names:
    stats[num_name] = {}
    stats[num_name]['min'] = []
    stats[num_name]['mean'] = []
    stats[num_name]['avg'] = []
    stats[num_name]['max'] = []
    stats[num_name]['std'] = []
    stats[num_name]['sum'] = []
    if xBoundaries:
      stats[num_name]['min_cut'] = []
      stats[num_name]['mean_cut'] = []
      stats[num_name]['avg_cut'] = []
      stats[num_name]['max_cut'] = []
      stats[num_name]['std_cut'] = []
      stats[num_name]['sum_cut'] = []
  stats['N'] = []
  if xBoundaries:
    stats['N_cut'] = []

  # find name of data set
  dataSetName = ""
  for label in labels:
    label = label.strip()
    if (label != ""):
      for fileName in files:
        if label in fileName:
          dataSetName = re.sub(r'/', '', label)
          break
      fixTabs(fileName)
      data[dataSetName] = {}
      data[dataSetName]['fileName'] = fileName
      data[dataSetName]['name'] = dataSetName
      data[dataSetName]['recarray'] = csv2rec(fileName, delimiter=delimiterStr, names=names)
      if not xColumnName:
        xColumnName = data[dataSetName]['recarray'].dtype.names[0]
      array = data[dataSetName]['recarray'][xColumnName]
      stats['N'].append(len(array))
      cutIndexes = [-1, -1]
      data[dataSetName]['cutIndexes'] = cutIndexes
      if xBoundaries and len(xBoundaries)==2:
        i = 0
        for x in array:
          if cutIndexes[0] == -1 and x >= xBoundaries[0] and x <= xBoundaries[1]:
            cutIndexes[0] = i
          if cutIndexes[1] == -1 and x >= xBoundaries[1]:
            cutIndexes[1] = i
          i += 1
        if cutIndexes[0] == -1:
          cutIndexes[0] = 0
        if cutIndexes[1] == -1:
          cutIndexes[1] = 0
        subarray = array[cutIndexes[0]:cutIndexes[1]]
        stats['N_cut'].append(len(subarray))
      
      for num_name in num_names:
        array = data[dataSetName]['recarray'][num_name]
        # print "name {0}: {1}".format(num_name, array)
        subarray = array[cutIndexes[0]:cutIndexes[1]]
        stats[num_name]['min'].append(np.min(array))
        stats[num_name]['mean'].append(np.mean(array))
        stats[num_name]['avg'].append(np.average(array))
        stats[num_name]['max'].append(np.max(array))
        stats[num_name]['std'].append(np.std(array))
        stats[num_name]['sum'].append(np.sum(array))
        if xBoundaries and len(subarray) > 0:
          stats[num_name]['min_cut'].append(np.min(subarray))
          stats[num_name]['mean_cut'].append(np.mean(subarray))
          stats[num_name]['avg_cut'].append(np.average(subarray))
          stats[num_name]['max_cut'].append(np.max(subarray))
          stats[num_name]['std_cut'].append(np.std(subarray))
          stats[num_name]['sum_cut'].append(np.sum(subarray))
  
  # writr results          
  out.write("\tavg\t(min)\t(max)\t(std)\tmin\t(std)\tmean\t(std)\tmax\t(std)\tstd\t(std)\tsum\t(std)\n")
  for num_name in num_names:
    statarray = np.array(stats[num_name]['avg'])
    out.write("{0}\t{1}\t{2}\t{3}\t{4}".format(num_name, np.average(statarray), np.min(statarray), np.max(statarray), np.std(statarray)))
    statarray = np.array(stats[num_name]['min'])
    out.write("\t{1}\t{2}".format(num_name, np.average(statarray), np.std(statarray)))
    statarray = np.array(stats[num_name]['mean'])
    out.write("\t{1}\t{2}".format(num_name, np.average(statarray), np.std(statarray)))
    statarray = np.array(stats[num_name]['max'])
    out.write("\t{1}\t{2}".format(num_name, np.average(statarray), np.std(statarray)))
    statarray = np.array(stats[num_name]['std'])
    out.write("\t{1}\t{2}".format(num_name, np.average(statarray), np.std(statarray)))
    statarray = np.array(stats[num_name]['sum'])
    out.write("\t{1}\t{2}\n".format(num_name, np.average(statarray), np.std(statarray)))
    if xBoundaries:
      statarray = np.array(stats[num_name]['avg_cut'])
      out.write("{0}\t{1}\t{2}\t{3}\t{4}".format(num_name+"Cut", np.average(statarray), np.min(statarray), np.max(statarray), np.std(statarray)))
      statarray = np.array(stats[num_name]['min_cut'])
      out.write("\t{1}\t{2}".format(num_name, np.average(statarray), np.std(statarray)))
      statarray = np.array(stats[num_name]['mean_cut'])
      out.write("\t{1}\t{2}".format(num_name, np.average(statarray), np.std(statarray)))
      statarray = np.array(stats[num_name]['max_cut'])
      out.write("\t{1}\t{2}".format(num_name, np.average(statarray), np.std(statarray)))
      statarray = np.array(stats[num_name]['std_cut'])
      out.write("\t{1}\t{2}".format(num_name, np.average(statarray), np.std(statarray)))
      statarray = np.array(stats[num_name]['sum_cut'])
      out.write("\t{1}\t{2}\n".format(num_name, np.average(statarray), np.std(statarray)))
    
  num_name = 'N'
  statarray = np.array(stats['N'])
  out.write("{0}\t{1}\t{2}\n".format(num_name, np.average(statarray), np.std(statarray)))
  if xBoundaries:
    statarray = np.array(stats['N_cut'])
    out.write("{0}\t{1}\t{2}\n".format('N_cut', np.average(statarray), np.std(statarray)))
    out.write("cutIndexes\t{0}\t{1}\n".format(cutIndexes[0], cutIndexes[1]))
  return data

def processRoutingData(data, clusterIndexes, columnnames, num_columnnames, sortColumn="", cutIndexes=None):
  if sortColumn:
    data = np.sort(data, order=sortColumn)
  
  clusteredData = {}
  drivercolumns = ['price','error','error_sqrt_root']
  
  if not cutIndexes:
    cutIndexes = [0,len(data)-1]

  i = 0
  cutValues =[data['time'][cutIndexes[0]], data['time'][cutIndexes[1]]]
  print "cutIndexes: {0} , time values: {1}".format(cutIndexes, cutValues)
  for row in data:
    ind = 9
    val  = row[ind]
    val = row[ind+1] - val > 0.5
    price = row[ind+1] - row[ind]
    error = row[4] - row[ind+1]
    sqrt_error = math.sqrt(math.pow(row[4] - row[ind+1], 2))
    for clusterIndexName,clusterIndex in clusterIndexes.iteritems():
      clusterValue  = row[clusterIndex]
      clusterName = columnnames[clusterIndex]
      clusterValue = row[clusterIndex]
      if columnnames[clusterIndex] == "selfishExpectedTravelTime":
        clusterValue = row[clusterIndex+1] - clusterValue > 0.5
        if clusterValue == True:
          clusterValue = "sacrificed" 
        else: 
          clusterValue = "nonsacrificed"
      if columnnames[clusterIndex] == "isCheater": 
        clusterValue = row[clusterIndex] == 1
        if clusterValue == True:
          clusterValue = "cheater" 
        else: 
          clusterValue = "noncheater"
      # initialize cluster             
      if not clusterValue in clusteredData.keys():
        clusteredData[clusterValue] = {}
        clusteredData[clusterValue]['columns'] = {}
        clusteredData[clusterValue]['cutIndexes'] = [-1,-1]
        clusteredData[clusterValue]['stats'] = {}
        for columnname in columnnames:
          clusteredData[clusterValue]['columns'][columnname] = []  
        for columnname in drivercolumns:
          clusteredData[clusterValue]['columns'][columnname] = [] 
        # print "Initialising cluster {0}".format(clusterValue, clusteredData[clusterValue])
      # populate
      # todo get cutindexes for each cluster set
      if clusteredData[clusterValue]['cutIndexes'][0] == -1 and row['time'] >= cutValues[0]:
        currentIndex = len(clusteredData[clusterValue]['columns']['travelTime'])
        clusteredData[clusterValue]['cutIndexes'][0] = currentIndex
        # print "cluster {0} index of cut value {1} ({4}) is {2}, whereas original cutindex is {3} ".format(clusterValue, cutValues[0], clusteredData[clusterValue]['cutIndexes'][0], cutIndexes[0], row['time'])
      if clusteredData[clusterValue]['cutIndexes'][1] == -1 and row['time'] >= cutValues[1]:
        currentIndex = len(clusteredData[clusterValue]['columns']['travelTime'])
        clusteredData[clusterValue]['cutIndexes'][1] = currentIndex
        # print "cluster {0} index of cut value {1} ({4}) is {2}, whereas original cutindex is {3} ".format(clusterValue, cutValues[1], currentIndex,cutIndexes[1], row['time'])
      for columnname in columnnames:
        clusteredData[clusterValue]['columns'][columnname].append(row[columnname])
      clusteredData[clusterValue]['columns']['price'].append(price) # price > 0 for sacrificed users 
      clusteredData[clusterValue]['columns']['error'].append(error) # actual travel time - system travel time, error > 0 for users who traveled longer than promised
      clusteredData[clusterValue]['columns']['error_sqrt_root'].append(sqrt_error)    
  
  for key,value in clusteredData.iteritems():
    if clusteredData[key]['cutIndexes'][0] == -1:
      clusteredData[key]['cutIndexes'][0] = 0
    if clusteredData[key]['cutIndexes'][1] == -1:
      clusteredData[key]['cutIndexes'][1] = 0
    for columnname in num_columnnames:
      array = np.array(value['columns'][columnname])
      subarray = array[clusteredData[key]['cutIndexes'][0]: clusteredData[key]['cutIndexes'][1]]
      # print key, " subarray of ",columnname, ": ", subarray
      if operator.isNumberType(array[0]):
        value['stats'][columnname] = {}
        value['stats'][columnname]['average'] = np.average(array)
        value['stats'][columnname]['min'] = np.min(array)
        value['stats'][columnname]['max'] = np.max(array)
        value['stats'][columnname]['std'] = np.std(array)
        value['stats'][columnname]['sum'] = sum(array)
        value['stats'][columnname]['count'] = len(array)
      if len(subarray) >  0 and operator.isNumberType(array[0]):
        value['stats'][columnname]['average_cut'] = np.average(subarray)
        value['stats'][columnname]['min_cut'] = np.min(subarray)
        value['stats'][columnname]['max_cut'] = np.max(subarray)
        value['stats'][columnname]['std_cut'] = np.std(subarray)
        value['stats'][columnname]['sum_cut'] = sum(subarray)
        value['stats'][columnname]['count_cut'] = len(subarray)
    for columnname in drivercolumns:
      array = np.array(value['columns'][columnname])
      if operator.isNumberType(array[0]):
        value['stats'][columnname] = {}
        value['stats'][columnname]['average'] = np.average(array)
        value['stats'][columnname]['min'] = np.min(array)
        value['stats'][columnname]['max'] = np.max(array)
        value['stats'][columnname]['std'] = np.std(array)
        value['stats'][columnname]['sum'] = sum(array)
        value['stats'][columnname]['count'] = len(array)
      if len(subarray) >  0 and operator.isNumberType(array[0]):
        value['stats'][columnname]['average_cut'] = np.average(subarray)
        value['stats'][columnname]['min_cut'] = np.min(subarray)
        value['stats'][columnname]['max_cut'] = np.max(subarray)
        value['stats'][columnname]['std_cut'] = np.std(subarray)
        value['stats'][columnname]['sum_cut'] = sum(subarray)
        value['stats'][columnname]['count_cut'] = len(subarray)
  return clusteredData

def processGlobalCostsData(data, names, num_names, sep, sortColumn=""):
  if sortColumn:
    data = np.sort(data, order=sortColumn)
  
  clusteredData = {}
  for row in data:
    time = row[0]
    i = 1
    for name in names:
      elemData = row[i].split(sep)
      clusterValue = elemData[0]
      if not clusterValue in clusteredData.keys():
        clusteredData[clusterValue] = {}
        clusteredData[clusterValue]['columns'] = {}
        clusteredData[clusterValue]['stats'] = {}
        clusteredData[clusterValue]['stats']['average'] = {}
        clusteredData[clusterValue]['stats']['min'] = {}
        clusteredData[clusterValue]['stats']['max'] = {}
        clusteredData[clusterValue]['stats']['std'] = {}
        clusteredData[clusterValue]['stats']['sum'] = {}
        clusteredData[clusterValue]['stats']['count'] = {}
        clusteredData[clusterValue]['columns']['time'] = []
        clusteredData[clusterValue]['columns']['routeId'] = []
        for column_name in num_names:
          clusteredData[clusterValue]['columns'][column_name] = [] 
      
      clusteredData[clusterValue]['columns']['time'].append(time)
      clusteredData[clusterValue]['columns']['routeId'] = clusterValue
      j = 1
      for column_name in num_names:
        clusteredData[clusterValue]['columns'][column_name].append(float(elemData[j]))
        j += 1
      i += 1

  for key,value in clusteredData.iteritems():
    for column_name in num_names:
      array = np.array(value['columns'][column_name])
      if operator.isNumberType(array[0]):
        value['stats']['average'][column_name] = np.average(array)
        value['stats']['min'][column_name] = np.min(array)
        value['stats']['max'][column_name] = np.max(array)
        value['stats']['std'][column_name] = np.std(array)
        value['stats']['sum'][column_name] = sum(array)
        value['stats']['count'][column_name] = len(array)
  return clusteredData


def calculateAvg(inputData, num_names, outputFile):
  out = open(outputFile, 'w')
  sumStats = {}
  for setKey,setValue in inputData.iteritems():
    clusteredData = setValue['clusteredData']
    for key,value in clusteredData.iteritems():
      #initialize
      if not key in sumStats.keys():
        sumStats[key] = {}
        for columnname,stats in value['stats'].iteritems():
          sumStats[key][columnname] = {}
          for statname in stats.keys():
            sumStats[key][columnname][statname] = []
      # populate
      for columnname,stats in value['stats'].iteritems():
        for statname,statvalue in stats.iteritems():
          if statname in sumStats[key][columnname].keys():
            sumStats[key][columnname][statname].append(statvalue)
  for key,value in sumStats.iteritems():
    out.write("cluster\t{0}\n".format(key))
    for columnname,stats in value.iteritems():
      out.write("{0}\n".format(columnname))
      out.write("name\tavg\tmin\tmax\tstd\tmean\tcount\n")
      for statname in stats.keys():
        statarray = np.array(sumStats[key][columnname][statname])
        avg = np.average(statarray)
        min = np.min(statarray)
        mean = np.mean(statarray)
        max = np.max(statarray)
        std = np.std(statarray)
        count = len(statarray)
        out.write("{0}\t{1}\t{2}\t{3}\t{4}\t{5}\t{6}\n".format(statname,avg, min, max, std, mean, count))
        # out.write("{0}\t{1}\n".format(statname,sumStats[key][columnname][statname]))
      # for columnname,statvalues in columnnames.iteritems():
      #   out.write(columnname+"\t")
      #   statarray = []
      #   for value in statvalues:
      #     out.write("{0}\t".format(value))
      #     statarray.append(float(value))
      #   out.write("\n")
      #   statarray = np.array(statarray)
      #   out.write("min\tmean\taverage\tmax\tstd\n")
      #   out.write("{0}\t".format(np.min(statarray)))
      #   out.write("{0}\t".format(np.mean(statarray)))
      #   out.write("{0}\t".format(np.average(statarray)))
      #   out.write("{0}\t".format(np.max(statarray)))
      #   out.write("{0}\n".format(np.std(statarray)))
  

  return

def processElem(elem, args={}):
  startX = float(args["startX"])
  endX = float(args["endX"])
  xLabel = args["xLabel"]
  stepSize = args['stepSize']
  root = args['rootTagName']
  tagName = args['tagName']
  data = args['data']
  # element = {}
  inRange = False
  for name, value in elem.items():
    
    if name == xLabel:
      xValue = float(value)
      if xValue >= startX and xValue <= endX:
        inRange = True
        break
  if inRange:
    for name, value in elem.items():   
      if name not in data:
        data[name] = []
      else:
        data[name].append(value)
  return 0

def removekey(d, key):
    r = dict(d)
    del r[key]
    return r

def fastIter(context, processElement, args={}):
  step = 0
  for event, elem in context:
    args['step'] = step
    doProcess = processElement(elem, args)
    elem.clear()
    while elem.getprevious() is not None:
      del elem.getparent()[0]
    if doProcess==1:
      step += 1
    if doProcess==-1:
      del context
      return
  del context
  return

def readFile(filename, sep1, sep2):
  dataFile = open(filename, 'r')
  rows = dataFile.read().split(sep1)
  dataFile.close()
  columns = []
  for row in rows:
    rowElems = row.split(sep2)
    if len(columns) == 0:
      for rowElem in rowElems:
        columns.append([])
    for i in range(0,len(rowElems)):
      columns[i].append(int(rowElems[i]))
  return columns

def readCVSFile(filename, sep, names = None):
  columns = csv2rec(filename, delimiter=sep, names=names)
  return columns

def plot(plotType, xData, data, xLabel, yLabel, yLabels, outputFile, linesX=None, yMins=None, yMaxs=None, removeZeros = False):
  if len(xData) < 1 or len(data) < 1:
    print "No data to plot"
    return
  # mpl.rcParams['figure.figsize'] = 10, 5
  # mpl.rcParams['font.size'] = 14
  fig, ax = plt.subplots()
  # ax.set_xlabel('time')
  # ax.set_xlim(0, 6000)
  # ax.set_xticks([0,600,1200,1800,2400,3000,3600,4200,4800,5400,6000])
  # ax2 = ax.twiny()
  # ax2.set_xlabel('flow')
  # ax2.set_xlim(100, 2100)
  # ax2.set_xticks([100,300,500,700,900,1100,1300,1500,1700,1900,2100])
  plt.grid(True, which = 'both')
  axis = {}
  i = 0
  yminall = 999999
  ymaxall = 0
  for column in data:
    color = colors[i % len(colors)]
    linestyle = linestyles[i % len(linestyles)]
    style = styles[i % len(styles)]
    if plotType == "line":
      if type(xData[0]) == type([0,1]):
        if len(xData) == 1:
          x = xData[0]
        if len(xData) > 1:
          x = xData[i % len(xData)]
      else :
        x = xData
        # print len(x)
      # check 0 
      # x = xData
      y = column
      if removeZeros:
        xToRemove = {}
        for xIndex in (0,len(column)-1):
          if column[xIndex] == 0:
            xToRemove[xIndex] = 1
        for xIndex in xToRemove.keys():
          x = np.delete(x,xIndex)
        print "X without zeros " , len(x)
        y = y[y != 0] 
      ax.plot(x, y, linestyle=linestyle, color=color, linewidth=2.0, label="{0}".format(yLabels[i]))
      # plot boundaries
      if yMins and yMaxs:
        yMin = yMins[i]
        yMin = yMin[yMin != 0]
        yMax = yMaxs[i]
        yMax = yMax[yMax != 0]
        # print "y without zeros " ,len(y), " ", len(yMin), " ", len(yMax)
        ax.fill_between(x, yMin, yMax, facecolor='yellow', alpha=0.5,label='1 sigma range')

    if plotType == "scatterplot":
      # s=data['steps_count']**2
      xIndex = i % len(xData)
      ax.scatter(xData[xIndex], column, alpha=0.5, c=color, s=10, marker = style, linewidth=0, label="{0}".format(yLabels[i]))  
    ymax = float(max(column)) 
    if ymax > ymaxall:
      ymaxall = ymax
    ymin = float(min(column))
    if ymin < yminall:
      yminall = ymin
    i += 1
    legend = ax.legend(loc='best')
    legend.get_frame().set_alpha(0.5)
  
  if linesX:
    for lineX in linesX:
      l = plt.axvline(x=lineX)

  # p = plt.axhspan(300, 1300, facecolor='0.5', alpha=0.5)
  plt.xlabel(xLabel)
  plt.ylabel(yLabel)
  fig.autofmt_xdate(bottom=0.2, rotation=0, ha='left')
  print "saving figure to {0}".format(outputFile)
  plt.savefig(outputFile) 

  #   
  # #read x column  
  # label = inputData.keys()[0]
  # data = inputData[label]
  # x = data[xColumn]
  # # x = np.split(x, [indexStart, indexStop])
  # x = x[indexStart: indexStop]
  # # read single columns
  # axis = {}
  # for column in plottedColumnsSingle:
  #   axis[legends[column]] = data[column]

  # for column in plottedColumnMultiple:
  #   for label,data in inputData.items():
  #     label = "{0}{1}".format(legends[column], label)
  #     label = re.sub(r'[\[\]]', '', label)
  #     axis[label] = data[column]
  
  # # read multiple columns for each given label (data from different files)
  # axisNum = 0   
  # for label,y in axis.items():
  #   color = colors[axisNum % len(colors)]
  #   if seriesType == "line":
  #     linestyle = linestyles[axisNum % len(linestyles)]
  #     # y = np.split(y, [indexStart, indexStop])
  #     y = y[indexStart: indexStop]

  #     ax.plot(x, y, linestyle=linestyle, color=color, linewidth=2.0, markersize=10, label="{0}".format(label))  
  #     legend = ax.legend(loc='best')
  #     legend.get_frame().set_alpha(0.5)
  #     for label in legend.get_texts():
  #         label.set_fontsize('large')
  #     for label in legend.get_lines():
  #         label.set_linewidth(2) 
  #   if seriesType == "scatterplot":
  #     style = styles[axisNum % len(styles)]
  #     # s=data['steps_count']**2
  #     ax.scatter(x, y, c=color, alpha=0.5, marker = style)
  #   axisNum += 1
  return

def fixTabs(filename):
  file = open(filename, 'r')
  content = file.read()
  file.close()
  content = re.sub(r"[\t]+", "\t", content)
  file = open(filename, 'w')
  file.write(content)
  file.close()
  return

def plot2(x, y, plotname):
  fig = plt.figure()
  rect = fig.patch
  rect.set_facecolor('black')

  ax1 = fig.add_subplot(1,1,1, axisbg='grey') # height, width, chart # 
  # ax1 = fig.add_subplot(2,2,1, axisbg='grey') # height, width, chart # -> 2 rows, 2 square charts on thhe top row
  # ax1 = fig.add_subplot(2,2,2, axisbg='grey') # height, width, chart # -
  # ax1 = fig.add_subplot(2,1,2, axisbg='grey') # height, width, chart # -> 1 rectangle chart on the bottom row
  ax1.plot(x, y, 'c', linewidth=3.3)

  # color of ticks for axes
  # ax1.tick_params(axis='x', color='c') # color -> only ticks, colors -> also text
  ax1.tick_params(axis='x', colors='c')
  ax1.tick_params(axis='y', colors='c')

  ax1.spines['bottom'].set_color('w')
  ax1.spines['top'].set_color('w')
  ax1.spines['left'].set_color('w')
  ax1.spines['right'].set_color('w')

  ax1.xaxis.label.set_color('c')
  ax1.yaxis.label.set_color('c')

  title = 'Title'
  xtitle = 'x'
  ytitle = 'y'

  ax1.set_title(title, color='c')
  ax1.set_xlabel(xtitle)
  ax1.set_ylabel(ytitle)

  # plt.show()
  outputfile = "plot_" + plotname + "_" + xtitle + "-" + ytitle + ".png"
  plt.savefig(outputfile)   

  return


# This is the standard boilerplate that calls the main() function.
if __name__ == '__main__':
  main()



