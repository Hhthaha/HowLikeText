#include"HowLikeText.h"
#include<fstream>
//求语意相似度的
string HowLike::GBKToUTF8(string s)
{
	int len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), -1, NULL, 0);//最后一个参数置0返回转成的UTF8所需的空间大小，倒数第三个设置为-1表示s里面所有的字符都转，第三个参数是将要转换的字符串，最后一个设为0表示返回需要的地址的大小
	wchar_t* wch = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), -1, wch, len);//转到了wch里面，从GBK转成UTF16，将当前字符编码转化为UTF16的
	len = WideCharToMultiByte(CP_UTF8, 0, wch, -1, NULL, 0, NULL, NULL);//将UTF16的转为UTF8 的
	char * ch = new char[len];
	WideCharToMultiByte(CP_UTF8, 0, wch, -1, ch, len, NULL, NULL);//从UTF16转成UTF8
	string c = ch;
	if (wch)
	{
		delete[] wch;
		wch = nullptr;
	}
	if (ch)
	{
		delete[] ch;
		ch = nullptr;
	}
	return c;
}


HowLike::HowLike(string dict)
	: DICT(dict)//传入jieba的dict根目录
	, DICT_PATH(DICT+"/jieba.dict.utf8")
	, HMM_PATH(DICT + "/hmm_model.utf8")
	, USER_DICT_PATH(DICT +"/user.dict.utf8" )
	, IDF_PATH(DICT +"/idf.utf8" )
	, STOP_WORD_PATH(DICT +"/stop_words.utf8" )
	, _maxWordNumber(100)  //这个值理论上越大最后的值越准确，这个是取多少个放在set里面，多次实验得出这个值基本上是最好的。再大需要的时间就久了
	, _jieba(DICT_PATH,
	         HMM_PATH,
	         USER_DICT_PATH,
	         IDF_PATH,
	         STOP_WORD_PATH)
			 
 {
	GetStopWord(STOP_WORD_PATH.c_str());
 }
void HowLike::GetStopWord(const char* stopWordFile)
{
	ifstream file2(stopWordFile);
	if (!file2.is_open())
	{
		cout << "file open is " << "failed" << endl;
		return;
	}
	string line;
	while (!file2.eof())
	{
		getline(file2, line);
		_stopWordSet.insert(line);//把停用词放到了_stopWordSet里面 停用词都是语气词，对语意的影响几乎没有 
	}
	file2.close();
}
WordMap HowLike::getWordMap(const char* file)
{
	ifstream file1(file);
	if (!file1.is_open())//判断文件是否打开
	{
		cout << "file open is " << "failed" << endl;
		return  WordMap();
	}
	string line;//存储每一行的文字
	WordMap wm;//存储词和词频
	while (!file1.eof())//循环条件：文件没有读到末尾
	{
		getline(file1, line);//获取每一行的词。放到line里。
		vector<string>  words;
		line = GBKToUTF8(line);
		_jieba.Cut(line, words, true);//用jieba分词
		//统计词频
		for (const auto& e : words)
		{
			if (_stopWordSet.count(e)>0)
				continue;
			else
			{
				if (wm.count(e)>0)
					wm[e]++;
				else
					wm[e] = 1;
			}
		}
	}
	file1.close();
	return wm;
}
bool mysort(pair<string, int>p1, pair<string, int>p2)
{
	return p1.second > p2.second;
}
vector<std::pair<std::string, int>> HowLike::Sort(WordMap& wm)
{
	vector<std::pair<std::string, int>> wwm(wm.begin(), wm.end());
	sort(wwm.begin(), wwm.end(),mysort);//sort排序列化的容器，map是（key，value）类型，转成vector
	return wwm;
}

void HowLike::ChoseWord(std::vector<std::pair<std::string, int>>& vc, wordSet& ws)
{
	int size = vc.size();
	int len = size > _maxWordNumber ? _maxWordNumber : size;
	for (int i = 0; i < len; i++)
	{
		ws.insert(vc[i].first);
	}
}


vector<double> HowLike::GetVectorQuantity(wordSet& ws, WordMap& wm)
{
	vector<double> vec;
	for (const auto& w : ws)
	{
		if (wm.count(w) > 0 )
		{
			vec.push_back(wm[w]);
		}
		else
		{
			vec.push_back(0);
		}
	}
	return vec;
}
double HowLike::cosine(std::vector<double> oneHot1, std::vector<double> oneHot2)
{
	assert(oneHot1.size() == oneHot2.size());
	int len = oneHot1.size();
	double dianji = 0;
	for (int i = 0; i < len; i++)
	{
	    double b = oneHot1[i] * oneHot2[i];
		dianji = dianji + b;
	}
	double mo1 = 0;
	double mo2 = 0;
	double sum1 = 0;
	double sum2 = 0;
	for (int j = 0; j < len; j++)
	{
		double c = pow(oneHot1[j], 2);
		sum1 = sum1 + c;
	}
	mo1 = pow(sum1, 0.5);
	for (int j = 0; j < len; j++)
	{
		double c = pow(oneHot2[j], 2);
		sum2 = sum2 + c;
	}
	mo2 = pow(sum2, 0.5);
	return  dianji / (mo1*mo2);
}
double HowLike::getHowLike(const char* file1, const char* file2)
{
	HowLike::GetStopWord(HowLike::STOP_WORD_PATH.c_str());//获取停用词，放到_stopWordSet里面，是unordered_set类型
	WordMap wm1 =HowLike::getWordMap(file1);
	WordMap wm2 = HowLike::getWordMap(file2);
	vector<pair<string, int>> vc1 = HowLike::Sort(wm1);
	vector<pair<string, int>> vc2 = HowLike::Sort(wm2);
	wordSet ws;
	HowLike::ChoseWord(vc1, ws);
	HowLike::ChoseWord(vc2, ws);
	vector<double> VQ1, VQ2;
	VQ1 = HowLike::GetVectorQuantity(ws, wm1);
	VQ2 = HowLike::GetVectorQuantity(ws, wm2);
	cout<<"两篇文章的文本相似度为："<< HowLike::cosine(VQ1, VQ2)*100<<"%"<<endl;
	return HowLike::cosine(VQ1, VQ2);
}
