#!/bin/python3

import argparse
from checker import Checker

parser = argparse.ArgumentParser(description='Проверка параметров модуля ядра.')
parser.add_argument('module_name', type=str, help='Имя модуля для проверки')
args = parser.parse_args()

checker = Checker(args.module_name)

print("Проверка my_str:", checker.check_my_str())
print("Проверка ch_val:", checker.check_ch_val())
print("Проверка idx:", checker.check_idx())
