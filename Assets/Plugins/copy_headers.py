import argparse
import os
import time
from shutil import copyfile

start_time = time.time()

parser = argparse.ArgumentParser(description='Provide source path for header files.')
parser.add_argument('source', type=str, help='Source directory for headers, e.g. C:/Users/ryand/Dev/Kalide/KalideLibrary/KalideLibrary/Source')

source_path = parser.parse_args().source
source_header_files = []

print("\n.........\n")
print("Looking in " + source_path + "...")
print("\n.........\n")

for root, dirs, files in os.walk(source_path):
    for file in files:
        if file.endswith(".h"):
            print("Found header file {0}".format(file))
            source_header_files.append(os.path.join(root, file))

dir_path = os.getcwd() + "/iOS"
print("\n")

# source
for f in source_header_files:
    relative_path = f.replace(source_path, '')

    file_name = os.path.basename(f)
    if file_name == "pch.h" or file_name == "framework.h":
        continue

    if not os.path.exists(os.path.dirname(dir_path + relative_path)):
        os.mkdir(os.path.dirname(dir_path + relative_path))
    
    print("Copying {0} to {1}".format(f, dir_path + relative_path))
    copyfile(f, dir_path + relative_path)

print("\n.........\n")

# include
include_path = source_path.replace('Source', 'Include')
include_path = include_path.replace('source', 'Include')

if os.path.exists(include_path):

    print("Found Include path {0}".format(include_path))
    print("\n.........\n")

    include_files = []

    for root, dirs, files in os.walk(include_path):
        for file in files:
            if file.endswith(".h"):
                print("Found header file {0}".format(file))
                include_files.append(os.path.join(root, file))

    if not os.path.exists(dir_path + '/Include/'):
        os.mkdir(dir_path + '/Include/')

    print("\n")
    
    for f in include_files:
        relative_path = f.replace(include_path, '')
        relative_path = '/Include/' + relative_path
        
        if not os.path.exists(os.path.dirname(dir_path + relative_path)):
            os.mkdir(os.path.dirname(dir_path + relative_path))

        print("Copying {0} to {1}".format(f, dir_path + relative_path))
        copyfile(f, dir_path + relative_path) 

print("\n.........\n")
print("Finished in {0} seconds.".format(time.time() - start_time))