#!/usr/bin/env python
# -*- coding: utf-8 -*-
import re
import codecs

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

SENTENCE_DELIMITER = [u'.', u'?', u'\n']


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
            ls[c] = VN_LOWERCASE[ic].decode('utf-8')

    return u''.join(ls)


#input is a unicode string
def vn_touppercase(s):
    ls = list(s)
    for c in range(0, len(ls)):
        if vn_isuppercase(ls[c]):
            ic = VN_LOWERCASE.index(ls[c].encode('utf-8'))
            ls[c] = VN_UPPERCASE[ic].decode('utf-8')

    return u''.join(ls)


if __name__ == '__main__':
    lines = None
    with codecs.open('vn-words.txt', 'r', 'utf-8') as f:
        lines = f.readlines()

    list_single_words = []
    for line in lines:
        sw = line.strip().split(u' ')
        for w in sw:
            if w in [u'-', u'.', u'*']:
                continue

            lw = vn_tolowercase(w)
            while len(lw.encode('utf-8')) > 1 and lw[-1] in [u'-', u';', u',',u'!',u')']:
                lw = lw[0:-1]

            if not lw in list_single_words:
                match = re.match(ur'.*[\[\(\*",<>].*', lw)
                if match is None:
                    #if len(lw.decode('utf-8')) == 1:
                        #print w
                    list_single_words.append(lw)

    f.close()
    list_single_words = sorted(list_single_words)
    fs = open('vn-single-words.txt', 'wb')
    for w in list_single_words:
        fs.write((w+u'\n').encode('utf-8'))

    fs.close()