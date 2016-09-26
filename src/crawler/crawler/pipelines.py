#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Define your item pipelines here
#
# Don't forget to add your pipeline to the ITEM_PIPELINES setting
# See: http://doc.scrapy.org/en/latest/topics/item-pipeline.html

from db import create_cursor,close_cnx,insert_corpus

class CrawlerPipeline(object):
    def open_spider(self, spider):
        create_cursor()

    def close_spider(self, spider):
        close_cnx()

    def process_item(self, item, spider):
        insert_corpus(item['text'])
        return 'Done!'
