#ifndef __ERROR_H__
#define __ERROR_H__

#include <vector>

#define ERROR_A  1   // �Ƿ���̖�򲻷����~��
#define ERROR_B  2   // �����ض��x
#define ERROR_C  3   // δ���x������
#define ERROR_D  4   // ��������������ƥ��
#define ERROR_E  5   // ����������Ͳ�ƥ��
#define ERROR_F  6   // �l���Д��г��F���Ϸ������
#define ERROR_G  7   // �o����ֵ�ĺ������ڲ�ƥ���return�Z��
#define ERROR_H  8   // �з���ֵ�ĺ���ȱ��return�Z�����ڲ�ƥ���return�Z��
#define ERROR_I  9   // ���MԪ�ص���ֻ�������ͱ��_ʽ
#define ERROR_J  10  // ���ܸ�׃������ֵ
#define ERROR_K  11  // ������̖
#define ERROR_L  12  // ������С��̖
#define ERROR_M  13  // ����������̖
#define ERROR_N  14  // ���M��ʼ��������ƥ��
#define ERROR_O  15  // <����>��Ͳ�һ��
#define ERROR_P  16  // ȱ��ȱʡ�Z��
#define OTHER_ERROR 17  // �����e�`���

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
	static bool hasError();
};

void error(const char* str);
void error();

#endif // !__ERROR_H__
