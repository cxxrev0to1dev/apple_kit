#!/usr/bin/env python
#-*- coding:utf-8 -*-
import os,sys,logging,urllib,urllib2,zlib,re,time,datetime,random,string
import poplib,socks,socket,Queue,re
from email import parser
import appleid_info
############################################################################
socket_tmp = None
account_queue = Queue.Queue()
############################################################################
def ClearedInbox(account_name,password,pop3_server = None):
	try:
		if pop3_server==None:
			pop3_server = 'pop3.sina.com.cn'
		mailserver = poplib.POP3_SSL(pop3_server)
		mailserver.getwelcome()
		mailserver.user(account_name)
		mailserver.pass_(password)
	except:
		appleid_registed = appleid_info.AppleidInfo()
		appleid_registed.AccountAutomatedInfoAddLoginExcept(account_name,password)
		return True
	else:
		(numMsgs, totalSize) = mailserver.stat()
		for index in range(int(numMsgs)):
			numMessages = len(mailserver.list()[1])
			newestEmail = mailserver.retr(numMessages)
			mailserver.dele(numMessages)
		mailserver.quit()
		return False
def GetOnlineCode(account_name,password,pop3_server = None):
	global appleid_registed
	online_code = None
	try:
		if pop3_server==None:
			pop3_server = 'pop3.sina.com.cn'
		mailserver = poplib.POP3_SSL(pop3_server)
		mailserver.getwelcome()
		mailserver.user(account_name)
		mailserver.pass_(password)
	except:
		appleid_registed = appleid_info.AppleidInfo()
		appleid_registed.AccountAutomatedInfoAddLoginExcept(account_name,password)
		return online_code
	else:
		(numMsgs, totalSize) = mailserver.stat()
		if numMsgs>0:
			messages = [mailserver.retr(len(mailserver.list()[1]))]# for i in range(1, len(mailserver.list()[1]) + 1)
			messages = ["\n".join(mssg[1]) for mssg in messages]
			messages = [parser.Parser().parsestr(mssg) for mssg in messages]
			for message in messages:
				for part in message.walk():
					if part.get_content_type():
						charset = part.get_content_charset()
						content = part.get_payload(decode=True)
						if content != None and 'appleid.apple.com' in content:
							content = content.decode(charset).encode('utf-8')
							#print content
							content = content.split('\n\n')
							for index in content:
								if len(index)>=6 and index.isdigit():
									online_code = index
									print online_code
							break
			mailserver.quit()
		return online_code
def GetResetPasswordURL(account_name,password,pop3_server = None):
	global appleid_registed
	reset_url = None
	try:
		if pop3_server==None:
			pop3_server = 'pop3.sina.com.cn'
		mailserver = poplib.POP3_SSL(pop3_server)
		mailserver.getwelcome()
		mailserver.user(account_name)
		mailserver.pass_(password)
	except:
		appleid_registed = appleid_info.AppleidInfo()
		appleid_registed.AccountAutomatedInfoAddLoginExcept(account_name,password)
		return reset_url
	else:
		(numMsgs, totalSize) = mailserver.stat()
		if numMsgs>0:
			messages = [mailserver.retr(len(mailserver.list()[1]))]# for i in range(1, len(mailserver.list()[1]) + 1)
			messages = ["\n".join(mssg[1]) for mssg in messages]
			messages = [parser.Parser().parsestr(mssg) for mssg in messages]
			for message in messages:
				for part in message.walk():
					if part.get_content_type():
						charset = part.get_content_charset()
						content = part.get_payload(decode=True)
						if content != None and 'appleid.apple.com' in content:
							content = content.decode(charset).encode('utf-8')
							urls = re.findall('http[s]?://(?:[a-zA-Z]|[0-9]|[$-_@.&+]|[!*\(\),]|(?:%[0-9a-fA-F][0-9a-fA-F]))+', content)
							for index in urls:
								if "key=" in index:
									reset_url = index
									break
							break
		mailserver.quit()
	return reset_url
def SetSocket():
	global socket_tmp
	if socket_tmp!=None:
		socket.socket = socket_tmp
		socket_tmp = None
def SetPorxy(proxy_server,proxy_port):
	global socket_tmp
	if socket_tmp==None:
		socket_tmp = socket.socket
	socks.setdefaultproxy(socks.PROXY_TYPE_SOCKS5, proxy_server, int(proxy_port))
def SetPorxySocks5(proxy):
	proxy = proxy.replace("socks5://","")
	(proxy_server,proxy_port) = proxy.split(":")
	SetPorxy(proxy_server,proxy_port)
def InitAccountQueue():
	global account_queue
	appleid_registed = appleid_info.AppleidInfo()
	lines = [line.rstrip('\n') for line in open('account.mail')]
	for index in lines:
		try:
			index = index.split(":")
			(account_name,password) = index
			if appleid_registed.AccountAutomatedInfoExists(account_name):
				continue
			account_name = str(account_name)
			password = str(password)
			info = []
			info.append(account_name)
			info.append(password)
			print info
			account_queue.put(info)
		except Exception,e:
			pass
def GetAccount():
	global account_queue
	if account_queue.empty():
		return None
	return account_queue.get()
def dump():
	lines = [line.rstrip('\n') for line in open('account_mail.taobao')]
	with open("account.mail","ab") as f:
		for index in lines:
			try:
				index = index.split("£º")
				account = index[1].split("----")
				(account_name,password) = account
				account_name = str(account_name)
				password = str(password)
				print account_name,password
				f.write(account_name)
				f.write(":")
				f.write(password)
				f.write("\r\n")
			except Exception,e:
				pass
if __name__ == '__main__':
	#SetPorxySocks5("socks5://127.0.0.1:1080")
	GetResetPasswordURL("jwsgibg166@sina.com","rcpto369")
	#InitAccountQueue()