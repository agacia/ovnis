#!/usr/bin/python -tt

"""A Python program to analyse Sumo and OVNIS outpuy.
Try running this program from the command line:
  python analyse.py
"""
import os
# import csv
import matplotlib
matplotlib.use('Agg')  # headless mode
import matplotlib.pyplot as plt
import numpy as np
# import time
# from matplotlib.mlab import csv2rec
# import math
# from math import exp, pow
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
    parser = OptionParser()
    parser.add_option('--inputDir',
                      help=("input dir."), type="string", dest="inputDir")
    parser.add_option('--inputFile',
                      help=("input file."), type="string", dest="inputFile")
    parser.add_option('--inputFiles',
                      help=("input files."), type="string", dest="inputFiles")
    parser.add_option('--labels',
                      help=("Label titles"), type="string", dest='labelTitles')
    parser.add_option('--outputFile',
                      help=("outputFile."), type="string", dest="outputFile")
    parser.add_option('--outputDir',
                      help=("outputDir."), type="string", dest="outputDir")
    parser.add_option('--startX',
                      help=("Start time"), type="int", dest='startX')
    parser.add_option('--endX', help=("End time"), type="int", dest='endX')
    parser.add_option('--stepSize',
                      help=("Step size"), type="int", dest='stepSize')
    parser.add_option('--scenario',
                      help=("Scenario "), type="string", dest='scenario')
    (options, args) = parser.parse_args()

    print options

    args = {}
    args["startX"] = options.startX
    args["endX"] = options.endX
    args['stepSize'] = options.stepSize
    args['xLabel'] = 'time'
    scenario = options.scenario or "Kirchberg"

    if options.inputDir:
        process_average_calculation(options.inputDir, scenario)

    if options.inputFile:
        filename = options.inputFile

        if "communities.csv" in filename:
            process_communities(filename, options.outputDir)

        if "edges_error" in filename:
            process_edges_error(filename, options.outputDir, scenario)

        if "tripinfo" in filename or "output_log_routing_end" in filename:
            process_trips(filename, options.outputDir, scenario)


def read_stats(inputdir, filepattern, statnames, indexes, routenames):
    averages = {name: list() for name in statnames}
    routes = {name: {name: list() for name in statnames}
              for name in routenames}
    for root, dirs, files in os.walk(inputdir):
        for filename in files:
            if filepattern in filename:
                filepath = os.path.join(root, filename)
                with open(filepath) as f:
                    f.readline()  # skip
                    for statname in statnames:
                        data = f.readline().strip().lower().split("\t")
                        averages[statname].append(float(data[1]))
                    f.readline()  # skip header line
                    f.readline()  # skip header line
                    f.readline()  # skip empty
                    for line in f:
                        route_data = line.strip().lower().split("\t")
                        route_name = route_data[1]
                        for statname, index in zip(statnames, indexes):
                            routes[route_name][statname].append(
                                float(route_data[index]))
    return averages, routes


def write_average_stats(filepath, statnames, averages, route_averages):
    outf = open(filepath, 'w')
    outf.write("total\n")
    outf.write("statistic\tmean\tstd\tmin\tmax\n")
    # averages
    for name in statnames:
        outf.write("%s\t%.2f\t%.2f\t%.2f\t%.2f\n" % (
            name, np.mean(averages[name]), np.std(averages[name]),
            np.amin(averages[name]), np.amax(averages[name])))
    # route averages
    for route, route_stats in route_averages.items():
        outf.write("%s\n" % route)
        for name in statnames:
            outf.write("%s\t%.2f\t%.2f\t%.2f\t%.2f\n" % (
                name, np.mean(route_stats[name]), np.std(route_stats[name]),
                np.amin(route_stats[name]), np.amax(route_stats[name])))


def process_average_calculation(inputdir, scenario):
    filepattern = "tripstats.txt"
    statnames = ["mean", "sum", "std", "min", "max", "count"]
    indexes = [2, 3, 4, 5, 6, 7]
    if scenario == "Kirchberg":
        routenames = ["routedist#0", "routedist#1", "routedist#2"]
    else:
        routenames = ["main", "bypass"]
    averages, route_averages = read_stats(
        inputdir, filepattern, statnames, indexes, routenames)
    # total averages
    filepath = os.path.join(inputdir, "average.txt")
    write_average_stats(filepath, statnames, averages, route_averages)


