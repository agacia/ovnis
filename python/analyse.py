#!/usr/bin/python -tt

"""A Python program to analyse Sumo and OVNIS outpuy.
Try running this program from the command line:
  python analyse.py
"""
import os
import csv
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
  parser.add_option('--inputDir', help=("inputDir."), type="string", dest="inputDir")
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

  if options.inputDir:
    filepattern = "tripstats.txt"
    outfilepath = os.path.join(options.inputDir, "average.txt")
    outf = open(outfilepath, 'w')
    averages = {}
    stats = ["mean", "sum", "std", "min", "max", "count"]
    routes= ["routedist#0", "routedist#1","routedist#2"]
    averages = { name:list() for name in stats}
    routes = { name: { name:list() for name in stats} for name in routes }
    for root, dirs, files in os.walk(options.inputDir):
        for filename in files:
            if filepattern in filename:
                filepath = os.path.join(root, filename)
                print "read %s"%filepath
                with open(filepath) as f:
                    f.readline() # skip
                    data = f.readline().strip().lower().split("\t")
                    averages["mean"].append(float(data[1]))
                    data = f.readline().strip().lower().split("\t")
                    averages["sum"].append(float(data[1]))
                    data = f.readline().strip().lower().split("\t")
                    averages["std"].append(float(data[1]))
                    data = f.readline().strip().lower().split("\t")
                    averages["min"].append(float(data[1]))
                    data = f.readline().strip().lower().split("\t")
                    averages["max"].append(float(data[1]))
                    data = f.readline().strip().lower().split("\t")
                    averages["count"].append(float(data[1]))
                    header_line = f.readline()
                    header_line2 = f.readline()
                    f.readline() # skip empty
                    for line in f:
                        route_data = line.strip().lower().split("\t")
                        route_name = route_data[1]
                        routes[route_name]["count"].append(float(route_data[2]))
                        routes[route_name]["mean"].append(float(route_data[3]))
                        routes[route_name]["sum"].append(float(route_data[4]))
                        routes[route_name]["std"].append(float(route_data[5]))
                        routes[route_name]["min"].append(float(route_data[6]))
                        routes[route_name]["max"].append(float(route_data[7]))

    # total averages
    outf.write("total\n")
    outf.write("statistic\tmean\tstd\tmin\tmax\n")
    for name in stats:
        outf.write("%s\t%.2f\t%.2f\t%.2f\t%.2f\n"
                   %(name, np.mean(averages[name]), np.std(averages[name]),
                     np.amin(averages[name]), np.amax(averages[name])))

    # route averages
    for route,route_stats in routes.items():
        outf.write("%s\n"%route)
        for name in stats:
            outf.write("%s\t%.2f\t%.2f\t%.2f\t%.2f\n"
                   %(name, np.mean(route_stats[name]), np.std(route_stats[name]),
                     np.amin(route_stats[name]), np.amax(route_stats[name])))



  if options.inputFile:
    filename = options.inputFile

    if "communities.csv" in filename:
      df = pd.read_csv(filename, sep="\t")
      title="Average speed"
      plt.figure(1)
      plt.scatter(df['step'], df['timeMeanSpeed'])
      outputfile = options.outputDir + "plot_" + title + ".png"
      plt.savefig(outputfile)

    if "edges_error" in filename:
      new_header = "Time\tVehicle Id\tSize Vanet\tSize Perfect\tError\tPerfect\tVanet\tDiff\tStatic\troute Kennedy\tKennedy\tKennedy Error\tKennedy Perfect\tKennedy Vanet\tKennedy Diff\tKennedy Static\troute Adenauer\tAdenauer\tAdenauer Error\tAdenauer Perfect\tAdenauer Vanet\tAdenauer Diff\tAdenauer Static\troute Thuengen\tThuengen\tThuengen Error\tThuengen Perfect\tThuengen Vanet\tThuengen Diff\tThuengen Static"
      filename, skipped_lines = clean_file(filename, False, new_header)
      df = pd.read_csv(filename, sep="\t")
      xlabel="Time"

      xtitle = 'Time (seconds)'
      ytitle = 'Duration (seconds)'
      title = "Total travel times"
      ylabels=["Perfect","Vanet"]
      ylabel = "Diff"
      plot_lines(df, xlabel, ylabels, title, xtitle, ytitle, options.outputDir)
      write_stats(df, xlabel, ylabel, options.outputDir, skipped_lines)

      title = "Kennedy travel times"
      ylabels=["Kennedy Perfect","Kennedy Vanet"]
      ylabel = "Kennedy Diff"
      plot_lines(df, xlabel, ylabels, title, xtitle, ytitle, options.outputDir)
      write_stats(df, xlabel, ylabel, options.outputDir, skipped_lines)

      title = "Adenauer travel times"
      ylabels=["Adenauer Perfect","Adenauer Vanet"]
      ylabel = "Adenauer Diff"
      plot_lines(df, xlabel, ylabels, title, xtitle, ytitle, options.outputDir)
      write_stats(df, xlabel, ylabel, options.outputDir, skipped_lines)

      title = "Thuengen travel times"
      ylabels=["Thuengen Perfect","Thuengen Vanet"]
      ylabel = "Thuengen Diff"
      plot_lines(df, xlabel, ylabels, title, xtitle, ytitle, options.outputDir)
      write_stats(df, xlabel, ylabel, options.outputDir, skipped_lines)

      title = "Error Travel times"
      ylabels=["Kennedy Diff", "Adenauer Diff", "Thuengen Diff"]
      plot_lines(df, xlabel, ylabels, title, xtitle, ytitle, options.outputDir)

      title = "Perfect Travel times"
      ylabels=["Kennedy Perfect", "Adenauer Perfect", "Thuengen Perfect"]
      plot_lines(df, xlabel, ylabels, title, xtitle, ytitle, options.outputDir)

      title = "Vanet Travel times"
      ylabels=["Kennedy Vanet", "Adenauer Vanet","Thuengen Vanet"]
      plot_lines(df, xlabel, ylabels, title, xtitle, ytitle, options.outputDir)

    if "tripinfo" in filename or "output_log_routing_end" in filename:
      filename, skipped_lines = clean_file(filename, True, None)
      df = pd.read_csv(filename, sep="\t")
      route_names = {"2069.63":"Kennedy", "2598.22": "Adenauer", "2460.76": "Thuengen", "1262.43":"Kennedy", "1791.02":"Adenauer", "1653.56":"Thuengen", "routedist#0":"Kennedy", "routedist#1":"Adenauer", "routedist#2":"Thuengen"}
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
      plot_group_lines(df, groupby_col, xlabel, ylabel, title, xtitle, ytitle, route_names, options.outputDir)
      write_group_stats(df, groupby_col, xlabel, ylabel, route_names, options.outputDir, skipped_lines)

  return

