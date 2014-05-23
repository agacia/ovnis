#!/usr/bin/python -tt

"""A Python program to analyse Sumo and OVNIS outpuy.
Try running this program from the command line:
  python analyse.py
"""
import os
import matplotlib
matplotlib.use('Agg') # headless mode
import matplotlib.pyplot as plt
import numpy as np
import time
from matplotlib.mlab import csv2rec
import math
from math import exp, pow
import pandas as pd
from optparse import OptionParser

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
  parser.add_option('--labels', help=("Label titles"), type="string", dest='labelTitles')
  parser.add_option('--outputFile', help=("outputFile."), type="string", dest="outputFile")
  parser.add_option('--outputDir', help=("outputDir."), type="string", dest="outputDir")
  parser.add_option('--startX', help=("Start time"), type="int", dest='startX')
  parser.add_option('--endX', help=("End time"), type="int", dest='endX')
  parser.add_option('--stepSize', help=("Step size"), type="int", dest='stepSize')
  (options, args) = parser.parse_args()
  print options

  args = {}
  args["startX"] = options.startX
  args["endX"] = options.endX
  args['stepSize'] = options.stepSize
  args['xLabel'] = 'time'

  if options.inputFile:
    filename = options.inputFile
    df = pd.read_csv(filename, sep="\t")

    if "communities.csv" in filename:
      title="Average speed"
      plt.figure(1)
      plt.scatter(df['step'], df['timeMeanSpeed'])
      outputfile = options.outputDir + "plot_" + title + ".png"
      plt.savefig(outputfile)

    if "tripinfo" in filename or "output_log_routing_end" in filename:
      route_names = {"2069.63":"Kennedy", "2598.22": "Adenauer", "2460.76": "Thuengen", "1262.43":"Kennedy", "1791.02":"Adenauer", "1653.56":"Thuengen", "routedist#0":"Kennedy", "routedist#1":"Adenauer", "routedist#2":"Thuengen"}
      print route_names
      # tripinfo
      # arrival waitSteps vType depart  routeLength vaporized duration  arrivalSpeed  devices departPos departDelay departLane  departSpeed arrivalPos  rerouteNo id  arrivalLane
      xlabel="arrival"
      ylabel="duration"
      title = "Trip duration"
      xtitle = 'Time (seconds)'
      ytitle = 'Duration (seconds)'
      if "tripinfo" in filename:
        groupby_col = "routeLength"
        df[groupby_col] = df[groupby_col].map(lambda x: '%.2f' % x)
      else:
        groupby_col = "routeId"
      plot_scatterplot(df, groupby_col, xlabel, ylabel, title, xtitle, ytitle, route_names, options.outputDir)
      plot_lines(df, groupby_col, xlabel, ylabel, title, xtitle, ytitle, route_names, options.outputDir)
      write_stats(df, groupby_col, xlabel, ylabel, route_names, options.outputDir)

  return


def write_stats(df, groupby_col, xlabel, ylabel, route_names, outputdir):
  filename = os.path.join(outputdir, "tripstats.txt")
  outfile = open(filename, 'w')
  grouped = df.groupby(groupby_col)
  ylabel2 = "staticCost"
  routes = grouped.aggregate({ylabel:[np.size, np.mean, np.std, np.amin, np.amax], ylabel2:[np.mean, np.std]}).reset_index()
  routes.to_csv(outfile, sep='\t')

def get_axes_ranges(grouped, xlabel, ylabel):
  xmin = 0
  ymin = 0
  xmax = 0
  ymax = 0
  for name,group in grouped:
    x = max(group[xlabel])
    if x > xmax:
      xmax = x
    y = max(group[ylabel])
    if y > ymax:
      ymax = y
  return [xmin, xmax, ymin, ymax]

def plot_lines(df, groupby_col, xlabel, ylabel, title, xtitle, ytitle, route_names, outputdir):

  colors = ['c','b','r','y','g','m']
  grouped = df.groupby(groupby_col)
  fig = plt.figure()
  num_groups = len(grouped)
  print "num of groups:",num_groups
  # find axes range
  [xmin, xmax, ymin, ymax] = get_axes_ranges(grouped, xlabel, ylabel)
  axes = []
  for i,value in enumerate(grouped):
    name,group = value
    color = colors[i%len(colors)]
    #print i, name, color
    x = group[xlabel]
    y = group[ylabel]
    # if len(axes) > 0:
    #   axes.append(fig.add_subplot(1,num_groups,i+1, sharex=axes[0], sharey=axes[0], axisbg='white', autoscale_on=False)) # height, width, chart #
    # else:
    #   axes.append(fig.add_subplot(1, num_groups, i+1, axisbg='white')) # height, width, chart #
    axes.append(fig.add_subplot(1, num_groups, i+1, axisbg='white')) # height, width, chart #
    axes[i].set_ylim([ymin,ymax])
    axes[i].set_xlim([xmin,xmax])
    axes[i].locator_params(nbins=4)
    axes[i].plot(x, y, color, linewidth=1, label=route_names[name])
    axes[i].legend(loc='lower center')
    axes[i].set_xlabel(xtitle)

  axes[0].set_ylabel(ytitle)
  outputfile = outputdir + "plot_" + title + "_lines" + ".png"
  plt.savefig(outputfile)

def plot_scatterplot(df, groupby_col, xlabel, ylabel, title, xtitle, ytitle, route_names, outputdir):
  colors = ['c','b','r','y','g','m']
  grouped = df.groupby(groupby_col)
  fig = plt.figure()
  plt.figure(1)
  print "num of groups:",len(grouped)
  for i,value in enumerate(grouped):
    name,group = value
    color = colors[i%len(colors)]
    print "plotting group\t", i, name, color
    plt.scatter(group[xlabel], group[ylabel], s=20, c=color, marker='.', label=route_names[name], lw = 0)
  [xmin, xmax, ymin, ymax] = get_axes_ranges(grouped, xlabel, ylabel)
  plt.xlim([xmin,xmax])
  plt.xlabel(xtitle)
  plt.ylabel(ytitle)
  plt.legend(loc='lower right')
  outputfile = outputdir + "plot_" + title + "_scatterplot" + ".png"
  print "saving ", outputfile
  plt.savefig(outputfile)



# This is the standard boilerplate that calls the main() function.
if __name__ == '__main__':
  main()



