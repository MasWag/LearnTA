"""Python script to execute the DOTA learning"""
import sys
import time

# The filename is "smart_learner" but the teacher does not return anything smarter than the normal queries.
from DOTALearningSMT.smart_learner import learn_ota
from DOTALearningSMT.ota import buildOTA


def smt_learn_dota(file_name):
    print(f'Start learning with: {file_name}')
    o = buildOTA(file_name)
    start_time = time.perf_counter()
    learned_ota, mem_num, eq_num = learn_ota(o, verbose=False)
    end_time = time.perf_counter()
    print(f'File name: {file_name}')
    print(f'Total number of membership query: {mem_num}')
    print(f'Total number of equivalence query: {eq_num}')
    print(f'Total time of learning: {(end_time - start_time)}')


if __name__ == "__main__":
    if len(sys.argv) == 1:
        print('Usage: python ./run.py [json files]')
    for file_name in sys.argv[1:]:
        smt_learn_dota(file_name)