def clean_file(filename, is_header = False, new_header=None):
  clean_filename = filename+'_clean'
  number_of_columns = 0
  skipped_lines = 0
  f_out = open(clean_filename, 'w')
  with open(filename, 'r') as f:
    if is_header:
      header_line = next(f)
    if new_header:
      header_line = new_header
    number_of_columns = len(header_line.split('\t'))
    f_out.write(header_line)
    for data_line in f:
      if len(data_line.strip().split('\t'))!=number_of_columns:
        skipped_lines += 1
        continue
      f_out.write(data_line)
    return clean_filename, skipped_lines

def write_group_stats(df, groupby_col, xlabel, ylabel, route_names, outputdir, skipped_lines):
  filename = os.path.join(outputdir, "tripstats.txt")
  outfile = open(filename, 'w')
  outfile.write("skipped_lines\t%d\n"%skipped_lines)
  meanTT = np.mean(df[ylabel])
  sumTT = np.sum(df[ylabel])
  stdTT = np.std(df[ylabel])
  minTT = np.min(df[ylabel])
  maxTT = np.max(df[ylabel])
  count = len(df[ylabel])
  outfile.write("Mean TT\t%.2f\n"%meanTT)
  outfile.write("Sum TT\t%.2f\n"%sumTT)
  outfile.write("Std TT\t%.2f\n"%stdTT)
  outfile.write("Min TT\t%.2f\n"%minTT)
  outfile.write("Max TT\t%.2f\n"%maxTT)
  outfile.write("Count\t%.2f\n"%count)
  grouped = df.groupby(groupby_col)
  ylabel2 = "staticCost"
  routes = grouped.aggregate({ylabel:[np.size, sum, np.mean, np.std, np.amin, np.amax], ylabel2:[np.mean, np.sum, np.std]}).reset_index()
  print "Writing to file %s"%outfile
  routes.to_csv(outfile, sep='\t')

