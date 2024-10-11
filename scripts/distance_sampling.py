import matplotlib.pyplot as plt
import numpy as np
import math
import random

# global params
val_count = 32
sample_count = 1000
sample_set_size = 10
scale = 0.3

def generate_local_collision_propability():
    global p_collision_local, p_collision_local_max
    density = np.random.rand(val_count)
    f_collision = lambda mu: 1-math.exp(-mu*scale)
    p_collision_local = np.array([f_collision(mu) for mu in density])
    p_collision_local_max = np.max(p_collision_local)
    return p_collision_local

def calculate_exact_global_collision_propability():
    p_collision = []
    current_transmittance = 1
    for p in p_collision_local:
        p_collision += [p*current_transmittance]
        current_transmittance *= 1-p
    return p_collision

def direct_collision_sampling():
    inv_sample_count = 1/sample_count
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
    return s_collision

def indirect_collision_sampling():
    inv_sample_count = 1/sample_count
    rng_mat_mask = np.random.rand(sample_set_size, val_count)
    rng_mat_sampling = np.random.rand(sample_count, val_count)
    f_sample = lambda xi,p: 1 if xi < p else 0

    s_static_mask = np.zeros(shape=(sample_set_size, val_count))
    
    for i in range(sample_set_size):
        s_static_mask[i] = np.array([f_sample(rng_mat_mask[i][j], p_collision_local[j]/p_collision_local_max ) for j in range(val_count)])
    
    # sample global collision distribution
    s_collision = np.zeros(shape=(val_count))
    for i in range(sample_count):
        sample_idx = random.randint(0,sample_set_size-1)

        s_dynamic_mask = np.array([f_sample(rng_mat_sampling[i][j], p_collision_local_max ) for j in range(val_count)])
        s_sample_mask = np.multiply(s_dynamic_mask, s_static_mask[sample_idx])

        for j in range(val_count):
            if s_sample_mask[j] == 1:
                s_collision[j] += inv_sample_count
                break
    return s_collision

def compute_error(sampled, exact):
    f_mse = lambda a,b: (a-b)
    error = np.array([f_mse(sampled[i], exact[i]) for i in range(val_count)])
    return error

# Plot
generate_local_collision_propability()
exact_collision_propability = calculate_exact_global_collision_propability()
sampled_collision_propability_direct = direct_collision_sampling()
error_direct = compute_error(sampled_collision_propability_direct,exact_collision_propability)
mse_direct = np.sum(np.multiply(error_direct,error_direct))

sampled_collision_propability_indirect = indirect_collision_sampling()
error_indirect = compute_error(sampled_collision_propability_indirect,exact_collision_propability)
mse_indirect = np.sum(np.multiply(error_indirect,error_indirect))

inv_val_count = 1/val_count
X = inv_val_count*0.5 + np.arange(0,1,inv_val_count)
figure, axis = plt.subplots(4)
axis[0].bar(X, exact_collision_propability, width=inv_val_count, edgecolor="white", color="green", linewidth=0.7, label='exact')
axis[0].legend()
bar_width= inv_val_count*0.4
bar_center_offset = inv_val_count*0.5
axis[1].bar(X+bar_center_offset-bar_width, sampled_collision_propability_direct, width=bar_width, edgecolor="white", linewidth=0.1, label='value direct')
axis[1].bar(X+bar_center_offset, sampled_collision_propability_indirect, width=bar_width, edgecolor="white", linewidth=0.7, label='value indirect')
axis[1].legend()

axis[2].bar(X+bar_center_offset-bar_width, error_direct, width=bar_width, edgecolor="white", linewidth=0.1, label='error direct')
axis[2].bar(X+bar_center_offset, error_indirect, width=bar_width, edgecolor="white", linewidth=0.1, label='error indirect')
axis[2].legend()

labels = ['mse direct', 'mse indirect']
values = [mse_direct, mse_indirect]
bar_colors = ['blue', 'orange']
axis[3].bar(labels, values, color=bar_colors)
plt.show()