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
That should print:
  Hello World -or- Hello Alice
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
from operator import itemgetter, attrgetter
from matplotlib.finance import candlestick

linestyles = ['-.', '-', '--', ':']
colors = ('b', 'g', 'r', 'c', 'm', 'y', 'k')
styles = [
  'o',
  'v',
  '^',
  '1',
    r'$\lambda$',
    r'$\bowtie$',
    r'$\circlearrowleft$',
    r'$\clubsuit$',
    r'$\checkmark$']

# Define a main() function that prints a little greeting.
def main():
  # print sys.argv
  parser = OptionParser()
  parser.add_option('--inputFile', help=("inputFile."), type="string", dest="inputFile")
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

  if options.inputFile:
    inputFile = options.inputFile
  else:
    print "No input file"
    return

  # read main xml data
  context = etree.iterparse(inputFile, events=('end',), tag=args['tagName'])
  fastIter(context, processElem, args)
  dataMain = args['data'].copy()
  args['data'] = {}
  plot("line", dataMain['time'], [dataMain['loaded'], dataMain['emitted']], "time", ["main loaded", "main emited"], options.outputDir + "plot_loaded.png", "loaded_emmited")
  plot("line", dataMain['time'], [dataMain['meanTravelTime']], "time", ["main mean travel time"], options.outputDir + "plot_meantraveltime.png", "meantraveltime")
  # read csv dataFile
  sep = '\t'
  names = ['time', 'routeId', 'vehicleId', 'startReroute', 'travelTime', 'startEdgeId', 'endEdgeId', 'vehiclesOnRoute', 'isCheater', 'selfishExpectedTravelTime', 'expectedTravelTime', 'wasCongested', 'delayTime']
  inputFile = "/Users/agatagrzybek/workspace/ovnis/scenarios/Highway/main-test/1000/ovnisOutput/routing_end"
  # formats = ['time', 'routeId', 'vehicleId', 'startReroute', 'travelTime', 'startEdgeId', 'endEdgeId', 'vehiclesOnRoute', 'isCheater', 'selfishExpectedTravelTime', 'expectedTravelTime', 'wasCongested', 'delayTime']
  # dtype = dict(names = names, formats=formats)
  dataMain = readCVSFile(inputFile, sep, names)
  plot("scatterplot", [dataMain['time']], [dataMain['travelTime']], "time", ["main travel time"], options.outputDir + "plot_traveltime.png", "traveltime")
  
  # 
  # # read bypass xml data
  # inputFile = "/Users/agatagrzybek/workspace/ovnis/scenarios/Highway/bypass-test/variable/sumoOutput/summary.xml"
  # context = etree.iterparse(inputFile, events=('end',), tag=args['tagName'])
  # fastIter(context, processElem, args)
  # dataBypass = args['data'].copy()
  # args['data'] = {}

  # plot("line", [dataMain['time'], dataBypass['time']], [dataMain['loaded'], dataMain['emitted'], dataBypass['loaded'], dataBypass['emitted']], "time", ["main loaded", "main emited", "bypass loaded", "bypass emitted"], options.outputDir + "plot_loaded.png", "loaded_emmited")
  # plot("line", [dataMain['time'], dataBypass['time']], [dataMain['running'], dataBypass['running']], "time", ["main running", "bypass running"], options.outputDir + "plot_running.png", "running")
  # plot("line", [dataMain['time'], dataBypass['time']], [dataMain['meanTravelTime'], dataBypass['meanTravelTime']], "time", ["main mean travel time", "bypass mean travel time"], options.outputDir + "plot_meantraveltime.png", "meantraveltime")
  # # plot("scatterplot", [dataMain['time'], dataBypass['time']], [dataMain['loaded'],  dataBypass['loaded'], dataMain['emitted'], dataBypass['emitted']], "time", ["main loaded", "main emited", "bypass loaded", "bypass emitted"], options.outputDir + "plot_loaded.png", "loaded_emmited")
  # # plot("scatterplot", [dataMain['time'], dataBypass['time']], [dataMain['meanTravelTime'], dataBypass['meanTravelTime']], "time", ["main mean travel time", "bypass mean travel time"], options.outputDir + "plot_meantraveltime.png", "meantraveltime")

  # # read csv dataFile
  # sep = '\t'
  # names = ['time', 'routeId', 'vehicleId', 'startReroute', 'travelTime', 'startEdgeId', 'endEdgeId', 'vehiclesOnRoute', 'isCheater', 'selfishExpectedTravelTime', 'expectedTravelTime', 'wasCongested', 'delayTime']
  # inputFile = "/Users/agatagrzybek/workspace/ovnis/scenarios/Highway/main-test/variable/ovnisOutput/routing_end"
  # # formats = ['time', 'routeId', 'vehicleId', 'startReroute', 'travelTime', 'startEdgeId', 'endEdgeId', 'vehiclesOnRoute', 'isCheater', 'selfishExpectedTravelTime', 'expectedTravelTime', 'wasCongested', 'delayTime']
  # # dtype = dict(names = names, formats=formats)
  # dataMain = readCVSFile(inputFile, sep, names)
  # inputFile = "/Users/agatagrzybek/workspace/ovnis/scenarios/Highway/bypass-test/variable/ovnisOutput/routing_end"
  # dataBypass = readCVSFile(inputFile, sep, names)
  # # print "data.dtype.names", data.dtype.names
  # plot("scatterplot", [dataMain['time'], dataBypass['time']], [dataMain['travelTime'], dataBypass['travelTime']], "time", ["main travel time", "bypass travel time"], options.outputDir + "plot_traveltime.png", "traveltime")
  