def process_communities(filename, outputdir):
    df = pd.read_csv(filename, sep="\t")
    title = "Average speed"
    plt.figure(1)
    plt.scatter(df['step'], df['timeMeanSpeed'])
    outputfile = os.path.join(outputdir, "plot_" + title + ".png")
    plt.savefig(outputfile)


def clean_file(filename, is_header=False, new_header=None):
    clean_filename = filename + '_clean'
    number_of_columns = 0
    skipped_lines = 0
    fout = open(clean_filename, 'w')
    with open(filename, 'r') as f:
        if is_header:
            header_line = next(f)
        if new_header:
            header_line = new_header
        number_of_columns = len(header_line.split('\t'))
        fout.write(header_line)
        for data_line in f:
            if len(data_line.strip().split('\t')) != number_of_columns:
                print "skipped lines", skipped_lines, "data.length" , len(data_line.strip().split('\t')) , number_of_columns
                skipped_lines += 1
                continue
            fout.write(data_line)
    return clean_filename, skipped_lines


def write_group_stats(df, groupby_col, xlabel, ylabel,
                      route_names, outputdir, skipped_lines):
    filename = os.path.join(outputdir, "tripstats.txt")
    outfile = open(filename, 'w')
    outfile.write("skipped_lines\t%d\n" % skipped_lines)
    meanTT = np.mean(df[ylabel])
    sumTT = np.sum(df[ylabel])
    stdTT = np.std(df[ylabel])
    minTT = np.min(df[ylabel])
    maxTT = np.max(df[ylabel])
    count = len(df[ylabel])
    outfile.write("Mean TT\t%.2f\n" % meanTT)
    outfile.write("Sum TT\t%.2f\n" % sumTT)
    outfile.write("Std TT\t%.2f\n" % stdTT)
    outfile.write("Min TT\t%.2f\n" % minTT)
    outfile.write("Max TT\t%.2f\n" % maxTT)
    outfile.write("Count\t%.2f\n" % count)
    grouped = df.groupby(groupby_col)
    ylabel2 = "staticCost"
    routes = grouped.aggregate(
        {ylabel: [np.size, sum, np.mean, np.std, np.amin, np.amax],
         ylabel2: [np.mean, np.sum, np.std]}).reset_index()
    print "Writing to file %s" % outfile
    routes.to_csv(outfile, sep='\t')


def write_stats(df, xlabel, ylabel, outputdir, skipped_lines):
    filename = os.path.join(outputdir, "tripstats_error.txt")
    outfile = open(filename, 'w')
    outfile.write("skipped_lines\t%d\n" % skipped_lines)
    meanTT = np.mean(df[ylabel])
    sumTT = np.sum(df[ylabel])
    stdTT = np.std(df[ylabel])
    minTT = np.min(df[ylabel])
    maxTT = np.max(df[ylabel])
    outfile.write("Mean TT\t%.2f\n" % meanTT)
    outfile.write("Sum TT\t%.2f\n" % sumTT)
    outfile.write("Std TT\t%.2f\n" % stdTT)
    outfile.write("Min TT\t%.2f\n" % minTT)
    outfile.write("Max TT\t%.2f\n" % maxTT)
    outfile.close()


def get_group_axes_ranges(grouped, xlabel, ylabel):
    xmin = 0
    ymin = 0
    xmax = 0
    ymax = 0
    for name, group in grouped:
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
    colors = ['c', 'b', 'r', 'y', 'g', 'm']
    fig = plt.figure()
    [xmin, xmax, ymin, ymax] = get_axes_ranges(df, xlabel, ylabels)
    axes = []
    subplots = 1
    for j in range(subplots):
        axes.append(fig.add_subplot(
            1, subplots, j+1, axisbg='white'))  # height, width, chart #
        axes[j].set_ylim([ymin, ymax])
        axes[j].set_xlim([xmin, xmax])
        axes[j].locator_params(nbins=4)
        for i, ylabel in enumerate(ylabels):
            color = colors[i % len(colors)]
            print i, ylabel, color
            x = df[xlabel]
            y = df[ylabel]
            axes[j].plot(x, y, color, linewidth=1, label=ylabel)
            axes[j].legend(loc='lower center')
            axes[j].set_xlabel(xtitle)
    axes[0].set_ylabel(ytitle)
    outputfile = outputdir + "plot_" + title + "_lines" + ".png"
    plt.savefig(outputfile)


