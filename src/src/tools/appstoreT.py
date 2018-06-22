#!/usr/bin/env python
#-*- coding:utf-8 -*-
import os,sys,logging,urllib,urllib2,zlib,re,time,base64,datetime,random,string,socket
import requests,urllib,httplib,json,Queue,socks,random,binascii,uuid
from md5 import md5
from random import randint
from multiprocessing import Pool
############################################################################
socket_tmp = None
############################################################################
def GetResponse():
	sid = md5(str(uuid.uuid4())).hexdigest()
	request_https = urllib2.Request('http://register.xyzs.com/appstoreT/registerPc?sid=' + sid + '&from=xyzs&client_version=2015.12.08')
	request_https.add_header('Accept', "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8")
	request_https.add_header('Referer', "https://appleid.apple.com")
	request_https.add_header('Accept-Language', "zh-CN,zh;q=0.8")
	request_https.add_header('User-Agent', "WinHttp")
	response = urllib2.urlopen(request_https)
	return response.read()
class ProxyManagement:
	def __init__(self):
		self.proxy_queue = Queue.Queue()
	def __del__(self):
		self.proxy_queue.join()
	def ResetSocks5Queue(self):
		user_socks5 = open(str(os.path.abspath(os.curdir) + "\\settings_file\\user_socks5.bin"), 'r')
		print 'reset socks5 begin...'
		socket_temp = socket.socket
		socks5_list = []
		for line in user_socks5.readlines():
			try:
				proxy = line.strip('\n')
				socks.setdefaultproxy(socks.PROXY_TYPE_SOCKS5, proxy.split(":")[0], int(proxy.split(":")[1]))
				socket.socket = socks.socksocket
				if len(urllib2.urlopen('http://baidu.com').read()):
					print proxy,':OK!'
					socks5_list.append(proxy)
					#self.proxy_queue.put(proxy)
			except Exception,ex:
				print ex
				continue
		socket.socket = socket_temp
		random.shuffle(socks5_list)
		for index in socks5_list:
			self.proxy_queue.put(index)
		print 'reset socks5 end...'
	def GetSocks5(self):
		if self.proxy_queue.empty():
			self.ResetSocks5Queue()
		socks5_proxy = self.proxy_queue.get()
		remote_proxy = "socks5://" + socks5_proxy
		print remote_proxy
		return remote_proxy
	def SetSocket(self):
		global socket_tmp
		if socket_tmp!=None:
			socket.socket = socket_tmp
			socket_tmp = None
	def SetPorxy(self,proxy_server,proxy_port):
		global socket_tmp
		if socket_tmp==None:
			socket_tmp = socket.socket
		socks.setdefaultproxy(socks.PROXY_TYPE_SOCKS5, proxy_server, int(proxy_port))
	def SetPorxySocks5(self,proxy):
		proxy = proxy.replace("socks5://","")
		(proxy_server,proxy_port) = proxy.split(":")
		self.SetPorxy(proxy_server,proxy_port)
def WrittenAppstoreT(x):
	for interval in range(x):
		try:
			f = open(str(os.path.abspath(os.curdir) + "\\settings_file\\appstoreT.mail"),'a')
			f.write(str(GetResponse() + "\n"))
			f.close()
		except Exception,ex:
			print ex
			continue
	print 'OK!'
if __name__ == '__main__':
	socket.setdefaulttimeout(10)
	while True:
		try:
			pool = Pool(processes=20)
			result = pool.apply_async(WrittenAppstoreT, [5000000])
			result.get(timeout=10000)
			pool.close()
			pool.join()
		except:
			pass
	print 'OK!'