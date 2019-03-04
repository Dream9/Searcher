import sys
import jieba
import pdb
import random
import time
from bs4 import BeautifulSoup
import pymysql
import getopt
from queue import Queue
import requests
import logging

# 设置信息
logging.basicConfig(level=logging.INFO, format='%(lineno)d-%(levelname)s:%(message)s')
logging.getLogger().setLevel(logging.INFO)

headers = (
    {'User-Agent': 'Mozilla/5.0 (Windows; U; Windows NT 6.1; en-US; rv:1.9.1.6) Gecko/20091201 Firefox/3.5.6'},
    {
        'User-Agent': 'Mozilla/5.0 (Windows NT 6.2) AppleWebKit/535.11 (KHTML, like Gecko) Chrome/17.0.963.12 Safari/535.11'},
    {'User-Agent': 'Mozilla/5.0 (compatible; MSIE 10.0; Windows NT 6.2; Trident/6.0)'}
)

num_docs = 1000 #网页数目
depth=5 #深度

#两个常量
_EXITFAIL = -1
_EXITSUCCESS = 0

# 与MySQL有关的几个参数
username = ""
password = ""
host = "localhost"
database = ""
tablename = "search_test"

# 起始url
'''
特别说明一下，url的第一位用于记录当前深度（未超过10）
'''
url = "1https://www.bookbao99.net/"

'''
返回一个Mysql的连接
'''


def connectSql(username, password, host, database):
    config = {
        'host': host,
        'user': username,
        'passwd': password,
        'db': database
    }
    try:
        con = pymysql.connect(**config)
    except Exception as e:
        logging.warning(e)
        logging.info("Connection failed")
        exit(_EXITFAIL)
    return con


if __name__ == '__main__':
    opts, args = getopt.getopt(sys.argv[1:], "hn:qu:p:d:", ["help", "username=", "password=", "host=","quit","database="])
    # 读取参数
    for op, value in opts:
        if op == "-h":
            print("""Simple Search Engine/r/n 
                  -h:help/r/n" /
                  -n:numbers of docs/r/n
                  -q:quit""")
        elif op == "-n":
            num_docs = value
        elif op == "-u" or op=="--username":
            username = value
        elif op == "-p" or op=="--password":
            password = value
        elif op == "-d" or op=="--database":
            database = value
        elif op=="--host":
            host=value
        elif op == "q" or op=="quit":
            exit(_EXITSUCCESS)
        else:
            logging.warning("Bad parameters,more information for -h")
            exit(_EXITFAIL)
    # pdb.set_trace()
    print("debug")
    ACK = input('Are you sure that %s words database will be built ?(y/n)' % (num_docs))
    if ACK != 'y':
        sys.exit('Shutdown!')
    docname = tablename + "_doc"
    termname = tablename + "_term"

    # 连接MySql,初始化存储表格
    con = connectSql(username, password, host, database)

    with con.cursor() as cur:
        cur.execute("drop table if exists %s" % (docname))
        cur.execute("create table %s (id int primary key," /
                    "link text not NULL" %(docname))
        cur.execute("drop table if exists %s" % (termname))
        cur.execute("create table %s (term varchar(30) primary key," /
                    "list text not NULL" %(termname))
    con.commit()
    con.close()

    # 待获取网页以及已访问网络
    linkQueue = Queue()
    linkVisited = set()
    # 应该有一个合适的调度系统

    linkQueue.put(url)  # 初始起点
    logging.info(time.strftime("start time: %H:%M:%S"), time.time())
    linkCount = 0
    # 爬取数据
    while not linkQueue.empty():
        if linkCount>=num_docs:
            break
        url = linkQueue.get()
        if int(url[0])>depth or url[1:] in linkVisited:
            continue
        linkVisited.add(url[1:])

        try:
            responce = requests.get(url[1:], headers[random.randint(0,len(headers))])
        except Exception as e:
            logging.warning(e)
            continue

        status_code=responce.status_code
        if status_code>=300 or status_code<200:
            logging.info("connection is refused")
            continue
        content=responce.content.decode('utf-8')

        contentSoup=BeautifulSoup(content,'lxml')

        # 找到下一层的页面
        # 实例   <a href="/BookList-c_8-t_0-o_0.html">都市</a>
        hrefs=contentSoup.find_all(name='a')
        for href in hrefs:
            if str(href["href"])[-4:]=="html":
                tmplink=str(href["href"])
                # 补全相对网址
                if tmplink[0]=='/':
                    tmplink=url+tmplink
                # 判断是否访问过
                if tmplink not in linkVisited:
                    linkQueue.put(str(int(url[0])+1)+tmplink)

        '''解析内容
        示例
            <meta name="keywords" content="仙妖同途：仙君有个小妖精" />
            <meta name="description" content="仙妖同途：仙君有个小妖精全文阅读,仙妖同途：仙君有个小妖精TXT下载,仙妖同途：仙君有个小妖精全集,仙妖同途：仙君有个小妖精最新章节" />

        '''

        keywords = contentSoup.find('meta', name="keywords").get_text("", strip=True)
        descriptions = contentSoup.find('meta', name="description").get_text("", strip=True)
        shortdescriptions=contentSoup('meta',property = "og:description").get_text("", strip=True)

        termlist=[]
        if keywords is not None:
            analysis=jieba.cut_for_search(keywords)
            termlist.append(list(analysis))

        if descriptions is not None:
            analysis = jieba.cut_for_search(descriptions)
            termlist.append(list(analysis))

        if shortdescriptions is not None:
            analysis = jieba.cut_for_search(shortdescriptions)
            termlist.append(list(analysis))
        if len(termlist)==0:
            continue
        linkCount=linkCount+1

        '''
        数据存储
        '''
        connect=connectSql(username,password,host,database)
        with connect.cursor() as cur:
            try:
                '''
                文档记录
                '''
                cur.execute("insert into "+docname+" values( %d ,%d)" ,(linkCount,url[1:]))
                '''
                词典记录
                '''
                for term in termlist:
                    cur.execute("select list from "+termname+" where term=%s" ,(term))
                    ans=cur.fetchall()
                    if(len(ans)==0):
                        cur.execute("insert into "+termname+" values(%s,%s)" ,(term,str(linkCount)))
                    else:
                        new_list=ans[0][0]+" "+str(linkCount)
                        cur.execute("update "+termname+" set list=%s where term=%s)" ,(new_list,term) )
            except Exception as e:
                logging.critical(e)
                cur.execute("rollback")
        connect.commit()
        connect.close()
    logging.info("Finished !! "+time.strftime("%H:%M:%S",time.time()))