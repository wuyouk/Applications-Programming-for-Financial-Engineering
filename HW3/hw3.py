#!/usr/bin/python
import sys
import pandas as pd

if len(sys.argv) < 3:
    print('Usage: hw3q1.py K N data_file')
    exit(0)

num_assets = int(sys.argv[1])
num_days = int(sys.argv[2])

if num_assets > 1000 or num_assets <= 0 or num_days > 250 or num_days <= 0:
    print('Invalid input')
    exit(0)

file = str(sys.argv[3])

# read in a K * N dataframe, K rows assets, N columns days
data = pd.read_csv(file, header = None, nrows = num_assets, usecols = list(range(0, num_days)))
# transpose to get N * K dataframe, N rows days, K columns assets
data = pd.DataFrame.transpose(data)
print('data:')
print(data)
# compute return
r_data = data.pct_change()
r_data = r_data.dropna()
print('returns data:')
print(r_data)

# compute average return for each asset
r_avg_data = r_data.mean()
r_avg_data = r_avg_data.to_frame()
r_avg_data.columns = ['return']
print('average return data:')
print(r_avg_data)
# compute covariance matrix
print('covariance data:')
r_cov_data = r_data.cov()
print(r_cov_data)

# write to csv
r_avg_data.to_csv('return.csv')
r_cov_data.to_csv('covariance.csv')

# write to example_large.txt
output = open('example_large.txt', "w")
upper = str(max(0.01, round(2.0/num_assets,3)))
output.write("n " + str(num_assets) + "\n\n" + "j_lower_upper_mu\n\n")
for i in range(num_assets):
    output.write(str(i) + ' 0 ' + upper + ' ' + str(r_avg_data.iloc[i,0]) + '\n')

output.write('\nlambda 10\n\ncovariance\n\n')
r_cov_data.to_string(output, index=False, header=False)

output.write('\n\nEND\n')
output.close()