#!/usr/bin/env python
#-*- coding:utf-8 -*-
import os,sys,logging,urllib,urllib2,zlib,re,time,base64,datetime,random,string,socket
import requests,urllib,httplib,json,Queue,socks
from random import randint
############################################################################
cookie = ""
socket_tmp = None
############################################################################
def GetValidation(account_name,scnt):
	global cookie
	headers = {
		"Connection": "keep-alive",
		"Content-Length": len(account_name)+2,
		"scnt": scnt,
		"Origin": "https://appleid.apple.com",
		"User-Agent": "Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/49.0.2587.3 Safari/537.36",
		"Content-Type": "application/json",
		"Accept": "*/*",
		"DNT": 1,
		"Cookie":cookie,
		"X-Requested-With": "XMLHttpRequest",
		"X-Apple-Api-Key": "cbf64fd6843ee630b463f358ea0b707b",
		"Referer": "https://appleid.apple.com/account",
		"Accept-Encoding": "identity",
		"Accept-Language": "zh-CN,zh;q=0.8"
	}
	pyload = '"' + account_name + '"'
	request_https = urllib2.Request('https://appleid.apple.com/account/validation/appleid', pyload, headers)
	response = urllib2.urlopen(request_https)
	return response.read()
def GetSCNT():
	global cookie
	request_https = urllib2.Request('https://appleid.apple.com/account')
	request_https.add_header('Accept', "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8")
	request_https.add_header('Accept-Encoding', "gzip, deflate, sdch")
	request_https.add_header('Referer', "https://appleid.apple.com")
	request_https.add_header('Accept-Language', "zh-CN,zh;q=0.8")
	request_https.add_header('User-Agent', "Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/49.0.2587.3 Safari/537.36")
	response = urllib2.urlopen(request_https)
	items = str(response.headers).split('\r\n')
	for index in items:
		if "Set-Cookie" in index:
			set_cookie = index[index.find(':') + 1:]
			if set_cookie[0]==' ':
				set_cookie = set_cookie[1:]
				set_cookie = set_cookie[0:set_cookie.find(';')]
			if cookie!=None and set_cookie in cookie:
				continue
			if cookie!="":
				cookie += "; "
			cookie += set_cookie
	request_https = urllib2.Request('https://appleid.apple.com/sessionIdleTimeOut')
	request_https.add_header('Cookie', cookie)
	request_https.add_header('Accept-Encoding', "gzip, deflate, sdch")
	request_https.add_header('Accept', "*/*")
	request_https.add_header('Referer', "https://appleid.apple.com/account")
	request_https.add_header('Accept-Language', "zh-CN,zh;q=0.8")
	request_https.add_header('User-Agent', "Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/49.0.2587.3 Safari/537.36")
	response = urllib2.urlopen(request_https)
	items = str(response.headers).split('\r\n')
	scnt = None
	for index in items:
		if "scnt" in index:
			scnt = index[index.find(':') + 1:]
			if scnt[0]==' ':
				scnt = scnt[1:]
			break
	return scnt
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
if __name__ == '__main__':
	socket.setdefaulttimeout(10)
	scnt = GetSCNT()
	account_file_lines = [line.rstrip('\n') for line in open(str(os.path.abspath(os.curdir) + "\\account.mail"))]
	proxy_mangage = ProxyManagement()
	proxy_mangage.SetPorxySocks5(proxy_mangage.GetSocks5())
	interval = 1
	for index in account_file_lines:
		try:
			(account_name,password) = index.split(":")
			if (interval%5)==0:
				scnt = GetSCNT()
				proxy_mangage.SetPorxySocks5(proxy_mangage.GetSocks5())
			validation_json = json.loads(GetValidation(account_name,scnt))
			if validation_json['used']==True:
				print validation_json
				f = open(str(os.path.abspath(os.curdir) + "\\settings_file\\appleid_used.mail"),'a')
				f.write(str(account_name + "\n"))
				f.close()
			elif validation_json['valid']!=True:
				print validation_json
				f = open(str(os.path.abspath(os.curdir) + "\\settings_file\\appleid_valid.mail"),'a')
				f.write(str(account_name + "\n"))
				f.close()
			elif validation_json['used']==False and validation_json['valid']==True:
				print validation_json
				continue
			else:
				print "else:",validation_json
				f = open(str(os.path.abspath(os.curdir) + "\\settings_file\\appleid_exception.mail"),'a')
				f.write(str(index + "\n"))
				f.close()
			interval += 1
		except Exception,ex:
			print ex
			proxy_mangage.SetPorxySocks5(proxy_mangage.GetSocks5())
			f = open(str(os.path.abspath(os.curdir) + "\\settings_file\\appleid_exception.mail"),'a')
			f.write(str(index + "\n"))
			f.close()
			continue
	print 'OK!'