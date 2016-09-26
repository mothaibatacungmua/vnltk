#!/usr/bin/env python
# -*- coding: utf-8 -*-

VN_LOWERCASE = [
    'a','ạ','ả','à','ã',
    'â','ậ','ẩ','ầ','ẫ',
    'ă','ặ','ẳ','ằ','ẵ',
    'b','c','d','đ',
    'e','ẹ','ẻ','è','ẽ',
    'ê','ệ','ể','ề','ễ',
    'f','g','h',
    'i','ị','ỉ','ì','ĩ',
    'j','k','l','m','n',
    'o','ọ','ỏ','ò','õ',
    'ô','ộ','ổ','ồ','ỗ',
    'ơ','ợ','ở','ờ','ỡ',
    'p','q','r','s','t',
    'u','ụ','ủ','ù','ũ',
    'ư','ự','ử','ừ','ữ',
    'v','w','x','y','z']

VN_UPPERCASE = [
    'A','Ạ','Ả','À','Ã',
    'Â','Ậ','Ẩ','Ầ','Ã',
    'Ă','Ặ','Ắ','Ằ','Ẵ',
    'B','C','D','Đ',
    'E','Ẹ','Ẻ','È','Ẽ',
    'Ê','Ệ','Ể','Ề','Ễ',
    'F','G','H',
    'I','Ị','Ỉ','Ì','Ĩ',
    'J','K','L','M','N',
    'O','Ọ','Ỏ','Ò','Õ',
    'Ô','Ộ','Ổ','Ồ','Ỗ',
    'Ơ','Ợ','Ớ','Ờ','Ỡ',
    'P','Q','R','S','T',
    'U','Ụ','Ủ','Ù','Ũ',
    'Ư','Ự','Ử','Ừ','Ữ',
    'V','W','X','Y','Z']

SENTENCE_DELIMITER = ['.', '?', '\n']


#input is a unicode character
def vn_islowercase(char):
    return char.encode('utf-8') in VN_LOWERCASE


#input is a unicode character
def vn_isuppercase(char):
    return char.encode('utf-8') in VN_UPPERCASE


#input is a unicode string
def vn_tolowercase(s):
    ls = list(s)
    for c in range(0, len(ls)):
        if vn_isuppercase(ls[c]):
            ic = VN_UPPERCASE.index(ls[c].encode('utf-8'))
            ls[c] = VN_LOWERCASE[ic]

    return u''.join(ls)


#input is a unicode string
def vn_touppercase(s):
    ls = list(s)
    for c in range(0, len(ls)):
        if vn_isuppercase(ls[c]):
            ic = VN_LOWERCASE.index(ls[c].encode('utf-8'))
            ls[c] = VN_UPPERCASE[ic]

    return u''.join(ls)

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

    for s in sentences:
        print s