def process_edges_error(filename, outputdir, scenario):
    if scenario == "Kirchberg":
        new_header = "Time\tVehicle Id\tSize Vanet\tSize Perfect\tError\t" \
            "Perfect\tVanet\tDiff\tStatic\troute Kennedy\tKennedy\tKennedy Error\t" \
            "Kennedy Perfect\tKennedy Vanet\tKennedy Diff\tKennedy Static\t" \
            "route Adenauer\tAdenauer\tAdenauer Error\tAdenauer Perfect\t" \
            "Adenauer Vanet\tAdenauer Diff\tAdenauer Static\troute Thuengen\t" \
            "Thuengen\tThuengen Error\tThuengen Perfect\tThuengen Vanet\t" \
            "Thuengen Diff\tThuengen Static\n"
    else:
        new_header = "Time\tVehicle Id\tSize Vanet\tSize Perfect\tError\t" \
            "Perfect\tVanet\tDiff\tStatic\troute Bypass\tBypass\tBypass Error\t" \
            "Bypass Perfect\tBypass Vanet\tBypass Diff\tBypass Static\t" \
            "route Main\tMain\tMain Error\t" \
            "Main Perfect\tMain Vanet\tMain Diff\tMain Static\n"
    filename, skipped_lines = clean_file(filename, False, new_header)
    xlabel = "Time"
    df = pd.read_csv(filename, sep="\t", index_col=False)
    xtitle = 'Time (seconds)'
    ytitle = 'Duration (seconds)'
    title = "Total travel times"
    ylabels = ["Perfect", "Vanet"]
    ylabel = "Diff"
    #print df["index"]
    df[xlabel] = df[xlabel].convert_objects(convert_numeric=True)
    df[ylabel] = df[ylabel].convert_objects(convert_numeric=True)
    for label in ylabels:
        df[label] = df[label].convert_objects(convert_numeric=True)
    plot_lines(df, xlabel, ylabels, title, xtitle, ytitle, outputdir)
    write_stats(df, xlabel, ylabel, outputdir, skipped_lines)

    if scenario == "Kirchberg":
        ylabels = ["Kennedy Perfect", "Kennedy Vanet"]
        title = "Kennedy travel times"
        ylabel = "Kennedy Diff"
    else:
        title = "Main travel times"
        ylabels = ["Main Perfect", "Main Vanet"]
        ylabel = "Main Diff"
    plot_lines(df, xlabel, ylabels, title, xtitle, ytitle, outputdir)
    write_stats(df, xlabel, ylabel, outputdir, skipped_lines)

    if scenario == "Kirchberg":
        ylabels = ["Adenauer Perfect", "Adenauer Vanet"]
        title = "Adenauer travel times"
        ylabel = "Adenauer Diff"
    else:
        ylabels = ["Bypass Perfect", "Bypass Vanet"]
        title = "Bypass travel times"
        ylabel = "Bypass Diff"
    plot_lines(df, xlabel, ylabels, title, xtitle, ytitle, outputdir)
    write_stats(df, xlabel, ylabel, outputdir, skipped_lines)

    if scenario == "Kirchberg":
        title = "Thuengen travel times"
        ylabels = ["Thuengen Perfect", "Thuengen Vanet"]
        ylabel = "Thuengen Diff"
        plot_lines(df, xlabel, ylabels, title, xtitle, ytitle, outputdir)
        write_stats(df, xlabel, ylabel, outputdir, skipped_lines)

    title = "Error Travel times"
    if scenario == "Kirchberg":
        ylabels = ["Kennedy Diff", "Adenauer Diff", "Thuengen Diff"]
    else:
        ylabels = ["Main Diff", "Bypass Diff"]
    plot_lines(df, xlabel, ylabels, title, xtitle, ytitle, outputdir)

    title = "Perfect Travel times"
    if scenario == "Kirchberg":
        ylabels = ["Kennedy Perfect", "Adenauer Perfect", "Thuengen Perfect"]
    else:
        ylabels = ["Main Perfect", "Bypass Perfect"]
    plot_lines(df, xlabel, ylabels, title, xtitle, ytitle, outputdir)

    title = "Vanet Travel times"
    if scenario == "Kirchberg":
        ylabels = ["Kennedy Vanet", "Adenauer Vanet", "Thuengen Vanet"]
    else:
        ylabels = ["Main Vanet", "Bypass Vanet"]
    plot_lines(df, xlabel, ylabels, title, xtitle, ytitle, outputdir)


