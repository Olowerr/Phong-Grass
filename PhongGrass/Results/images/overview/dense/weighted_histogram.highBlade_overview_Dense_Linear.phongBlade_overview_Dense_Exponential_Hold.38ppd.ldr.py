import matplotlib.pyplot as plt
import os
import sys
import numpy as np
from matplotlib.ticker import (MultipleLocator)

dimensions = (25, 15)  #  centimeters

lineColor = 'blue'
fillColor = 'lightblue'
meanLineColor = 'red'
weightedMedianLineColor = 'gray'
quartileLineColor = 'purple'
fontSize = 14
numPixels = 2073600

meanValue = 0.1287
maxValue = 0.974937
minValue = 0

weightedMedianValue = 0.226105

firstWeightedQuartileValue = 0.140231

thirdWeightedQuartileValue = 0.340858

dataX = [0.005, 0.015, 0.025, 0.035, 0.045, 0.055, 0.065, 0.075, 0.085, 0.095, 0.105, 0.115, 0.125, 0.135, 0.145, 0.155, 0.165, 0.175, 0.185, 0.195, 0.205, 0.215, 0.225, 0.235, 0.245, 0.255, 0.265, 0.275, 0.285, 0.295, 0.305, 0.315, 0.325, 0.335, 0.345, 0.355, 0.365, 0.375, 0.385, 0.395, 0.405, 0.415, 0.425, 0.435, 0.445, 0.455, 0.465, 0.475, 0.485, 0.495, 0.505, 0.515, 0.525, 0.535, 0.545, 0.555, 0.565, 0.575, 0.585, 0.595, 0.605, 0.615, 0.625, 0.635, 0.645, 0.655, 0.665, 0.675, 0.685, 0.695, 0.705, 0.715, 0.725, 0.735, 0.745, 0.755, 0.765, 0.775, 0.785, 0.795, 0.805, 0.815, 0.825, 0.835, 0.845, 0.855, 0.865, 0.875, 0.885, 0.895, 0.905, 0.915, 0.925, 0.935, 0.945, 0.955, 0.965, 0.975, 0.985, 0.995]

dataFLIP = [405959, 47296, 68968, 79127, 84005, 83325, 78354, 73619, 70883, 68320, 65547, 63460, 61261, 58266, 54939, 52174, 49063, 45266, 42650, 39682, 36887, 34344, 31653, 29601, 27322, 25579, 23683, 21913, 20263, 18737, 17304, 15991, 14933, 13700, 12521, 11435, 10661, 9864, 9092, 8287, 7584, 7087, 6286, 5777, 5388, 5044, 4599, 4031, 3798, 3364, 3135, 2903, 2664, 2390, 2063, 1913, 1795, 1660, 1486, 1339, 1310, 1058, 1044, 911, 838, 744, 669, 609, 601, 508, 488, 416, 362, 359, 319, 279, 263, 274, 210, 176, 222, 183, 176, 139, 156, 125, 131, 104, 102, 79, 76, 51, 47, 63, 58, 88, 119, 3, 0, 0]

bucketStep = 0.01
weightedDataFLIP = np.empty(100)
moments = np.empty(100)
for i in range(100) :
	weight = (i + 0.5) * bucketStep
	weightedDataFLIP[i] = dataFLIP[i] * weight
weightedDataFLIP /= (numPixels /(1024 * 1024))  # normalized with the number of megapixels in the image

maxY = max(weightedDataFLIP)

sumWeightedDataFLIP = sum(weightedDataFLIP)

font = { 'family' : 'serif', 'style' : 'normal', 'weight' : 'normal', 'size' : fontSize }
lineHeight = fontSize / (dimensions[1] * 15)
plt.rc('font', **font)
fig = plt.figure()
axes = plt.axes()
axes.xaxis.set_minor_locator(MultipleLocator(0.1))
axes.xaxis.set_major_locator(MultipleLocator(0.2))

fig.set_size_inches(dimensions[0] / 2.54, dimensions[1] / 2.54)
axes.set(title = 'Weighted \uA7FBLIP Histogram', xlabel = '\uA7FBLIP error', ylabel = 'Weighted \uA7FBLIP sum per megapixel')

plt.bar(dataX, weightedDataFLIP, width = 0.01, color = fillColor, edgecolor = lineColor)

plt.text(0.99, 1.0 - 1 * lineHeight, 'Mean: ' + str(f'{meanValue:.4f}'), ha = 'right', fontsize = fontSize, transform = axes.transAxes, color=meanLineColor)

plt.text(0.99, 1.0 - 2 * lineHeight, 'Weighted median: ' + str(f'{weightedMedianValue:.4f}'), ha = 'right', fontsize = fontSize, transform = axes.transAxes, color=weightedMedianLineColor)

plt.text(0.99, 1.0 - 3 * lineHeight, '1st weighted quartile: ' + str(f'{firstWeightedQuartileValue:.4f}'), ha = 'right', fontsize = fontSize, transform = axes.transAxes, color=quartileLineColor)

plt.text(0.99, 1.0 - 4 * lineHeight, '3rd weighted quartile: ' + str(f'{thirdWeightedQuartileValue:.4f}'), ha = 'right', fontsize = fontSize, transform = axes.transAxes, color=quartileLineColor)

plt.text(0.99, 1.0 - 5 * lineHeight, 'Min: ' + str(f'{minValue:.4f}'), ha = 'right', fontsize = fontSize, transform = axes.transAxes)
plt.text(0.99, 1.0 - 6 * lineHeight, 'Max: ' + str(f'{maxValue:.4f}'), ha = 'right', fontsize = fontSize, transform = axes.transAxes)
axes.set_xlim(0.0, 1.0)
axes.set_ylim(0.0, maxY * 1.05)
axes.axvline(x = meanValue, color = meanLineColor, linewidth = 1.5)

axes.axvline(x = weightedMedianValue, color = weightedMedianLineColor, linewidth = 1.5)

axes.axvline(x = firstWeightedQuartileValue, color = quartileLineColor, linewidth = 1.5)

axes.axvline(x = thirdWeightedQuartileValue, color = quartileLineColor, linewidth = 1.5)

axes.axvline(x = minValue, color='black', linestyle = ':', linewidth = 1.5)

axes.axvline(x = maxValue, color='black', linestyle = ':', linewidth = 1.5)

plt.savefig("C:/Users/Oliver/source/repos/Phong-Grass/PhongGrass/Results/images/overview/dense/weighted_histogram.highBlade_overview_Dense_Linear.phongBlade_overview_Dense_Exponential_Hold.38ppd.ldr.pdf")
