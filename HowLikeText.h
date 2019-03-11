#define _CRT_SECURE_NO_WARNINGS
#pragma once
#include <unordered_map>
#include <string>
#include <unordered_set>
#include<Windows.h>
#include<wchar.h>
#include <cppjieba/jieba.hpp>
#include<iostream>
#include<assert.h>
#include<math.h>
using namespace std;
typedef std::unordered_map<std::string, int> WordMap;
	typedef std::unordered_set<std::string> wordSet;
class HowLike
{
private:

	void GetStopWord(const char* stopWordFile);//获取停用词
	WordMap getWordMap(const char* file);//求词频键值对
	std::string UTF8ToGBK(std::string str);
	std::string GBKToUTF8(std::string str);
	std::vector<std::pair<std::string, int>> Sort(WordMap& wm);///排序，获取词频从大到小排序的关键字
	void ChoseWord(std::vector<std::pair<std::string, int>>& vc, wordSet& ws);//将vc里存的关键词插入到ws里，用引用当数据太大传值太慢
	std::vector<double> GetVectorQuantity(wordSet& ws, WordMap& wm);//求关键词向量
	double cosine(std::vector<double> oneHot1, std::vector<double> oneHot2);//求相似度

	std::string DICT;
	std::string DICT_PATH;
	std::string HMM_PATH;
	std::string USER_DICT_PATH;
	std::string IDF_PATH;
	std::string STOP_WORD_PATH; //字符串为了传入使用Jieba分词的文件路径
	cppjieba::Jieba _jieba; 

	wordSet _stopWordSet;
	int _maxWordNumber;
public:
	HowLike(std::string dict);
	double getHowLike(const char* file1, const char* file2);//封装，对外只暴露一个接口供用户使用
};
