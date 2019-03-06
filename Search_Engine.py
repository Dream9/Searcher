import requests
import logging
import pymysql
import jieba
import math
import random
import time
from bs4 import BeautifulSoup
from sources import _EXITFAIL
from sources import _EXITSUCCESS
from sources import _MAX_PAGE
from sources import headers
# 设置信息
logging.basicConfig(level=logging.INFO, format='%(lineno)d-%(levelname)s:%(message)s')
logging.getLogger().setLevel(logging.INFO)

'''
连接SQL
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

'''
筛选虚词
'''
def pickup(wodlist):
    # 待扩展
    pass

'''

'''
def score(wordlist,cursor,N):
    TF={}
    SCORE={}
    termSelectSQL="select list from "+termname+" where term=%s"
    for word in wordlist:
        # 查询倒排表
        cursor.execute(termSelectSQL,(word))
        docs=cursor.fetchall()[0][0]
        if docs is None:
            continue
        doclist=docs.split(" ")
        D=0#记录出现在了多少个不同的文档之中
        for docid in doclist:
            if docid in TF:
                # 重复出现过，记录index文档中这个词的词频
                TF[docid]+=1
            else:
                TF[docid]=1
                D+=1
        '''
        计算IDF，统计文档得分（逐个term累和）
        '''
        IDF=math.log(N/D,2)
        for docid in TF:
            if docid in SCORE:
                # 已经计算过这篇文档的其他词的得分，更新之
                SCORE[docid]+=TF[docid]*IDF
            else:
                # 第一次计算这篇文档的得分
                SCORE[docid]=TF[docid]*IDF
    '''
    sort一下得分
    '''
    SCORE=sorted(SCORE.items(),key=lambda x:x[1],reverse=True)#默认是升序
    print("网页排名完成")
    return SCORE

'''
参数的初始化
'''
def init(uname, passwd, hos, db,doctable,termtable):
    global username,password, host, database,docname,termname
    username=uname
    password=passwd
    host=hos
    database=db
    termname=termtable
    docname=doctable
'''
在已经建立倒排表的情况下进行搜索
   流程：计算词频，计算log(D/N),计算得分，显示结果
'''
def search_main():
    starttime=time.time()
    connect=connectSql(username,password, host, database)
    docSearchSql="select count(*) from "+docname
    docSearchidSql="select link from "+docname+" where id=%s"
    with connect.cursor() as cur:
        # 计算所有网页数目
        cur.execute(docSearchSql)
        N=cur.fetchall()[0][0]+1

        # 解析查询语句
        word=input("伪典·Search:(what are you looking for?)--->")
        analysis=jieba.cut_for_search(word)#得到每一个词
        #这里应该有一个筛选，或者重新定权重
        pickup(analysis)

        ans=score(analysis,cur,N)

        if len(ans)==0:
            print("Nothing!!")

        '''
        显示搜索结果，显示部分页面
        '''
        numPage=0
        for docid,docscore in ans:
            if numPage>=_MAX_PAGE:
                break
            cur.execute(docSearchidSql,(docid))
            url=cur.fetchall()[0][0]
            print(numPage," : ",url," :%f" %(docscore))

            try:
                response=requests.get(url,headers[random.randint(0,len(headers)-1)])
            except Exception as e:
                logging.info(e)
                print("Web page cannot be displayed\r\n")
                continue
            status=response.status_code
            if status >= 300 or status < 200:
                print("Connection is refused\r\n")
                continue
            # 显示网页的部分信息
            # 由于这里并没有记录term在文档中出现的位置，因此这能显示这个文档重要的部分
            content=response.content.decode("utf-8")
            contentSoup = BeautifulSoup(content, 'lxml')

            descriptions = contentSoup.find('meta', attrs={'name': "description", "content": True})
            shortdescriptions = contentSoup.find('meta', attrs={'property': "og:description", "content": True})
            if descriptions is not None:
                print(descriptions["content"].strip())
            if shortdescriptions is not None:
                print(shortdescriptions["content"].strip())
            print("\r\n")
            numPage +=1
    connect.close()
    endtime=time.time()
    print("Time use:",endtime-starttime)
    # exit(_EXITSUCCESS)



if __name__=="__main__":
    '''
    测试用例
    '''
    init("root","vvo123456","localhost","pytest","search_test_doc","search_test_term")
    # 搜索循环
    while(True):
        search_main()
        ACK=input("Continue? y/n")
        if(ACK != "y"):
            exit(_EXITSUCCESS)