def process_trips(filename, outputdir, scenario):
    filename, skipped_lines = clean_file(filename, True, None)
    df = pd.read_csv(filename, sep="\t")
    route_names = {"2069.63": "Kennedy", "2598.22": "Adenauer",
                    "2460.76": "Thuengen", "1262.43": "Kennedy",
                    "1791.02": "Adenauer", "1653.56": "Thuengen",
                    "routedist#0": "Kennedy",
                    "routedist#1": "Adenauer",
                    "routedist#2": "Thuengen",
                    "main": "Main", "bypass": "Bypass"}
    # tripinfo
    # arrival waitSteps vType depart  routeLength vaporized duration  arrivalSpeed  devices departPos departDelay departLane  departSpeed arrivalPos  rerouteNo id  arrivalLane
    xlabel = "arrival"
    ylabel = "duration"
    title = "Trip duration"
    xtitle = 'Time (seconds)'
    ytitle = 'Duration (seconds)'
    if "tripinfo" in filename:
        groupby_col = "routeLength"
        df[groupby_col] = df[groupby_col].map(lambda x: '%.2f' % x)
    else:
        groupby_col = "routeId"
    df[xlabel] = df[xlabel].convert_objects(convert_numeric=True)
    df[ylabel] = df[ylabel].convert_objects(convert_numeric=True)
    plot_scatterplot(df, groupby_col, xlabel, ylabel, title, xtitle,
                     ytitle, route_names, outputdir)
    plot_group_lines_v(df, groupby_col, xlabel, ylabel, title, xtitle,
                       ytitle, route_names, outputdir)
    plot_group_lines_a(df, groupby_col, xlabel, ylabel, title, xtitle,
                       ytitle, route_names, outputdir)
    plot_group_lines_a(df, groupby_col, xlabel, 'travelTime',
                       'Travel time on routes', xtitle, ytitle,
                       route_names, outputdir)
    plot_group_lines(df, groupby_col, xlabel, ylabel, title, xtitle,
                     ytitle, route_names, outputdir)
    write_group_stats(df, groupby_col, xlabel, ylabel, route_names,
                      outputdir, skipped_lines)


def plot_group_lines_v(df, groupby_col, xlabel, ylabel, title, xtitle, ytitle,
                       route_names, outputdir):
    colors = ['c', 'b', 'r', 'y', 'g', 'm']
    grouped = df.groupby(groupby_col)
    fig = plt.figure()
    num_groups = len(grouped)
    [xmin, xmax, ymin, ymax] = get_group_axes_ranges(grouped, xlabel, ylabel)
    axes = []
    for i, value in enumerate(grouped):
        name, group = value
        color = colors[i % len(colors)]
        print i, name, color
        x = group[xlabel]
        y = group[ylabel]
        axes.append(fig.add_subplot(
            num_groups, 1, i+1, axisbg='white'))  # height, width, chart #
        axes[i].set_ylim([ymin, ymax])
        axes[i].set_xlim([xmin, xmax])
        axes[i].locator_params(nbins=4)
        axes[i].plot(x, y, color, linewidth=1, label=route_names[name])
        axes[i].legend(loc='lower center')
        axes[i].set_xlabel(xtitle)
    axes[0].set_ylabel(ytitle)
    outputfile = os.path.join(outputdir,
                              "plot_groups_v_" + title + "_lines" + ".png")
    plt.savefig(outputfile)


