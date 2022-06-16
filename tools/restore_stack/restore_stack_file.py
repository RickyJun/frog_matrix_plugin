# This is a sample Python script.

# Press ⌃R to execute it or replace it with your code.
# Press Double ⇧ to search everywhere for classes, files, tool windows, actions, and settings.

# 移动包打完包，自动生产libapp.so符号表，跟着aab，apk一起上传到仓库里
#
import datetime
import string
import os
import sys
import urllib.error
from urllib import request
import json

import config.config as config

WaitRestoreStackFile = "wait_restore_stack_file.json"
##
ARM64AddressLen = 16
ARM32AddressLen = 8
##
AppSymbolsDict = {}
FlutterSymbolsDict = {}
SymbolsDict = {}


def load_symbols(flutter_version, app_version, cpu):
    # 检查libapp，libflutter符号文件
    local_app_symbols_file = "{0}/{1}/version{2}_{3}".format(os.path.abspath('.'), cpu, app_version, config.AppSymbolName)
    local_flutter_symbols_file = "{0}/{1}/version{2}_{3}".format(os.path.abspath('.'), cpu, flutter_version, config.FlutterSymbolName)
    if not os.path.exists(local_app_symbols_file):
        app_symbol_url = config.AppSymbolUrl.format(app_version, config.AppSymbolName)
        try:
            print("precess 5%...start download "+app_symbol_url)
            request.urlretrieve(app_symbol_url, local_app_symbols_file)
            print("precess 10%...end download " + app_symbol_url)
        except urllib.error.HTTPError as e:
            print("precess error : AppSymbolUrl load from " + app_symbol_url + "errCode:", e.code)

    if not os.path.exists(local_flutter_symbols_file):
        flutter_symbol_url = config.FlutterSymbolUrl.format(app_version, config.FlutterSymbolName)
        try:
            print("precess 15%...start download " + flutter_symbol_url)
            request.urlretrieve(flutter_symbol_url, local_app_symbols_file)
            print("precess 20%...end download " + flutter_symbol_url)
        except urllib.error.HTTPError as e:
            print("FlutterSymbolUrl load from " + flutter_symbol_url + "errCode:{0}", e.code)

    # 读到内存中
    if os.path.exists(local_app_symbols_file):
        with open(local_app_symbols_file, 'r') as local_app_file:
            line = local_app_file.readline()
            symbol_line = line.split(" ")
            try:
                AppSymbolsDict[symbol_line[0]] = symbol_line[2]
            except:
                print("local_app_file error")
    if os.path.exists(local_flutter_symbols_file):
        with open(local_flutter_symbols_file, 'r') as local_flutter_file:
            while True:
                line = local_flutter_file.readline()
                line.lstrip()
                line.rstrip()
                if not line:
                    break
                symbol_line = line.split(" ")
                if symbol_line[0].startswith("00"):
                    try:
                        FlutterSymbolsDict[symbol_line[0]] = symbol_line[2]
                    except:
                        print("local_flutter_file error")

    SymbolsDict["app"] = AppSymbolsDict
    SymbolsDict["flutter"] = FlutterSymbolsDict


def read_stack_file(input_file_url):
    local_stack_file = "{0}/{1}".format(os.path.abspath('.'), WaitRestoreStackFile)
    if not os.path.exists(local_stack_file):
        print("precess 25%...start download "+input_file_url)
        request.urlretrieve(input_file_url, local_stack_file)
        print("precess 30%...end download "+input_file_url)
    with open(local_stack_file, 'r') as local_app_file:
        stack_content = local_app_file.read()

    return json.loads(stack_content)


def write_stack_file(output_stack_json_obj, output_file_dir):
    print("precess 95%...start output")
    json_str = json.dumps(output_stack_json_obj)
    json_str = json_str.replace("{", "{\n")
    json_str = json_str.replace("}", "\n}")
    json_str = json_str.replace("[", "[\n")
    json_str = json_str.replace("]", "\n]")
    json_str = json_str.replace(",", ",\n")
    if not os.path.exists(output_file_dir):
        os.mkdir(output_file_dir)
    file_name = datetime.datetime.now().strftime('%Y_%m_%d') + "_stack_file_restored.json"
    if output_file_dir.endswith("/") or output_file_dir.endswith("\\"):
        output_file_path = output_file_dir + file_name
    else:
        output_file_path = output_file_dir + "/" + file_name
    if os.path.exists(output_file_path):
        os.remove(output_file_path)
    with open(output_file_path, 'w') as local_stack_output_file:
        local_stack_output_file.write(json_str)
    tem_local_stack_file = "{0}/{1}".format(os.path.abspath('.'), WaitRestoreStackFile)
    if os.path.exists(tem_local_stack_file):
        os.remove(tem_local_stack_file)
    print("success")


