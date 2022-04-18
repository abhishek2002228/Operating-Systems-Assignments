import matplotlib.pyplot as plt

filename = input("Enter filename to plot the graph for ")
f = open(filename, "r")

x = [100, 1000, 5000, 100000, 500000, 1000000]
y = []
for val in f:
	if val != " ":
		y = (val.split(" "))
f.close()
plt.plot(x, y)
plt.xlabel("Workload ")
plt.ylabel("Time taken in seconds ")
plt.show()

