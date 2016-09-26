#!/usr/bin/env python
# -*- coding: utf-8 -*-

import scrapy
from bs4 import BeautifulSoup

class VietNamNet(scrapy.Spider):
    name = 'vietnamnet.vn'
    HOST = 'http://vietnamnet.vn'
    MAX_PAGE = 2
    def start_requests(self):
        urls = [
            'http://vietnamnet.vn/vn/thoi-su/',
            'http://vietnamnet.vn/vn/kinh-doanh/',
            'http://vietnamnet.vn/vn/giai-tri/',
            'http://vietnamnet.vn/vn/the-gioi/',
            'http://vietnamnet.vn/vn/giao-duc/',
            'http://vietnamnet.vn/vn/doi-song/',
            'http://vietnamnet.vn/vn/phap-luat/',
            'http://vietnamnet.vn/vn/the-thao/',
            'http://vietnamnet.vn/vn/cong-nghe/',
            'http://vietnamnet.vn/vn/suc-khoe/',
            'http://vietnamnet.vn/vn/bat-dong-san/',
            'http://vietnamnet.vn/vn/ban-doc/'
        ]

        for url in urls[0:1]:
            for i in range(1, VietNamNet.MAX_PAGE):
                r = url + 'trang' + str(i) + '/index.html'
                yield scrapy.Request(url=r, callback=self.parse_main_category)

    def parse_main_category(self, response):
        list_article = response.css('ul.ListArticle')
        for article in list_article:
            hrefs = article.css('a').xpath('@href').extract()
            for href in hrefs:
                article_url = VietNamNet.HOST + href
                yield scrapy.Request(url=article_url, callback=self.parse_article)

    def parse_article(self, response):
        article = response.xpath('//div[@id="ArticleContent"]').extract_first()
        soup = BeautifulSoup(article, 'html.parser')
        table = soup.find_all('table')
        p = soup.find_all('p')
        p[-1].extract()

        for child in table:
            child.extract()

        div = soup.find_all('div')
        for child in div[1:]:
            child.extract()

        yield {
            'text': soup.get_text()
        }