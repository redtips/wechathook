from pymysql import connect
import xml.sax
import xml.sax.handler
import pprint


class XMLHandler(xml.sax.handler.ContentHandler):
    def __init__(self):
        self.buffer = ""
        self.mapping = {}

    def startElement(self, name, attributes):
        self.buffer = ""

    def characters(self, data):
        self.buffer += data

    def endElement(self, name):
        self.mapping[name] = self.buffer

    def getDict(self):
        return self.mapping


def save(s):
    data = s
    xh = XMLHandler()
    xml.sax.parseString(s, xh)
    ret = xh.getDict()
    open('C://Users//EDZ//Documents//WeChatRobot//源码//WeChatHelper//Debug//ret.txt','a').write(s)
    conn = connect(host='127.0.0.1', port=3306, database='demo',user='root',password='123456')
    cursor = conn.cursor()
    try:
        sql_str = 'insert into demo (text) values ("%s")' % (ret['url'])
        print(sql_str)
        cursor.execute(sql_str)
    except Exception:
        s = 'error'
        sql_str = 'insert into demo (text) values ("%s")' % (s)
        cursor.execute(sql_str)
    print(sql_str)
    cursor.execute(sql_str)
    cursor.close()
    conn.close()