def alignment_address(address, address_len):
    i = 0
    fix_num = address_len - len(address)
    while i < fix_num:
        address = "0"+address
        i = i+1
    return address


def do_restore(stack_list, symbols_dict, cpu):
    print("precess 30%...start restore")
    if cpu == "arm64":
        adrress_len = ARM64AddressLen
    else:
        adrress_len = ARM32AddressLen
    for item_json in stack_list:
        origininstruction_addr = item_json["instruction_addr"]
        instruction_addr = alignment_address(item_json["instruction_addr"], adrress_len)
        last_match_symbols = {}
        ""
        if item_json["object_name"].endswith("libflutter.so"):
            real_symbols_dict = symbols_dict["flutter"]
        else:
            real_symbols_dict = symbols_dict["app"]
        # 过滤匹配度最高的符号
        tem_symbols_dict = real_symbols_dict
        for index in range(adrress_len - len(origininstruction_addr)+1, len(instruction_addr)):
            # symbol_key 是 符号对应的地址
            last_symbol_key = ""
            match_symbols = {}
            not_match_flag = 0
            for symbol_key in tem_symbols_dict.keys():
                if symbol_key[0:index] == instruction_addr[0:index]:
                    # 把匹配的符号中的前一个符号也加上
                    if not_match_flag == 0:
                        not_match_flag = 1
                        match_symbols[last_symbol_key] = real_symbols_dict[symbol_key]
                    match_symbols[symbol_key] = real_symbols_dict[symbol_key]
                else:
                    # 前面已经匹配到了，因为是升序排序过的，所以后面也不会有匹配成功的了
                    if not_match_flag == 1:
                        break
                last_symbol_key = symbol_key
            #
            if len(match_symbols) == 0:
                break
            last_match_symbols = match_symbols
            tem_symbols_dict = match_symbols

        #选定符号
        last_symbol_key = "null"
        for symbol_key in last_match_symbols.keys():
            instruction_addr_int = int("0x"+instruction_addr, 16)
            symbol_address_int = int("0x"+symbol_key, 16)
            if instruction_addr_int < symbol_address_int:
                item_json["symbol_name"] = last_match_symbols[last_symbol_key]
                item_json["symbol_addr"] = last_symbol_key
                break
            last_symbol_key = symbol_key
    print("precess 90%...end restore")
    return stack_list


def start_restore(input_file_url, last_symbol_key):
    # 根据version检查libapp_so_sym.txt是否存在，到http://ftp.xinyu100.com/YesDesktop/702/libapp_so_sym.txt
    # 读取堆栈文件
    stack_json_obj = read_stack_file(input_file_url)
    app_version = stack_json_obj["app_version"]
    flutter_version = stack_json_obj["flutter_version"]
    cpu = stack_json_obj["cpu"]
    load_symbols(flutter_version, app_version, cpu)
    stack_list = stack_json_obj["stack_list"]
    # 解析
    stack_list = do_restore(stack_list, SymbolsDict, cpu)
    stack_json_obj["stack_list"] = stack_list
    # 输出文件
    write_stack_file(stack_json_obj, last_symbol_key)


# Press the green button in the gutter to run the script.
# test url = https://raw.githubusercontent.com/RickyJun/notes/main/test/stack_file_121.json?token=GHSAT0AAAAAABVR4SP5FWON5KSWQBGKUKH2YVKZH4Q
if __name__ == '__main__':
    argName0 = "--url="
    argName1 = "--ouput_file_path="
    urlArg = sys.argv[1]
    if not urlArg or not urlArg.startswith(argName0):
        print("bitch!!!!!, you have to set --url just like --url=\"https://raw.githubusercontent.com/filename\"")
    else:
        url = urlArg[len(argName0):len(urlArg)]
        output_file_dir = ""
        if len(sys.argv) >= 3:
            output_file_dir = sys.argv[2]
        if output_file_dir.startswith(argName1):
            output_file_dir = output_file_dir[len(argName1):len(output_file_dir)]
            if output_file_dir.startswith("."):
                output_file_dir = "{0}{1}".format(os.path.abspath('.'), output_file_dir[1:len(output_file_dir)])
        else:
            output_file_dir = "{0}/ouput".format(os.path.abspath('.'))
        start_restore(url, output_file_dir)
