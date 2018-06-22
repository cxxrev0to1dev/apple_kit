#!/usr/bin/env python
#-*- coding:utf-8 -*-
import os,sys,logging,urllib,urllib2,zlib,re,time,datetime,random,string
import poplib,socks,socket,Queue,re,base64
import json
from email import parser
############################################################################
class AppleidInfo:
	def __init__(self):
		self.accountName = None
		self.mail_password = None
		self.password = None
		self.confirmPassword = None
		self.firstName = None
		self.lastName = None
		self.birthday = None
		self.securityQuestion0 = None
		self.securityQuestion1 = None
		self.securityQuestion2 = None
		self.securityAnswer0 = None
		self.securityAnswer1 = None
		self.securityAnswer2 = None
	def __del__(self):
		self.accountName = None
		self.password = None
		self.confirmPassword = None
		self.firstName = None
		self.lastName = None
		self.birthday = None
		self.securityQuestion0 = None
		self.securityQuestion1 = None
		self.securityQuestion2 = None
		self.securityAnswer0 = None
		self.securityAnswer1 = None
		self.securityAnswer2 = None
	def SetAccountName(self,accountName):
		self.accountName = self.StringEncode(accountName)
	def SetMailPassword(self,mail_password):
		self.mail_password = self.StringEncode(mail_password)
	def SetPassword(self,password):
		self.password = self.StringEncode(password)
	def SetConfirmPassword(self,confirmPassword):
		self.confirmPassword = self.StringEncode(confirmPassword)
	def SetFirstName(self,firstName):
		self.firstName = self.StringEncode(firstName)
	def SetLastName(self,lastName):
		self.lastName = self.StringEncode(lastName)
	def SetBirthday(self,birthday):
		self.birthday = self.StringEncode(birthday)
	def SetSecurityQuestion0(self,securityQuestion0):
		self.securityQuestion0 = self.StringEncode(securityQuestion0)
	def SetSecurityQuestion1(self,securityQuestion1):
		self.securityQuestion1 = self.StringEncode(securityQuestion1)
	def SetSecurityQuestion2(self,securityQuestion2):
		self.securityQuestion2 = self.StringEncode(securityQuestion2)
	def SetSecurityAnswer0(self,securityAnswer0):
		self.securityAnswer0 = self.StringEncode(securityAnswer0)
	def SetSecurityAnswer1(self,securityAnswer1):
		self.securityAnswer1 = self.StringEncode(securityAnswer1)
	def SetSecurityAnswer2(self,securityAnswer2):
		self.securityAnswer2 = self.StringEncode(securityAnswer2)
	def GetAppleidInfo(self):
		appleid_info = {}
		appleid_info["accountName"] = self.accountName
		appleid_info["mail_password"] = self.mail_password
		appleid_info["password"] = self.password
		appleid_info["confirmPassword"] = self.confirmPassword
		appleid_info["firstName"] = self.firstName
		appleid_info["lastName"] = self.lastName
		appleid_info["birthday"] = self.birthday
		appleid_info["securityQuestion0"] = self.securityQuestion0
		appleid_info["securityQuestion1"] = self.securityQuestion1
		appleid_info["securityQuestion2"] = self.securityQuestion2
		appleid_info["securityAnswer0"] = self.securityAnswer0
		appleid_info["securityAnswer1"] = self.securityAnswer1
		appleid_info["securityAnswer2"] = self.securityAnswer2
		return urllib.urlencode(appleid_info)
	def AccountAutomatedInfoAdd(self):
		try:
			url = 'http://iosapi.appchina.com/admin/account/automated/info/add.json'
			data = json.loads(self.GetResponse(url,self.GetAppleidInfo()))
			return data['data']
		except:
			pass
		return False
	def AccountAutomatedInfoAddLoginExcept(self,account_name,mail_password):
		try:
			tag = "mail_server_login_except"
			self.accountName = account_name
			self.mail_password = mail_password
			self.password = tag
			self.confirmPassword = tag
			self.firstName = tag
			self.lastName = tag
			self.birthday = tag
			self.securityQuestion0 = tag
			self.securityQuestion1 = tag
			self.securityQuestion2 = tag
			self.securityAnswer0 = tag
			self.securityAnswer1 = tag
			self.securityAnswer2 = tag
			return self.AccountAutomatedInfoAdd()
		except:
			pass
		return False
	def AccountAutomatedInfoExists(self,str):
		try:
			appleid_info = {}
			appleid_info["accountName"] = str
			url = 'http://iosapi.appchina.com/admin/account/automated/info/exists.json'
			data = json.loads(self.GetResponse(url,urllib.urlencode(appleid_info)))
			return data['data']
		except:
			pass
		return False
	def GetResponse(self,url,str):
		headers = {
			"Connection": "keep-alive",
			"Content-Length": len(str),
			"User-Agent": "Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/49.0.2587.3 Safari/537.36",
			"Content-Type": "application/json",
			"Accept": "*/*",
			"Accept-Encoding": "identity",
			"Accept-Language": "zh-CN,zh;q=0.8"
		}
		response = urllib2.urlopen(url, str)
		data = response.read()
		try:
			tmp = data
			tmp = tmp.decode('utf-8')
			data = tmp
		except:
			pass
		return data
	def WrittenTest(self):
		f = open(str(os.path.abspath(os.curdir) + "\\settings_file\\appleid_test.mail"),'a')
		f.write(str(self.GetAppleidInfo() + "\n"))
		f.close()
	def StringEncode(self,str):
		return str.encode('utf-8')
if __name__ == '__main__':
	#SetPorxySocks5("socks5://127.0.0.1:1080")
	apple_info = AppleidInfo()
	apple_info.accountName = "luofei2@appchina.com"
	apple_info.password="123456"
	apple_info.confirmPassword="123456"
	apple_info.mail_password="123456"
	print apple_info.AccountAutomatedInfoAdd()
	print apple_info.AccountAutomatedInfoExists('luofei3@appchina.com')
	#InitAccountQueue()