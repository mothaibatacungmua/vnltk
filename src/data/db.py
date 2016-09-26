#!/usr/bin/env python
# -*- coding: utf-8 -*-


import mysql.connector
import hashlib

'''
https://dev.mysql.com/doc/connector-python/en/
'''

config = {
  'user': 'vnltk',
  'password': '123qwe',
  'host': '127.0.0.1',
  'database': 'vncorpus',
  'raise_on_warnings': True,
}

cnx = None
cursor = None
q_add_corpus = ("INSERT INTO `corpus` (id, body) VALUES (%s, %s)")
q_get_corpus = ("SELECT * FROM `corpus` LIMIT %s OFFSET %s")

def create_cursor():
    global cnx, cursor
    if not cursor is None:
        return cursor

    cnx = mysql.connector.connect(**config)
    cursor = cnx.cursor()
    return cursor


def insert_corpus(text):
    global cnx, cursor, add_corpus
    data_corpus = (hashlib.md5(text.encode('utf-8')).hexdigest(), text)
    cursor.execute(q_add_corpus, data_corpus)
    cnx.commit()
    pass


def get_corpus(limit, offset):
    global cnx, cursor, add_corpus
    cursor.execute(q_get_corpus, (limit, offset))
    return cursor.fetchall()

def close_cnx():
    global cnx, cursor
    cursor.close()
    cnx.close()


if __name__ == '__main__':
    cnx = mysql.connector.connect(**config)
    cursor = cnx.cursor()

    data = 'thứ ngày tháng năm'

    data_corpus = (hashlib.md5(data).hexdigest(), data)

    cursor.execute(add_corpus, data_corpus)

    cnx.commit()

    cursor.close()
    cnx.close()