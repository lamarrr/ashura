from matplotlib import pyplot as pyp
import numpy as np
import sys
from PIL import Image

with open(sys.argv[1]) as file:
    pdata = np.reshape(
        np.array([int(c) for c in file.read().split(",")[:-1]]), (600, 1600, 4)).astype(np.uint8)

pyp.imshow(pdata)
pyp.show()


im = Image.fromarray(pdata)
im.save(sys.argv[1] + '_image.png')
