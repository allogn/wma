import matplotlib.pyplot as plt
import re
import numpy as np

gurobi_ans = open("../temp/gurobi_answers.txt","r")
brute_ans = open("../temp/brute_answers.txt","r")
fcla_ans = open("../temp/fcla_answers.txt","r")
files = open("../temp/file_list.txt","r")

brute_list = []
fcla_list = []
gurobi_list = []
file_list = []
args = []

for line in files:
    try:
        brute = int(brute_ans.readline())
        fcla = int(fcla_ans.readline())
        gurobi = int(gurobi_ans.readline())
        brute_list.append(brute)
        fcla_list.append(fcla)
        gurobi_list.append(gurobi)
        file_list.append(line)
        args.append(int(re.match(r'.*_[0-9]+_([0-9]+).gr', line).group(1)))
    except ValueError:
        pass

args_ind = np.argsort(np.array(args))
args = np.array(args)[args_ind]
brute_list = np.array(brute_list)[args_ind]
gurobi_list = np.array(gurobi_list)[args_ind]
fcla_list = np.array(fcla_list)[args_ind]

plt.plot(args,brute_list,'r-o',label="brute")
plt.plot(args,fcla_list,'b-.',label="wma")
plt.plot(args,gurobi_list,'g--+',label="gurobi")
plt.xlabel("Number of Customers")
plt.ylabel("Objective")
plt.legend()
plt.savefig("../temp/pic.png")