#!/usr/bin/python

import os
import sys
import getopt
import numpy as np
import matplotlib
import matplotlib.pyplot as plt
import string
import shutil
import csv
import scipy.stats as st
import math


		
def calculateMeanAndConfInt(list):
	npArray = np.array(list)
	mean = round(np.mean(npArray), 2)
	confInt = st.t.interval(0.95, len(npArray)-1, loc=np.mean(npArray), scale=st.sem(npArray))
	confIntAmplitude = confInt[1] - confInt[0]
	return mean, confIntAmplitude;

def readCsvFromDirectory(path):
	totalNodes = []
	nodesOnCirc = []
	totalCoverage = []
	covOnCirc = []
	hops = []
	messageSent = []
	totalCoveragePercent = []
	covOnCircPercent = []
	for fileName in os.listdir(path):
			with open(os.path.join(path, fileName), "r") as file:
				csvFile = csv.reader(file, delimiter=",")
				firstLine = True
				for row in csvFile:
					if (firstLine):
						firstLine = False
						continue
					totalNodes.append(int(row[5]))
					nodesOnCirc.append(int(row[6]))
					totalCoverage.append(int(row[7]))
					covOnCirc.append(int(row[8]))
					hops.append(float(row[10]))
					#print(hops[-1])
					#if (math.isnan(hops[-1])):
					#	print(file)
					messageSent.append(int(row[12]))
					totalCoveragePercent.append(((float(totalCoverage[-1]) / float(totalNodes[-1])) * 100))	
					
					covOnCircPercent.append(((float(covOnCirc[-1]) / float(nodesOnCirc[-1])) * 100))
					
	totalCovMean , totalCovConfInt = calculateMeanAndConfInt(totalCoveragePercent)
	covOnCircMean, covOnCircConfInt = calculateMeanAndConfInt(covOnCircPercent)
	hopsMean, hopsConfInt = calculateMeanAndConfInt(hops)
	messageSentMean, messageSentConfInt = calculateMeanAndConfInt(messageSent)

	return {"totalCoverageMean": totalCovMean, 
			"totalCovConfInt": totalCovConfInt,
			"covOnCircMean": covOnCircMean,
			"covOnCircConfInt": covOnCircConfInt,
			"hopsMean": hopsMean,
			"hopsConfInt": hopsConfInt,
			"messageSentMean": messageSentMean,
			"messageSentConfInt": messageSentConfInt
	}
