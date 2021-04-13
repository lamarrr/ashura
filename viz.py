from matplotlib import pyplot as pyp
import numpy as np
import sys


with open(sys.argv[1]) as file:
	pdata = np.reshape(np.array([int(c) for c in file.read().split(",")[:-1]]), (1080, 1080, 4))

pyp.imshow(pdata)
pyp.show()

