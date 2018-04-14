
# coding: utf-8

# In[7]:

import numpy as np
import matplotlib.pyplot as plt
import pandas as pd


# In[8]:

output_filename = "obj_pose-laser-radar-output.txt"
input_filename = "obj_pose-laser-radar-synthetic-input.txt"
p_x,p_y,v_x,v_y,px_rmse,py_rmse,vx_rmse,vy_rmse,nis,yaw,yaw_rate,sensor_type = np.loadtxt(output_filename,skiprows = 1,unpack = True)
#data = np.loadtxt(input_filename,dtype = str )


# In[9]:

my_cols=['c1','c2','c3','c4','c5','c6','c7','c8','c9','c10','c11']
with open(input_filename) as f:
    table_input = pd.read_table(f, sep='\t', header=None, names=my_cols, lineterminator='\n')


# In[10]:

table_L = table_input.loc[table_input['c1'] == "L"]
px_input_l = table_L.c2
py_input_l = table_L.c3
table_R = table_input.loc[table_input['c1'] == "R"]
px_input_r = table_R.c2 * np.cos(table_R.c3)
py_input_r = table_R.c2 * np.sin(table_R.c3)
px_gt_l =table_L.c5
py_gt_l =table_L.c6
vx_gt_l =table_L.c7
vy_gt_l =table_L.c8
yaw_gt_l = table_L.c9
yaw_rate_gt_l = table_L.c10



# In[13]:
plt.figure()
plt.plot(p_x,p_y,label = 'Estimate')
plt.plot(px_input_l,py_input_l, label= 'Laser Measurement')
plt.plot(px_input_r,py_input_r, label= 'Radar Measurement')
plt.xlabel('Position in x axis[m]')
plt.ylabel('Position in y axis[m]')
plt.legend(loc =0)
plt.grid()
plt.show()

plt.figure()
plt.plot(p_x,p_y,label = 'Estimate')
plt.plot(px_gt_l,py_gt_l, label= 'Ground Truth')
plt.xlabel('Position in x axis[m]')
plt.ylabel('Position in y axis[m]')
plt.legend(loc =0)
plt.grid()
plt.show()

plt.figure()
plt.plot(v_x,label = 'Estimate')
plt.plot(vx_gt_l, label= 'Ground Truth')
plt.xlabel('Sample No')
plt.ylabel('Velocity along x axis[m/s]')
plt.legend(loc =0)
plt.grid()
plt.show()

plt.figure()
plt.plot(v_y,label = 'Estimate')
plt.plot(vy_gt_l, label= 'Ground Truth')
plt.xlabel('Sample No')
plt.ylabel('Velocity along y axis[m/s]')
plt.legend(loc =0)
plt.grid()
plt.show()

plt.figure()
plt.plot(yaw,label = 'Estimate')
plt.plot(yaw_gt_l, label= 'Ground Truth')
plt.xlabel('Sample No')
plt.ylabel('Yaw [rad]')
plt.legend(loc =0)
plt.grid()
plt.show()

plt.figure()
plt.plot(yaw_rate,label = 'Estimate')
plt.plot(yaw_rate_gt_l, label= 'Ground Truth')
plt.xlabel('Sample No')
plt.ylabel('Yaw Rate [rad/s]')
plt.legend(loc =0)
plt.grid()
plt.show()

# In[7]:
plt.figure()
plt.plot(px_rmse, label = 'RMSE of Px')
plt.plot(py_rmse, label = 'RMSE of Py')
plt.xlabel('Filter iteration')
plt.ylabel('Root Mean Square Error')
plt.legend()
plt.grid()
plt.show()

# In[8]:
plt.figure()
plt.plot(vy_rmse,label = 'RMSE of Vx')
plt.plot(vx_rmse,label = 'RMSE of Vy')
plt.xlabel('Filter iteration')
plt.ylabel('Root Mean Square Error')
plt.legend()
plt.grid()
plt.show()

# in[9]

plt.figure()
nis_0_005 = 7.815*np.ones_like(nis)
plt.plot(nis, label = "NIS value")
plt.plot(nis_0_005, label = "5% NIS line")
plt.xlabel('Filter iteration')
plt.ylabel('NIS Value')
plt.legend()
plt.grid()
plt.show()