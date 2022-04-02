import matplotlib.pyplot as plt
import numpy as np

filename = input("Enter filename to plot the graph for ")
f = open(filename, "r")

x = [2**x for x in range(5)]
y = []
for val in f:
	if val != " ":
		y.append(float(val.strip()))
f.close()
plt.plot(x, y)
plt.xlabel("No of threads ")
plt.ylabel("Time taken in seconds ")
plt.show()

