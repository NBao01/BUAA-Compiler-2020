#ifndef __ERROR_H__
#define __ERROR_H__

#include <vector>

#define ERROR_A  1   // 非法符號或不符合詞法
#define ERROR_B  2   // 名字重定義
#define ERROR_C  3   // 未定義的名字
#define ERROR_D  4   // 函數參數個數不匹配
#define ERROR_E  5   // 函數參數類型不匹配
#define ERROR_F  6   // 條件判斷中出現不合法的類型
#define ERROR_G  7   // 無返回值的函數存在不匹配的return語句
#define ERROR_H  8   // 有返回值的函數缺少return語句或存在不匹配的return語句
#define ERROR_I  9   // 數組元素的下標只能是整型表達式
#define ERROR_J  10  // 不能改變常量的值
#define ERROR_K  11  // 應爲分號
#define ERROR_L  12  // 應爲右小括號
#define ERROR_M  13  // 應爲右中括號
#define ERROR_N  14  // 數組初始化個數不匹配
#define ERROR_O  15  // <常量>類型不一致
#define ERROR_P  16  // 缺少缺省語句
#define OTHER_ERROR 17  // 其他錯誤類型

class ErrorItem {
private:
	int type;
	int line;
public:
	ErrorItem(int type, int line);
	int getType();
	int getLine();
};

class ErrorHandler {
public:
	static void addErrorItem(ErrorItem* e);
	static void addErrorItem(int type, int line);
	static void output();
};

void error(const char* str);
void error();

#endif // !__ERROR_H__
