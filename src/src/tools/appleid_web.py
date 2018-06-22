#!/usr/bin/env python
#-*- coding:utf-8 -*-
import socks,UUWiseHelper
from random import randint
import os,sys,logging,urllib,urllib2,zlib,re,time,base64,datetime,random,string,itertools,shelve
import threading
from selenium import webdriver
from selenium.webdriver.common.keys import Keys
from selenium.webdriver.support.ui import Select
from PIL import Image
import socket,Queue
import account_mail,appleid_info

def MakePasswd(length=12, digits=4, upper=4, lower=4):
    random.seed(time.time())
    lowercase = string.lowercase.translate(None, "o")
    uppercase = string.uppercase.translate(None, "O")
    letters = "{0:s}{1:s}".format(lowercase, uppercase)
    password = list(
        itertools.chain(
            (random.choice(uppercase) for _ in range(upper)),
            (random.choice(lowercase) for _ in range(lower)),
            (random.choice(string.digits) for _ in range(digits)),
            (random.choice(letters) for _ in range((length - digits - upper - lower)))
        )
    )
    return "".join(random.sample(password, len(password)))
class ChinesePinYin:
	def OpenTable(self):
		f = open(os.path.abspath(os.curdir) + "\\settings_file\\chinese_pinyin.bin", 'r')
		table = f.read()
		f.close()
		return table
	def SearchPinyin(self,num, table):
		if(num>0 & num<160):
			return chr(num)
		v=table.split(';')
		for i in xrange(len(v)-1,-1,-1):
			s=v[i].split(',')
			if int(s[1])<=num:
				return s[0]
	def ChineseToPinYin(self,chinese):
		i=0
		str = ''
		table = self.OpenTable()
		while(i<len(chinese)-1):
			p = ord(chinese[i:i+1])
			if(p>160):
				i+=1
				q = ord(chinese[i:i+1])
				p = p*256+q-65536
			i+=1
			if str=='':
				str = '%s' % (self.SearchPinyin(p,table))
			else:
				tmp = '%s#%s' % (str, self.SearchPinyin(p,table))
				if len(tmp)<16:
					str = tmp
		return str
def ChineseUsername():
	chinese_username = []
	file = open(os.path.abspath(os.curdir) + "\\settings_file\\chinese_username.bin")
	chinese_pinyin = ChinesePinYin()
	while 1:
		username = file.readline()
		if not username:
			break
		chinese_username.append(chinese_pinyin.ChineseToPinYin(username))
	file.close()
	random.shuffle(chinese_username)
	return chinese_username
class SinaConfig:
	def __init__(self):
		self.chinese_username = ChineseUsername()
		os.environ["chromedriver"] = str(os.path.abspath(os.curdir) + "\\chromedriver.exe")
		self.capcha_img = os.environ["TMP"] + "\\capcha.jpg"
		self.proxy_queue = Queue.Queue()
		self.ResetSocks5()
	def ChromeOptionsArgProxy(self):
		chrome_options = webdriver.ChromeOptions()
		user_agents = [
			'Mozilla/5.0 (X11; U; Linux; i686; en-US; rv:1.6) Gecko Debian/1.6-7',
			'Konqueror/3.0-rc4; (Konqueror/3.0-rc4; i686 Linux;;datecode)',
			'Opera/9.52 (X11; Linux i686; U; en)',
			'Opera/9.00 (Windows NT 5.1; U; en)',
			'Opera/8.52 (X11; Linux i386; U; de)',
			'Mozilla/5.0 (compatible; MSIE 10.0; Macintosh; Intel Mac OS X 10_7_3; Trident/6.0)',
			'Mozilla/5.0 (Windows NT 6.1) AppleWebKit/537.2 (KHTML, like Gecko) Chrome/22.0.1216.0 Safari/537.2',
			'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_7_4) AppleWebKit/537.13 (KHTML, like Gecko) Chrome/24.0.1290.1 Safari/537.13',
			'Mozilla/5.0 (X11; CrOS i686 2268.111.0) AppleWebKit/536.11 (KHTML, like Gecko) Chrome/20.0.1132.57 Safari/536.11',
			'Mozilla/5.0 (Windows NT 6.2; Win64; x64; rv:16.0.1) Gecko/20121011 Firefox/16.0.1',
			'Mozilla/5.0 (Windows; U; Windows NT 6.0; de; rv:1.9.2.6) Gecko/20100625 Firefox/3.6.6 GTB7.1 (.NET CLR 3.5.30729)'
		]
		chrome_options.add_argument("user-agent=%s" % random.choice(user_agents))
		chrome_options.add_argument("--lang=zh-CN");
		socks5 = self.GetSocks5()
		account_mail.SetPorxySocks5(socks5)
		chrome_options.add_argument('--proxy-server=%s' % socks5)
		return chrome_options
	def NewChrome(self):
		is_ok = False
		while is_ok==False:
			try:
				new_driver = webdriver.Chrome(chrome_options=self.ChromeOptionsArgProxy())
				new_driver.set_window_size(960, 800)
				time.sleep(random.randint(1, 2))
				is_ok = True
			except Exception,ex:
				print ex
				time.sleep(random.randint(1, 2))
				continue
		return new_driver
	def ResetSocks5(self):
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
			self.ResetSocks5()
		socks5_proxy = self.proxy_queue.get()
		remote_proxy = "socks5://" + socks5_proxy
		print remote_proxy
		return remote_proxy
	def __del__(self):
		self.proxy_queue.join()
