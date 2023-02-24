from matplotlib import pyplot
import numpy as np


img = np.array(
[





















]
, dtype = np.uint8)

img = img.reshape((281, 500, 3))

pyplot.imshow(img)
pyplot.show()