# importing libraries
import matplotlib.pyplot as plt
import numpy as np
import math
import random

# compute local collision propability
val_count = 32
inv_val_count = 1/val_count

X = inv_val_count*0.5 + np.arange(0,1,inv_val_count)

density = np.random.rand(val_count)
scale = 0.1
f_collision = lambda mu: 1-math.exp(-mu*scale)
p_collision_local = np.array([f_collision(mu) for mu in density])


# compute exact global collision distribution
p_collision = []
current_transmittance = 1
for p in p_collision_local:
    p_collision += [p*current_transmittance]
    current_transmittance *= 1-p


# Collision sampling v1
'''
sample_count = 10
inv_sample_count = 1/sample_count
s_collision = np.zeros(shape=(val_count))

for i in range(sample_count):
    rng = np.random.rand(val_count)

    f_sample = lambda xi,p: 1 if xi < p else 0
    s_collision_local = np.array([f_sample(rng[j], p_collision_local[j]) for j in range(val_count)])
    
    for j in range(val_count):
        if s_collision_local[j] == 1:
            s_collision[j] += inv_sample_count
            break
'''

# Collision sampling v2
sample_count = 1000
inv_sample_count = 1/sample_count
sample_set_size = 1000

rng_mat = np.random.rand(sample_set_size, val_count)
s_collision_local = np.zeros(shape=(sample_set_size, val_count))
f_sample = lambda xi,p: 1 if xi < p else 0

# sample local collision propability
for i in range(sample_set_size):
    s_collision_local[i] = np.array([f_sample(rng_mat[i][j], p_collision_local[j]) for j in range(val_count)])


# sample global collision distribution
s_collision = np.zeros(shape=(val_count))
for i in range(sample_count):
    sample_idx = random.randint(0,sample_set_size-1)
    for j in range(val_count):
        if s_collision_local[sample_idx][j] == 1:
            s_collision[j] += inv_sample_count
            break





# Initialise the subplot function using number of rows and columns
figure, axis = plt.subplots(2)
#axis[0].bar(X, density, width=inv_val_count, edgecolor="white", linewidth=0.7)
#axis[0].set_title("Density")
#
#axis[1].bar(X, p_collision_local, width=inv_val_count, edgecolor="white", linewidth=0.7)
#axis[1].set_title("Local Collision Prob")

axis[0].bar(X, p_collision, width=inv_val_count, edgecolor="white", linewidth=0.7)
axis[0].set_title("Collision Prob")

axis[1].bar(X, s_collision, width=inv_val_count, edgecolor="white", linewidth=0.7)
axis[1].set_title("Sampled Collision Prob")
plt.show()