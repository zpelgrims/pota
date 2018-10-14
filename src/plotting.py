# run: python3.7 plotting.py


from matplotlib import pyplot
from mpl_toolkits.mplot3d import Axes3D
import json

with open('/Users/zeno/lentil/pota/point_data.json') as f:
    data = json.load(f)

pxpy = data["pxpy"]
out = data["out"]
sensor = data["sensor"]
sensor_shifted = data["sensor_shifted"]
aperture = data["aperture"]

max_points = 1000
count = 0

# out_ss = data["out_ss"]
# del out_ss[::2]
# for point in out_ss:
#     del point[2:4]

fig = pyplot.figure()
ax = Axes3D(fig)


pxpy_list = [[],[],[]]
for i in pxpy:
    if count < max_points:
        pxpy_list[0].append(i[0])
        pxpy_list[1].append(i[1])
        pxpy_list[2].append(0.0)
        count += 1
ax.scatter(pxpy_list[0], pxpy_list[1], pxpy_list[2])

count = 0
aperture_list = [[], [], []]
for i in aperture:
    if count < max_points:
        aperture_list[0].append(i[0])
        aperture_list[1].append(i[1])
        aperture_list[2].append(0.0)
        count += 1
ax.scatter(aperture_list[0], aperture_list[1], aperture_list[2])
pyplot.show()