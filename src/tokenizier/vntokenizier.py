#!/usr/bin/env python
# -*- coding: utf-8 -*-

from src.data.utils import vn_isuppercase
import codecs

def normalize(paragraph):
    return u' '.join(paragraph.split())

def sentence_segment(corpus):
    paragraphs = corpus.split(u'\n')

    sentences = []
    for p in paragraphs:
        norm_p = normalize(p)
        dot_pos = []
        ques_mark_pos = []
        colon_pos = []
        for i in range(0, len(norm_p)):
            if norm_p[i] == u'.':
                dot_pos.append(i)
            if norm_p[i] == u'?':
                ques_mark_pos.append(i)
            if norm_p[i] == u':':
                colon_pos.append(i)

        dot_sen_pos = []
        for pos in dot_pos:
            #find surrounding dots
            if pos == 0 or pos == len(norm_p) - 1:
                dot_sen_pos.append(pos)
                continue

            back_dot = pos-1
            front_dot = pos+1

            if norm_p[back_dot].isdigit() and norm_p[front_dot].isdigit():
                continue

            if norm_p[front_dot] == u' ' and vn_isuppercase(norm_p[back_dot]):
                continue

            while norm_p[back_dot] != u'.' and back_dot > 0: back_dot -= 1
            while norm_p[front_dot] != u'.' and front_dot < len(norm_p)-1: front_dot += 1

            if (pos - back_dot) > 4 or  (front_dot - pos) > 4:
                dot_sen_pos.append(pos)

        seg_pos = dot_sen_pos + ques_mark_pos + colon_pos
        if not 0 in seg_pos: seg_pos.append(0)
        if not (len(norm_p) - 1) in seg_pos: seg_pos.append(len(norm_p) - 1)

        seg_pos = sorted(seg_pos)
        for i in range(0, len(seg_pos)-1):
            if i == 0:
                sentences.append(norm_p[seg_pos[i]:seg_pos[i + 1]].strip())
            else:
                sentences.append(norm_p[(seg_pos[i]+1):seg_pos[i+1]].strip())

    return sentences


def tokenize(corpus):
    sentences = sentence_segment(corpus)
    tokens = []
    sps = []

    for s in sentences:
        ts = s.split()
        spt = []
        for tt in ts:
            fs = []
            while tt[0] == u'(' or tt[0] == u',':
                fs.append(tt[0])
                if not tt[0] in tokens:
                    tokens.append(tt[0])
                tt = u''.join(list(tt[1:]))

            bs = []
            while tt[-1] == u')' or tt[-1] == u',' or tt[-1] == u';':
                bs.insert(0,tt[-1])
                if not tt[-1] in tokens:
                    tokens.append(tt[-1])
                tt = u''.join(list(tt[:-1]))

            spt = spt + fs
            spt.append(tt)
            spt = spt + bs

            if not tt in tokens:
                tokens.append(tt)

        sps.append(u' '.join(spt))

    return tokens, sps


if __name__ == '__main__':
    from src.data.db import create_cursor, get_corpus
    create_cursor()
    list_corpus = get_corpus(1,1)

    # corpus is a unicode document
    corpus =  list_corpus[0][1]
    #print corpus
    tokens , sentences = tokenize(corpus)

    fs = codecs.open('../data/vn-sentences.txt', 'wb', 'utf-8')

    for s in sentences:
        for c in s:
            print hex(ord(c))

        fs.write(s+'\n')


    fs.close()


