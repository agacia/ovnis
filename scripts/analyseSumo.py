from lxml import etree
import sys
from xml.sax.saxutils import XMLGenerator
from xml.sax.xmlreader import AttributesNSImpl
from optparse import OptionParser	
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.mlab import csv2rec
import re
from pylab import *
import numpy
from numpy import * 
from operator import itemgetter, attrgetter
from matplotlib.finance import candlestick


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

def processElem(elem, args={}):
	startX = float(args["startX"])
	endX = float(args["endX"])
	xLabel = args["xLabel"]
	stepSize = args['stepSize']
	root = args['rootTagName']
	tagName = args['tagName']
	global data
	inRange = False
	for name, value in elem.items():
		
		if name == xLabel:
			xValue = float(value)
			if xValue >= startX and xValue <= endX:
				inRange = True
				break
	if inRange:
		for name, value in elem.items():
			# print "adding {0}: {1}".format(name, value)			
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

def plot(xData, data, sortColumn=""):

	if len(xData) < 1 or len(data) < 1:
		print "No data to plot"
		return

	mpl.rcParams['figure.figsize'] = 10, 5
	mpl.rcParams['font.size'] = 12
	fig, ax = plt.subplots()
	plt.grid(True, which = 'both')

	if sortColumn and sortColumn!="":
		for label,data in data.items():
			inputData[label] = numpy.sort(inputData[label], order=sortColumn)

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
	# 	axis[legends[column]] = data[column]

	# for column in plottedColumnMultiple:
	# 	for label,data in inputData.items():
	# 		# print "label " + label
	# 		label = "{0}{1}".format(legends[column], label)
	# 		label = re.sub(r'[\[\]]', '', label)
	# 		axis[label] = data[column]
	
	# # read multiple columns for each given label (data from different files)
	# axisNum = 0		
	# for label,y in axis.items():
	# 	color = colors[axisNum % len(colors)]
	# 	if seriesType == "line":
	# 		linestyle = linestyles[axisNum % len(linestyles)]
	# 		# y = np.split(y, [indexStart, indexStop])
	# 		y = y[indexStart: indexStop]

	# 		print "printing line {0} {1} {2}".format(label, color, linestyle)
	# 		ax.plot(x, y, linestyle=linestyle, color=color, linewidth=2.0, markersize=10, label="{0}".format(label))	
	# 		legend = ax.legend(loc='best')
	# 		legend.get_frame().set_alpha(0.5)
	# 		for label in legend.get_texts():
	# 		    label.set_fontsize('large')
	# 		for label in legend.get_lines():
	# 		    label.set_linewidth(2) 
	# 	if seriesType == "scatterplot":
	# 		print "printing scatterplot {0} {1}".format(label, color)
	# 		style = styles[axisNum % len(styles)]
	# 		# s=data['steps_count']**2
	# 		ax.scatter(x, y, c=color, alpha=0.5, marker = style)
	# 	axisNum += 1
	
	
	
	# plt.ylabel(ytitle)
	# plt.xlabel(xtitle)
	# fig.autofmt_xdate(bottom=0.2, rotation=0, ha='left')
	# outputfile = options.outputDir + xtitle + "-" + ytitle + ".png"
	# plt.savefig(outputfile)	

	return

######################

args = {} 
args["startX"] = options.startX
args["endX"] = options.endX
args['tagName'] = options.tagName
args['rootTagName'] = options.rootTagName
args['stepSize'] = options.stepSize
args['xLabel'] = 'time'
data = {}

# parser.add_option('--outputFile', help=("outputFile."), type="string", dest="outputFile")
# parser.add_option('--outputDir', help=("outputDir."), type="string", dest="outputDir")

if options.inputFile:
	inputFile = options.inputFile
else:
	print "No input file"
	os.exit()

# read data
context = etree.iterparse(inputFile, events=('end',), tag=args['tagName'])
fastIter(context, processElem, args)
# print data

narray = array(data)
print narray['loaded']
# print narray[0].dtype.name
	