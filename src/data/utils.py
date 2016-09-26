#!/usr/bin/env python
# -*- coding: utf-8 -*-
import re
lines = None
with open('Viet74K.txt') as f:
    lines = f.readlines()

list_single_words = []
for line in lines:
    sw = line.split()
    for w in sw:
        if w in ['-', '.', '*']:
            continue

        if not w in list_single_words:
            match = re.match(r'.*[\[\(\*",].*', w)
            if match is None:
                if len(w.decode('utf-8')) == 1:
                    print w
                list_single_words.append(w)

f.close()
list_single_words = sorted(list_single_words)
fs = open('VietSingleWords.txt', 'wb')
for w in list_single_words:
    fs.write(w + '\n')

fs.close()