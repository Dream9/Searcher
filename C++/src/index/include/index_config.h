/*************************************************************************
	> File Name: config.h
	> Author: Dream9
	> Brief: 
	> Created Time: 2019年06月11日 星期二 14时27分23秒
 ************************************************************************/

#ifndef _INDEX_INCLUDE_CONFIG_H_
#define _INDEX_INCLUDE_CONFIG_H_

#include<array>

//版本号
#include"../../utils/include/formatfile_config.h"

//默认分词分割符号
const char kSEPARATE_CHAR = '/';

//反向索引文件分割符号
const char kSEPARATE_CHAR_INVERSE_FILE = ' ';

//制定外部排序的界限,即1G内存下最大元素数量
const unsigned int kMAX_SORT_LIMIT = 1024*1024*1024/sizeof(std::pair<std::array<char,33>,int>);

//按单字节计算，允许汉字串最大长度
const int kMAX_WORD_LENGTH = 8;


#endif