# --------------------------------------------  

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
    # print elem
    for name, value in elem.items():   
      if name not in data:
        data[name] = []
      else:
        data[name].append(value)
      # element[name]=value
      # print "adding {0}: {1} = elem {2}".format(name, value, element)  
    # data.append(element)
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

def plot(plotType, xData, data, xLabel, yLabels, outputFile, plotname, sortColumn=""):

  if len(xData) < 1 or len(data) < 1:
    print "No data to plot"
    return

  # mpl.rcParams['figure.figsize'] = 10, 5
  # mpl.rcParams['font.size'] = 14
  fig, ax = plt.subplots()
  ax.set_xlabel('time')
  ax.set_xlim(0, 6000)
  ax.set_xticks([0,600,1200,1800,2400,3000,3600,4200,4800,5400,6000])
  # ax.set_ticks_position('top')
  ax2 = ax.twiny()
  ax2.set_xlabel('flow')
  ax2.set_xlim(100, 2100)
  ax2.set_xticks([100,300,500,700,900,1100,1300,1500,1700,1900,2100])
  # ax2.set_ticks_position('bottom')
  # ax2.set_xticklabels(['7','8','99'])
  # ax2.plot(range(1800), np.ones(1800)) # Create a dummy plot
  # ax2.cla()

  plt.grid(True, which = 'both')

  if sortColumn and sortColumn!="":
    for label,data in data.items():
      inputData[label] = numpy.sort(inputData[label], order=sortColumn)
  
  axis = {}
  i = 0
  yminall = 999999
  ymaxall = 0
  for column in data:
    color = colors[i % len(colors)]
    linestyle = linestyles[i % len(linestyles)]
    style = styles[i % len(styles)]
    if plotType == "line":
      if len(xData) > 1 and type(xData[0]) == type([0,1]):
        x = xData[i % len(xData)]
      else:
        x = xData
      print "pltting ", yLabels[i], " line to ", outputFile
      ax.plot(x, column, linestyle=linestyle, color=color, linewidth=2.0, label="{0}".format(yLabels[i]))
    if plotType == "scatterplot":
      print "pltting scatter to ", outputFile
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
  
  # plt.ylim(0, 500)
  # print "yminall ", yminall, ", ymaxall ", ymaxall
  # plt.ylim(yminall, ymaxall)
  plt.ylabel(plotname)
  # plt.xlabel("time")
  fig.autofmt_xdate(bottom=0.2, rotation=0, ha='left')
  print "saving figure to {0}".format(outputFile)
  # outputFile = "plot_" + plotname + ".png"
  plt.savefig(outputFile) 

  #   
  # #read x column  
  # label = inputData.keys()[0]
  # data = inputData[label]
  # x = data[xColumn]
  # print x
  # # x = np.split(x, [indexStart, indexStop])
  # x = x[indexStart: indexStop]
  # print x
  # # read single columns
  # axis = {}
  # # print "Reading single data from any data set, e.g. {0}".format(label)
  # for column in plottedColumnsSingle:
  #   axis[legends[column]] = data[column]

  # for column in plottedColumnMultiple:
  #   for label,data in inputData.items():
  #     # print "label " + label
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

  #     print "printing line {0} {1} {2}".format(label, color, linestyle)
  #     ax.plot(x, y, linestyle=linestyle, color=color, linewidth=2.0, markersize=10, label="{0}".format(label))  
  #     legend = ax.legend(loc='best')
  #     legend.get_frame().set_alpha(0.5)
  #     for label in legend.get_texts():
  #         label.set_fontsize('large')
  #     for label in legend.get_lines():
  #         label.set_linewidth(2) 
  #   if seriesType == "scatterplot":
  #     print "printing scatterplot {0} {1}".format(label, color)
  #     style = styles[axisNum % len(styles)]
  #     # s=data['steps_count']**2
  #     ax.scatter(x, y, c=color, alpha=0.5, marker = style)
  #   axisNum += 1
  
  
  
  # plt.ylabel(ytitle)
  # plt.xlabel(xtitle)
  # fig.autofmt_xdate(bottom=0.2, rotation=0, ha='left')
  # outputfile = options.outputDir + xtitle + "-" + ytitle + ".png"
  # plt.savefig(outputfile) 

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







