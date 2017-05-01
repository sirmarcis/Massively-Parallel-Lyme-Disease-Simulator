from pylab import imshow, get_cmap, savefig
import numpy as np
import sys


numfiles = int(sys.argv[1]) #Number of files (aka ranks)
width = int(sys.argv[2]) #Width of universe
datalocation = sys.argv[3] #folder of data
universe = {}

for i in range(numfiles):
    fname = datalocation + "/result." + str(i)
    emptylist = []
    index = '0';
    with open(fname) as f:
        count = 0
        for line in f:
            line = line.rstrip()
            line = line.strip('\n')
            line = line.split(' ')
            if line[0] != '':
                line = list(map(float, line))
                if index in universe:
                    universe[index].append(line)
                else:
                    universe[index] = []
                    universe[index].append(line)
                count += 1
                if count % (width/numfiles) == 0:
                    temp = index + ' + 1'
                    index = str(eval(temp))

temp = 0
for key, value in universe.items():
    data = np.array(universe[key])
    imshow(data, cmap=get_cmap("binary"), interpolation='nearest')
    savefig(datalocation + '/' + str(temp) + '.png')
    temp += 1