def plot_group_lines_a(df, groupby_col, xlabel, ylabel, title, xtitle,
                       ytitle, route_names, outputdir):
    colors = ['c', 'b', 'r', 'y', 'g', 'm']
    grouped = df.groupby(groupby_col)
    fig = plt.figure()
    num_groups = len(grouped)
    [xmin, xmax, ymin, ymax] = get_group_axes_ranges(grouped, xlabel, ylabel)
    axes = []
    axes.append(fig.add_subplot(2, 1, 1, axisbg='white'))
    axes[0].set_ylim([ymin, ymax])
    axes[0].set_xlim([xmin, xmax])
    ylabel2 = 'vehiclesOnRoute'
    [xmin, xmax, ymin, ymax] = get_group_axes_ranges(grouped, xlabel, ylabel2)
    axes.append(fig.add_subplot(2, 1, 2, axisbg='white'))
    axes[1].set_ylim([ymin, ymax])
    axes[1].set_xlim([xmin, xmax])
    # axes[0].locator_params(nbins=4)
    for i, value in enumerate(grouped):
        name, group = value
        color = colors[i % len(colors)]
        print i, name, color
        x = group[xlabel]
        y = group[ylabel]
        y2 = group['vehiclesOnRoute']
        axes[0].plot(x, y, color, linewidth=1, label=route_names[name])
        axes[1].plot(x, y2, color, linewidth=1)
    axes[0].legend(loc='lower center')
    axes[0].set_xlabel(xtitle)
    axes[0].set_ylabel(ytitle)
    axes[1].legend(loc='lower center')
    axes[1].set_xlabel(xtitle)
    axes[1].set_ylabel("Number of vehicles")
    outputfile = outputdir + "plot_groups_a_" + title + "_lines" + ".png"
    plt.savefig(outputfile)


def plot_group_lines(df, groupby_col, xlabel, ylabel, title, xtitle,
                     ytitle, route_names, outputdir):
    colors = ['c', 'b', 'r', 'y', 'g', 'm']
    grouped = df.groupby(groupby_col)
    fig = plt.figure()
    num_groups = len(grouped)
    [xmin, xmax, ymin, ymax] = get_group_axes_ranges(grouped, xlabel, ylabel)
    axes = []
    for i, value in enumerate(grouped):
        name, group = value
        color = colors[i % len(colors)]
        print i, name, color
        x = group[xlabel]
        y = group[ylabel]
        axes.append(fig.add_subplot(
            1, num_groups, i+1, axisbg='white'))  # height, width, chart #
        axes[i].set_ylim([ymin, ymax])
        axes[i].set_xlim([xmin, xmax])
        axes[i].locator_params(nbins=4)
        axes[i].plot(x, y, color, linewidth=1, label=route_names[name])
        axes[i].legend(loc='lower center')
        axes[i].set_xlabel(xtitle)

    axes[0].set_ylabel(ytitle)
    outputfile = outputdir + "plot_groups_" + title + "_lines" + ".png"
    plt.savefig(outputfile)


def plot_scatterplot(df, groupby_col, xlabel, ylabel, title, xtitle,
                     ytitle, route_names, outputdir):
    colors = ['c', 'b', 'r', 'y', 'g', 'm']
    grouped = df.groupby(groupby_col)
    plt.figure(1)
    for i, value in enumerate(grouped):
        name, group = value
        if name in route_names:
            color = colors[i % len(colors)]
            print "plotting group\t", i, name, color, type(group[xlabel])
            plt.scatter(group[xlabel], group[ylabel], s=20, c=color,
                        marker='.', label=route_names[name], lw=0)
    [xmin, xmax, ymin, ymax] = get_group_axes_ranges(grouped, xlabel, ylabel)
    plt.xlim([xmin, xmax])
    plt.xlabel(xtitle)
    plt.ylabel(ytitle)
    plt.legend(loc='lower right')
    outputfile = outputdir + "plot_" + title + "_scatterplot" + ".png"
    plt.savefig(outputfile)


# This is the standard boilerplate that calls the main() function.
if __name__ == '__main__':
    main()
