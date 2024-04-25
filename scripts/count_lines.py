#!/bin/python3
import os
import sys
import io

pattern = ['secure_include','secure_lib']
def ignore(path):
    return any(path.find(x) != -1 for x in pattern)

def count_lines_in_file(file_path):
    with io.open(file_path, 'r', encoding='utf-8') as f:
        return sum(1 for line in f)

def count_lines_in_directory(directory):
    total_lines = 0
    dirs = [os.path.join(directory, 'insecure'),
            os.path.join(directory, 'secure')]
    for d in dirs:
        for root, dirs, files in os.walk(d):
            for file in files:
                if file.endswith(('.cpp', '.h')):
                    file_path = os.path.join(root, file)
                    if ignore(file_path): continue
                    print(file_path)
                    total_lines += count_lines_in_file(file_path)
    return total_lines

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python script.py <directory_path>")
        sys.exit(1)

    directory_path = sys.argv[1]
    total_lines = count_lines_in_directory(directory_path)
    print("Total lines in directory:", total_lines)

