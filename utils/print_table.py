import pandas as pd
import os
import os.path
import json
import math


dirname = os.path.dirname(__file__)
print(dirname)
# Load the JSON file
atva = pd.read_json(os.path.join(dirname, '../logs/DOTALearningSMT-results.json'))
atva.columns = atva.columns.str.replace('.json', '', regex=False)

learnta = pd.read_json(os.path.join(dirname, '../logs/LearnTA-results.json'))
#learnta = learnta.append(atva.T['max_constant'])
learnta = pd.concat([learnta, atva.T['max_constant']], ignore_index=False)
learnta.columns = learnta.columns.str.replace('.json', '', regex=False)

# Make a table
full_table_dict = dict()
full_table_dict['LearnTA'] = learnta
full_table_dict['DOCTA'] = atva
full_table = pd.concat(full_table_dict, axis=1)
full_table = full_table.stack(level=0).T.stack(level=1).sort_index(ascending=[True,False])

print(full_table.to_string())
print(full_table.to_html(os.path.join(dirname, '../logs/table.html')))
