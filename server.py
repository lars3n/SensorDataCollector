# coding=utf-8
from SocketServer import TCPServer, StreamRequestHandler
import time
import datetime
import threading

host = ''
port = 9003
addr = (host, port)


def handle(msg):
    raw_datas = msg.split(';')
    data_strs = []
    for ind in range(len(raw_datas)):
        data = raw_datas[ind].strip()
        if data:
            data_strs.append(data)

    light_val = 0
    datas = []
    for data_str in data_strs:
        data_arr = data_str.split(',')
        d = dict(
            name=data_arr[0],
            value=int(data_arr[1]),
            danger_value=int(data_arr[2]),
            is_alarm=int(data_arr[3]),
        )
        datas.append(d)
        if d['name'] == 'light':
            light_val = d['value']

    print datas
    is_alarm = False
    for data in datas:
        danger_value = data['danger_value']
        if data['value'] > danger_value:
            is_alarm = True
            break


class MyRequestHandler(StreamRequestHandler, object):
    def handle(self):
        self.request.settimeout(5)
        try:
            msg = self.rfile.readline()
        except Exception, e:
            print e
            return

        if msg.startswith('*'):
            print 'connected from ', self.client_address
            msg = msg.strip().lstrip('*')
            print 'msg:', msg
            handle(msg)

        self.wfile.write('RESP [%s]:%s ' % (time.ctime(), msg))

    def finish(self):
        super(MyRequestHandler, self).finish()
        # print 'closing connection...'
        # self.connection.close()


class SimpleServer(TCPServer):
    def __init__(self, server_address, RequestHandlerClass):
        TCPServer.__init__(self, server_address, RequestHandlerClass)

    def handle_timeout(self):
        print('%s timeout...' % time.time())


if __name__ == '__main__':
    tcpServ = SimpleServer(addr, MyRequestHandler)
    tcpServ.timeout = 3
    print 'Server running...'
    while 1:
        tcpServ.handle_request()
