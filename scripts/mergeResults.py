import json
import os
import sys
import pandas as pd
import pickle

# load generated temporal file or make a new one if "refresh" is set or it does not exist
# returns pandas object, temporarily cached in temp directory
def load_results(solution_dir, data_dir):
    all_solutions = pd.DataFrame()
    for f in os.listdir(solution_dir):
        filename = os.path.join(solution_dir, f)
        result = pd.read_json(filename, typ='series')
        all_solutions = all_solutions.append(result,ignore_index=True)

    all_info = pd.DataFrame()
    for f in os.listdir(data_dir):
        if f[-5:] != '.json':
            continue
        filename = os.path.join(data_dir, f)
        result = pd.read_json(filename, typ='series')
        all_info = all_info.append(result,ignore_index=True)

    results = pd.merge(all_solutions, all_info, on="id")
    return results
