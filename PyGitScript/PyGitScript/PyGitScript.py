#coding=utf-8

#通过minidom解析xml文件
import xml.dom.minidom as xmldom#xml解析库
import os#操作系统相关的库
import shutil#文件操作相关的库
import threading#定时器操作相关的库
import git#git操作相关的库
import platform#判断操作系统类型相关的库


global file_total_num#文件总数
global time_val#时间间隔
global repo#git版本库对象
global git_path#git版本库路径

#定时任务
def on_timer(interval_index,file_num):
	#cur_range = file_list[interval_index-file_num:interval_index]
	git_operator = repo.git#获取git操作对象
	git_operator.pull()#拉取远程版本库的文件
	prev_index = interval_index-file_num
	cur_range = file_list[prev_index:interval_index]

	#根据操作系统类型选择文件目录拼接符
	cat_symbol = None
	sysstr = platform.system()
	if(sysstr =="Windows"):
		cat_symbol = "\\"
	elif(sysstr == "Linux"):
		cat_symbol = "/"

	for file_path in cur_range:
		shutil.copy(file_path,  git_path)
		print("Already copy %s to %s..." % (file_path,git_path))
		#获取文件名称
		file_name = os.path.split(file_path)[-1]
		#拼接成git所在目录的文件路径
		git_file_path = git_path + cat_symbol + file_name
		git_operator.add(git_file_path) # 添加文件
		print("Already add %s to cache..." % file_path)
	git_operator.commit('-m', 'Add') # git commit -m 'this is a test'
	print("Commit to cache...")
	git_operator.push()
	print("Aready push all to remote...")
	next_interval_index = interval_index + file_num
	#如果下标即将超过总文件数，直接放弃下次定时任务
	if next_interval_index > file_total_num:
		print("Current commit %d files" % (interval_index))
		return
	#根据配置文件提供的信息复制指定数量的文件到当前目录
	timer = threading.Timer(time_val,on_timer,[next_interval_index,file_num])
	timer.start()

def getFileList(dir, fileList):
	newDir = dir
	if os.path.isfile(dir):
		fileList.append(dir)
	elif os.path.isdir(dir): 
		for s in os.listdir(dir):
		#如果需要忽略某些文件夹，使用以下代码
		##if s == "xxx":
		##continue
			newDir=os.path.join(dir,s)
			getFileList(newDir, fileList) 
	return fileList

#加载XML配置文件
xmlfilepath = os.path.abspath("config.xml")
print ("xml file path：", xmlfilepath)

# 得到文档对象
domobj = xmldom.parse(xmlfilepath)
print("xmldom.parse:", type(domobj))
# 得到元素对象
py_git_script_node = domobj.documentElement
print ("domobj.documentElement %s" % type(py_git_script_node))

#PyGitScript
src_path_node = py_git_script_node.getElementsByTagName("SrcPath")[0]
src_path_node_text = src_path_node.childNodes[0]
print ("Type: %s,Data: %s" % (type(src_path_node),src_path_node_text.data))

git_path_node = py_git_script_node.getElementsByTagName("GitPath")[0]
git_path_node_text = git_path_node.childNodes[0]
print ("Type: %s,Data: %s" % (type(git_path_node),git_path_node_text.data))

time_val_node = py_git_script_node.getElementsByTagName("TimeVal")[0]
time_val_node_text = time_val_node.childNodes[0]
print ("Type: %s,Data: %s" % (type(time_val_node),time_val_node_text.data))

file_num_node = py_git_script_node.getElementsByTagName("FileNum")[0]
file_num_node_text = file_num_node.childNodes[0]
print ("Type: %s,Data: %s" % (type(file_num_node),file_num_node_text.data))

#文件目录
src_path = src_path_node_text.data
#git版本库目录
git_path = git_path_node_text.data
#时间间隔(单位:秒)
time_val = int(time_val_node_text.data)
#每次提交的文件数
file_num = int(file_num_node_text.data)

#获取目录下所有文件
file_list = getFileList(src_path,[]) 
#print(file_list)
file_total_num = len(file_list)#文件列表长度
interval_index = file_num#截止下标

#初始化git版本库对象
repo = git.Repo(git_path)


#启动定时器执行定时任务
timer = threading.Timer(time_val,on_timer,[interval_index,file_num])  #首次启动
timer.start()