sina_config = SinaConfig()
apple_driver = None#sina_config.NewChrome()
def ElementsClick(elements,index):
	if elements!=[]:
		elements[index].click()
def Checkcode():
	while os.path.exists(sina_config.capcha_img):
		checksum_code  = UUWiseHelper.GetChecksumCode(sina_config.capcha_img,101826,'29790881598f49a09838baccc60359bc','36362FA1-7912-4817-A0CE-E51E038B0784','miaozhijian','uudama')
		os.remove(sina_config.capcha_img)
		break
	if len(checksum_code)>=2 and checksum_code[0]==True:
		capcha_code = checksum_code[1].encode('ascii', 'ignore')
		return capcha_code
	return None
if __name__ == '__main__':
	socket.setdefaulttimeout(250)
	success_count = 0;
	#name_index = 0
	chinese_username_len = len(sina_config.chinese_username)
	account_mail.InitAccountQueue()
	while success_count!=400:
		try:
			capcha_src = None
			appleid_registed = appleid_info.AppleidInfo()
			apple_driver = sina_config.NewChrome()
			apple_username = account_mail.GetAccount()
			mail_password = apple_username[1]
			apple_username = apple_username[0]
			apple_password = None
			while account_mail.ClearedInbox(apple_username,mail_password):
				apple_username = account_mail.GetAccount()
				mail_password = apple_username[1]
				apple_username = apple_username[0]
			print 'sina:',apple_username,"-",mail_password
			apple_driver.get('https://appleid.apple.com/account')
			mailQuit = 0
			while ('web/account/client' in apple_driver.page_source)!=True:
				if mailQuit>10:
					break
				time.sleep(random.randint(1, 3))
				mailQuit += 1
			while os.path.exists(sina_config.capcha_img):
				os.remove(sina_config.capcha_img)
			if "accountName" in apple_driver.page_source:
				text_area = apple_driver.find_element_by_id('accountName')
				for user_i in range(len(apple_username)):
					text_area.send_keys(apple_username[user_i])
					time.sleep(random.randint(0, 1))
				appleid_registed.SetAccountName(apple_username)
				appleid_registed.SetMailPassword(mail_password)
			if "password" in apple_driver.page_source:
				text_area = apple_driver.find_element_by_id('password')
				password = MakePasswd()
				apple_password = password
				for index in range(len(password)):
					text_area.send_keys(password[index])
					time.sleep(random.randint(0, 1))
				text_area = apple_driver.find_element_by_id('confirmPassword')
				for index in range(len(password)):
					text_area.send_keys(password[index])
					time.sleep(random.randint(0, 1))
				appleid_registed.SetPassword(apple_password)
				appleid_registed.SetConfirmPassword(apple_password)
			if apple_driver.find_elements_by_class_name('has-errors')!=[]:
				apple_driver.get('https://iforgot.apple.com/password/verify/appleid#!&section=password')
				mail_quit = 0
				while ('appleid' in apple_driver.page_source)!=True:
					if mail_quit>10:
						break
					time.sleep(random.randint(1, 3))
					mail_quit += 1
				if 'appleid' in apple_driver.page_source:
					print 'form-control account-name has-errors'
					text_area = apple_driver.find_element_by_id('appleid')
					for index in apple_username:
						text_area.send_keys(index)
						time.sleep(random.randint(0, 1))
					ElementsClick(apple_driver.find_elements_by_id('action'),0)
					time.sleep(random.randint(3, 5))
					print '11111111111111111'
					mail_quit = 0
					while ('optionquestions' in apple_driver.page_source)!=True:
						if mail_quit>10:
							break
						time.sleep(random.randint(1, 3))
						mail_quit += 1
					print '22222222222222222'
					ElementsClick(apple_driver.find_elements_by_id('action'),0)
					time.sleep(random.randint(5, 10))
					print '33333333333333333'
					url = account_mail.GetResetPasswordURL(apple_username,mail_password)
					print url
					if url!=None:
						appleid_registed.SetAccountName(apple_username)
						appleid_registed.SetMailPassword(mail_password)
						apple_driver.get(url)
						mail_quit = 0
						while ('password' in apple_driver.page_source)!=True:
							if mail_quit>10:
								break
							time.sleep(random.randint(1, 3))
							mail_quit += 1
						text_area = apple_driver.find_element_by_id('password')
						for index in apple_password:
							text_area.send_keys(index)
							time.sleep(random.randint(0, 1))
						text_area = apple_driver.find_element_by_id('confirmPassword')
						for index in apple_password:
							text_area.send_keys(index)
							time.sleep(random.randint(0, 1))
						time.sleep(random.randint(3, 5))
						print '44444444444444444444444444'
						ElementsClick(apple_driver.find_elements_by_id('action'),0)
						print '55555555555555555555555555'
						time.sleep(random.randint(5, 10))
						appleid_registed.SetPassword(apple_password)
						appleid_registed.SetConfirmPassword(apple_password)
						print appleid_registed.AccountAutomatedInfoAdd()
						#f = open(str(os.path.abspath(os.curdir) + "\\settings_file\\appleid.mail"),'a')
						#f.write(str(apple_username + ":" + apple_password + "\n"))
						#f.close()
				apple_driver.quit()
				continue
			if "firstName" in apple_driver.page_source:
				lastName = ''
				firstName = ''
				while len(lastName)<=0 or len(firstName)<=0:
					name_index = random.randint(1, chinese_username_len)
					sina_config.chinese_username[name_index].strip(' \t\n\r')
					username = sina_config.chinese_username[name_index]
					username = username.split("#")
					lastName = filter(str.isalnum, username[0])
					for index in range(len(username)-1):
						firstName += filter(str.isalnum, username[index+1])
				text_area = apple_driver.find_element_by_id('firstName')
				for index in range(len(firstName)):
					text_area.send_keys(firstName[index])
					time.sleep(random.randint(0, 1))
				text_area = apple_driver.find_element_by_id('lastName')
				for index in range(len(lastName)):
					text_area.send_keys(lastName[index])
					time.sleep(random.randint(0, 1))
				appleid_registed.SetFirstName(firstName)
				appleid_registed.SetLastName(lastName)
			if "aid-birthday-input" in apple_driver.page_source:
				yyyy = str(randint(1980,1999))
				while (int(yyyy)<=1980 or int(yyyy)>=1999) or len(yyyy)!=4 or yyyy.isdigit()!=True:
					yyyy = str(randint(1980,1999))
				mm = str(randint(10,12))
				while (int(mm)<=10 or int(mm)>=12) or len(mm)!=2 or mm.isdigit()!=True:
					mm = str(randint(10,12))
				yy = str(randint(10,28))
				while (int(yy)<=10 or int(yy)>=28) or len(yy)!=2 or yy.isdigit()!=True:
					yy = str(randint(10,28))
				text_area = apple_driver.find_element_by_id('aid-birthday-input')
				for index in yyyy:
					text_area.send_keys(index)
					time.sleep(random.randint(0, 1))
				text_area.send_keys('-')
				text_area = apple_driver.find_element_by_id('aid-birthday-input')
				for index in mm:
					text_area.send_keys(index)
					time.sleep(random.randint(0, 1))
				text_area.send_keys('-')
				text_area = apple_driver.find_element_by_id('aid-birthday-input')
				for index in yy:
					text_area.send_keys(index)
					time.sleep(random.randint(0, 1))
				appleid_registed.SetBirthday(str(yyyy)+str(mm)+str(yy))
			if "question0" in apple_driver.page_source and "question1" in apple_driver.page_source and "question2" in apple_driver.page_source:
				Select(apple_driver.find_element_by_class_name("securityQuestion0")).select_by_index(1)
				Select(apple_driver.find_element_by_class_name("securityQuestion1")).select_by_index(1)
				Select(apple_driver.find_element_by_class_name("securityQuestion2")).select_by_index(1)
				appleid_registed.SetSecurityQuestion0("select_by_index(1)")
				appleid_registed.SetSecurityQuestion1("select_by_index(1)")
				appleid_registed.SetSecurityQuestion2("select_by_index(1)")
			if "answer0" in apple_driver.page_source:
				answer0 = apple_driver.find_element_by_id('answer0')
				answer0.click()
				actions = webdriver.ActionChains(apple_driver)
				for index in range(4):
					actions.send_keys(Keys.NUMPAD1)
					time.sleep(random.randint(0, 1))
				actions.send_keys(Keys.TAB)
				actions.perform()
				appleid_registed.SetSecurityAnswer0("1111")
			if "answer1" in apple_driver.page_source:
				apple_driver.find_element_by_id('answer1').click()
				actions = webdriver.ActionChains(apple_driver)
				for index in range(4):
					actions.send_keys(Keys.NUMPAD2)
					time.sleep(random.randint(0, 1))
				actions.send_keys(Keys.TAB)
				actions.perform()
				appleid_registed.SetSecurityAnswer1("2222")
			if "answer2" in apple_driver.page_source:
				apple_driver.find_element_by_id('answer2').click()
				actions = webdriver.ActionChains(apple_driver)
				for index in range(4):
					actions.send_keys(Keys.NUMPAD3)
					time.sleep(random.randint(0, 1))
				actions.send_keys(Keys.TAB)
				actions.perform()
				appleid_registed.SetSecurityAnswer2("3333")
			if "data:image/jpeg;base64," in apple_driver.page_source:
				capcha_src = apple_driver.page_source[apple_driver.page_source.index('"CAPTCHA"'):]
				capcha_src = capcha_src[capcha_src.index(','):capcha_src.index('/>')]
				capcha_dst = capcha_src
				while capcha_dst==capcha_src:
					capcha_bin = base64.b64decode(capcha_src)
					with open(sina_config.capcha_img,'wb') as f:
						f.write(capcha_bin)
					checksum_code = []
					capcha_code = Checkcode()
					apple_driver.find_element_by_id('typeTheCharacters').click()
					text_area = apple_driver.find_element_by_id('typeTheCharacters')
					for index in capcha_code:
						text_area.send_keys(index)
						time.sleep(random.randint(0, 1))
					ElementsClick(apple_driver.find_elements_by_class_name('verify'),0)
					time.sleep(random.randint(3, 3))
					if "\"CAPTCHA\"" in apple_driver.page_source:
						capcha_dst = apple_driver.page_source[apple_driver.page_source.index('"CAPTCHA"'):]
						capcha_dst = capcha_dst[capcha_dst.index(','):capcha_dst.index('/>')]
						if capcha_dst!=capcha_src:
							capcha_src = capcha_dst
							while os.path.exists(sina_config.capcha_img):
								os.remove(sina_config.capcha_img)
							text_area.send_keys("")
							print "reset capcha input!!!"
							continue
						else:
							capcha_dst = None
				for verify_resend_index in range(3):
					code = None
					for mail_index in range(10):
						time.sleep(random.randint(12, 12))
						code = account_mail.GetOnlineCode(apple_username,mail_password)
						if code!=None:
							break
					if code!=None:
						char1 = apple_driver.find_elements_by_id('char1')
						if char1!=[]:
							char1[0].send_keys(str(code))
						time.sleep(random.randint(2, 3))
						ElementsClick(apple_driver.find_elements_by_class_name('create'),0)
						for reg_ok_index in range(25):
							try:
								if "view-devices" in apple_driver.page_source:
									break
								time.sleep(random.randint(1, 1))
							except:
								time.sleep(random.randint(1, 1))
								continue
						if "view-devices" in apple_driver.page_source:#reg ok!
							print "registed apple id ok!"
							print appleid_registed.AccountAutomatedInfoAdd()
							#f = open(str(os.path.abspath(os.curdir) + "\\settings_file\\appleid.mail"),'a')
							#f.write(str(apple_username + ":" + apple_password + "\n"))
							#f.close()
							success_count += 1
							break
						if ((verify_resend_index+1)%2)==0:
							ElementsClick(apple_driver.find_elements_by_class_name('new-code'),0)
						else:
							ElementsClick(apple_driver.find_elements_by_class_name('verify-resend'),0)
						continue
			apple_driver.quit()
		except Exception,ex:
			print ex
			apple_driver.quit()
			while os.path.exists(sina_config.capcha_img):
				os.remove(sina_config.capcha_img)
			continue
	apple_driver.quit()
	print 'OK!!!!!!'