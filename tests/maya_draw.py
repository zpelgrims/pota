import maya.cmds as cmds
import math
import re

print "PYTHON: Reading data file"
dataFile = open('/Users/zeno/pota/tests/draw.pota', 'r')
 
file_line = dataFile.readline()
file_line = re.sub('[RAYS{}]', '', file_line)
point_list = [float(n) for n in file_line.split()]

dataFile.close()


for count in range(0, (len(point_list)/6)):
    cmds.curve(p=[
        (point_list[count * 6] * 20,
         point_list[count * 6 + 1] * 20,
         point_list[count * 6 + 2]),
         
        ((point_list[count * 6] + (point_list[count * 6 + 3] * 5000)) * 20,
         (point_list[count * 6 + 1] + (point_list[count * 6 + 4] * 5000)) * 20,
         point_list[count * 6 + 2] + (point_list[count * 6 + 5] * 5000))],
         
         d=1)


print "PYTHON: Drawing finished"