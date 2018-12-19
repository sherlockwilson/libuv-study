#coding=utf-8

#通过minidom解析xml文件
import xml.dom.minidom as xmldom#xml解析库
import os#操作系统相关的库
import shutil#文件操作相关的库
import threading#定时器操作相关的库
import git#git操作相关的库
import platform#判断操作系统类型相关的库
import random#随机数相关接口
import chardet#用来查看文件编码
import schedule#定时任务
import time#时间


global file_total_num#文件总数
global time_val#时间间隔
global repo#git版本库对象
global git_path#git版本库路径
global sysstr#操作系统类型
global cat_symbol#连接文件夹与文件夹之间的连接符，其中windows下是\\，Linux下是/
global local_file_list#本地文件列表
global commit_file_num#需要提交的文件数
global t

def replace_file_path(file):
	return file.replace("\\\\","\\")

def complete_file_path(file):
	if sysstr == "Windows":
		return (git_path + file.replace("/","\\")).replace("\\\\","\\")
	else:
		return file

#定时任务
def on_timer():
	global commit_file_num
	global t
	gitobj = repo.git#获取git操作对象
	print(gitobj.pull())#拉取远程版本库的文件并打印信息
	#通过git对象获取文件列表信息
	str_ls_files = gitobj.ls_files()
	#转换成列表
	raw_ls_init_files = str_ls_files.strip('\n').split('\n')
	#转换成完整文件路径列表
	ls_init_files = [complete_file_path(file) for file in raw_ls_init_files]
	#将本地文件列表减去本地仓库文件列表，剩下的就是还未提交的文件列表
	not_commit_list = list(set(local_file_list)- set(ls_init_files))

	#判断配置中需要提交的文件数是否大于未提交的文件数
	if commit_file_num > len(not_commit_list):
		commit_file_num = len(not_commit_list)
	#如果提交的文件数是0
	if commit_file_num == 0 :
		print("end commit")
		return
	#随机获取提交文件生成本轮要提交的文件列表
	current_commit_list = random.sample(not_commit_list,commit_file_num)
	#存放所有文件的行信息
	content=[]
	file_name_list = []
	for current_commit_file_path in current_commit_list:
		#获取文件名称
		file_name = os.path.split(current_commit_file_path)[-1]
		file_name_list.append(file_name.encode('utf-8'))
		#按行读取文件中所有数据（过滤掉换行符）
		with open(current_commit_file_path,'r') as f:
			data = f.read()
			if None == data:
				continue
			f_char_info = chardet.detect(data)
			if f_char_info['encoding'] == "ascii":
				pass
			else :
				data.decode(f_char_info['encoding']).encode('utf-8')
			content += data.splitlines()
		gitobj.add(current_commit_file_path) # git add文件并打印信息
		print("add %s" % current_commit_file_path)
	#获取包含//的列表
	match_list = [one for one in content if False == one.find("//")]
	#需要加入commit后面的注释信息
	comment = ""
	#随机获取一条包含//的行作为comments
	if match_list:
		comment= random.sample(match_list,1)[0]
	#按,分割文件的提交信息
	commit_file_info = ",".join(file_name for file_name in file_name_list)
	print(gitobj.commit("-a",'-m', 'Add ' + commit_file_info + " " + comment.encode('utf-8'))) #打印提交信息
	print(gitobj.push("-u","origin","master"))#打印推送信息
	#如果还有文件可读，且当前模式支持随机时间，则清空当前所有任务并计算随机时间执行
	if time_val < 0:
		#清楚所有任务并在3600秒内随机选个时间执行
		t=threading.Timer(random.randint(3600),on_timer)
	else:
		t=threading.Timer(time_val,on_timer)
	t.start()
#获取文件列表
def getFileList(dir, fileList):
	newDir = dir
	if os.path.isfile(dir):
		fileList.append(dir)
	elif os.path.isdir(dir): 
		for s in os.listdir(dir):
			newDir=os.path.join(dir,s)
			getFileList(newDir, fileList) 
	return fileList

if __name__ == "__main__":
	#操作系统名
	sysstr = platform.system()
	if(sysstr =="Windows"):
		cat_symbol = "\\"
	elif(sysstr == "Linux"):
		cat_symbol = "/"
	#加载XML配置文件
	xmlfilepath = os.path.abspath("config.xml")
	print ("xml file path：", xmlfilepath)
	# 得到文档对象
	domobj = xmldom.parse(xmlfilepath)
	# 得到元素对象
	py_git_script_node = domobj.documentElement
	git_path_node = py_git_script_node.getElementsByTagName("GitPath")[0]
	git_path_node_text = git_path_node.childNodes[0]

	git_commit_path_node = py_git_script_node.getElementsByTagName("GitCommitPath")[0]
	git_commit_path_node_text = git_commit_path_node.childNodes[0]

	time_val_node = py_git_script_node.getElementsByTagName("TimeVal")[0]
	time_val_node_text = time_val_node.childNodes[0]

	file_num_node = py_git_script_node.getElementsByTagName("FileNum")[0]
	file_num_node_text = file_num_node.childNodes[0]

	#git版本库目录
	git_path = git_path_node_text.data
	#git提交文件的目录路径
	git_commit_path = git_commit_path_node_text.data
	#时间间隔(单位:秒)
	time_val = int(time_val_node_text.data)
	#每次提交的文件数
	commit_file_num = int(file_num_node_text.data)

	#获取提交目录下所有文件
	local_file_list = getFileList(git_commit_path,[])
	if sysstr == "Windows":
		local_file_list = [replace_file_path(local_file) for local_file in local_file_list]
	file_total_num = len(local_file_list)#文件列表长度

	#初始化git版本库对象
	repo = git.Repo(git_path)

	#启动定时器执行定时任务
	#3600秒内随机选个时间执行
	if time_val < 0:
		t=threading.Timer(random.randint(3600),on_timer)
	#否则启用time_val指定的时间间隔来执行任务
	else:
		t=threading.Timer(time_val,on_timer)
	t.start()

