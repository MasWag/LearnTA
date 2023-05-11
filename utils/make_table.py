import pandas as pd
import json
import math

# Load the JSON file
learnta = pd.read_json('LearnTA-results.json')
learnta = learnta.append(atva.T['max_constant'])
learnta.columns = learnta.columns.str.replace('.json', '', regex=False)

atva = pd.read_json('DOTALearningSMT-results.json')
atva.columns = atva.columns.str.replace('.json', '', regex=False)

# Make a table
full_table_dict = dict()
full_table_dict['LearnTA'] = learnta
full_table_dict['DOCTA'] = atva
full_table = pd.concat(full_table_dict, axis=1)
full_table = full_table.stack(level=0).T.stack(level=1).sort_index(ascending=[True,False])

print(full_table.to_string())