def write_stats(df, xlabel, ylabel, outputdir, skipped_lines):
  filename = os.path.join(outputdir, "tripstats_error.txt")
  outfile = open(filename, 'w')
  outfile.write("skipped_lines\t%d\n"%skipped_lines)
  meanTT = np.mean(df[ylabel])
  sumTT = np.sum(df[ylabel])
  stdTT = np.std(df[ylabel])
  minTT = np.min(df[ylabel])
  maxTT = np.max(df[ylabel])
  outfile.write("Mean TT\t%.2f\n"%meanTT)
  outfile.write("Sum TT\t%.2f\n"%sumTT)
  outfile.write("Std TT\t%.2f\n"%stdTT)
  outfile.write("Min TT\t%.2f\n"%minTT)
  outfile.write("Max TT\t%.2f\n"%maxTT)
  outfile.close()

def get_group_axes_ranges(grouped, xlabel, ylabel):
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

def get_axes_ranges(df, xlabel, ylabels):
  xmin = 0
  ymin = 0
  xmax = 0
  ymax = 0
  x = max(df[xlabel])
  if x > xmax:
    xmax = x
  for ylabel in ylabels:
    y = max(df[ylabel])
    if y > ymax:
      ymax = y
  return [xmin, xmax, ymin, ymax]

def plot_lines(df, xlabel, ylabels, title, xtitle, ytitle, outputdir):
  colors = ['c','b','r','y','g','m']
  fig = plt.figure()
  num_lines = len(ylabels)
  print "xlabel", xlabel, ", ylabels", ylabels
  print "num of lines:",num_lines
  # find axes range
  [xmin, xmax, ymin, ymax] = get_axes_ranges(df, xlabel, ylabels)
  axes = []
  subplots = 1
  for j in range(subplots):
    axes.append(fig.add_subplot(1, subplots, j+1, axisbg='white')) # height, width, chart #
    axes[j].set_ylim([ymin,ymax])
    axes[j].set_xlim([xmin,xmax])
    axes[j].locator_params(nbins=4)
    for i,ylabel in enumerate(ylabels):
      color = colors[i%len(colors)]
      print i, ylabel, color
      x = df[xlabel]
      y = df[ylabel]
      axes[j].plot(x, y, color, linewidth=1, label=ylabel)
      axes[j].legend(loc='lower center')
      axes[j].set_xlabel(xtitle)

  axes[0].set_ylabel(ytitle)
  outputfile = outputdir + "plot_" + title + "_lines" + ".png"
  plt.savefig(outputfile)

def plot_group_lines(df, groupby_col, xlabel, ylabel, title, xtitle, ytitle, route_names, outputdir):

  colors = ['c','b','r','y','g','m']
  grouped = df.groupby(groupby_col)
  fig = plt.figure()
  num_groups = len(grouped)
  print "num of groups:",num_groups
  # find axes range
  [xmin, xmax, ymin, ymax] = get_group_axes_ranges(grouped, xlabel, ylabel)
  axes = []
  for i,value in enumerate(grouped):
    name,group = value
    color = colors[i%len(colors)]
    print i, name, color
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
  outputfile = outputdir + "plot_groups_" + title + "_lines" + ".png"
  plt.savefig(outputfile)

def plot_scatterplot(df, groupby_col, xlabel, ylabel, title, xtitle, ytitle, route_names, outputdir):
  colors = ['c','b','r','y','g','m']
  grouped = df.groupby(groupby_col)
  fig = plt.figure()
  plt.figure(1)
  print "num of groups:",len(grouped)
  for i,value in enumerate(grouped):
    name,group = value
    if name in route_names:
      color = colors[i%len(colors)]
      print "plotting group\t", i, name, color
      plt.scatter(group[xlabel], group[ylabel], s=20, c=color, marker='.', label=route_names[name], lw = 0)
  [xmin, xmax, ymin, ymax] = get_group_axes_ranges(grouped, xlabel, ylabel)
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



