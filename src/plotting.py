from matplotlib import pyplot
from mpl_toolkits.mplot3d import Axes3D
import json

with open('/Users/zeno/lentil/pota/point_data.json') as f:
    data = json.load(f)

out_cs = data["out_cs"]
out_ss = data["out_ss"]
del out_ss[::2]
for point in out_ss:
    del point[2:4]

fig = pyplot.figure()
ax = Axes3D(fig)

out_ss_x = []
out_ss_y = []
out_ss_z = []
for i in out_ss:
    out_ss_x.append(i[0])
    out_ss_y.append(i[1])
    out_ss_z.append(0.0)

ax.scatter(out_ss_x, out_ss_y, out_ss_z)
pyplot.show()