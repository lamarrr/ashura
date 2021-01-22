from matplotlib import pyplot as pyp
import numpy as np

with open("/home/lamar/Documents/workspace/oss/valkyrie/build/compositor.dump") as file:
	pdata = np.reshape(np.array([int(c) for c in file.read().split(",")[:-1]]), (1080, 1920, 4))

pyp.imshow(pdata)
pyp.show()

