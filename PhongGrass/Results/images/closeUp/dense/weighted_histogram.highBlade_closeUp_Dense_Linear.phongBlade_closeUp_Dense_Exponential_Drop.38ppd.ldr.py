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

meanValue = 0.145333
maxValue = 0.985966
minValue = 0

weightedMedianValue = 0.264397

firstWeightedQuartileValue = 0.145363

thirdWeightedQuartileValue = 0.435116

dataX = [0.005, 0.015, 0.025, 0.035, 0.045, 0.055, 0.065, 0.075, 0.085, 0.095, 0.105, 0.115, 0.125, 0.135, 0.145, 0.155, 0.165, 0.175, 0.185, 0.195, 0.205, 0.215, 0.225, 0.235, 0.245, 0.255, 0.265, 0.275, 0.285, 0.295, 0.305, 0.315, 0.325, 0.335, 0.345, 0.355, 0.365, 0.375, 0.385, 0.395, 0.405, 0.415, 0.425, 0.435, 0.445, 0.455, 0.465, 0.475, 0.485, 0.495, 0.505, 0.515, 0.525, 0.535, 0.545, 0.555, 0.565, 0.575, 0.585, 0.595, 0.605, 0.615, 0.625, 0.635, 0.645, 0.655, 0.665, 0.675, 0.685, 0.695, 0.705, 0.715, 0.725, 0.735, 0.745, 0.755, 0.765, 0.775, 0.785, 0.795, 0.805, 0.815, 0.825, 0.835, 0.845, 0.855, 0.865, 0.875, 0.885, 0.895, 0.905, 0.915, 0.925, 0.935, 0.945, 0.955, 0.965, 0.975, 0.985, 0.995]

dataFLIP = [155871, 153273, 137096, 120039, 127458, 104913, 83889, 72987, 68039, 64557, 61027, 57791, 54100, 50765, 47676, 44344, 41182, 38608, 35809, 33542, 31233, 28887, 27035, 25151, 23891, 22336, 20620, 19330, 17941, 16809, 16306, 15120, 14674, 13797, 12919, 12267, 11620, 11072, 10441, 9729, 9176, 8619, 8544, 8093, 7483, 6803, 6394, 6117, 5953, 5398, 5122, 4945, 4651, 4437, 4158, 3817, 3641, 3376, 3150, 3074, 2683, 2618, 2443, 2372, 2162, 2123, 2080, 1934, 1894, 1817, 1774, 1536, 1420, 1414, 1324, 1301, 1190, 1129, 1087, 1043, 1057, 942, 806, 739, 656, 653, 545, 520, 420, 419, 375, 401, 350, 289, 256, 390, 1327, 907, 109, 0]

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

plt.savefig("C:/Users/Oliver/source/repos/Phong-Grass/PhongGrass/Results/images/closeUp/dense/weighted_histogram.highBlade_closeUp_Dense_Linear.phongBlade_closeUp_Dense_Exponential_Drop.38ppd.ldr.pdf")
