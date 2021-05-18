#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<windows.h>//�̺߳�����Ҫ����ϵͳ�ĺ����� 
#include<process.h>//�̺߳���ͷ�ļ��� 

#define SIXTEEN 16
#define EIGHT 8
#define X1 129
#define X2 257

typedef struct message{
	char filename[32];//�ļ����� 
	int address;//��ȡָ��ĳ�ʼ��ַ�� 
	int id;//�߳�id �� 
}	Message;

typedef struct systemRegister{
	int proCount;//����������� 
	int instruRegister;//ָ��Ĵ����� 
	int signRegister;//��־�Ĵ����� 
}   SystemReg;
typedef SystemReg * SystemReg_PTR;

typedef struct genrealRegister{
	int id;//���߳�id�� 
	int dataReg[4];//���ݼĴ����� 
	int ptrReg[4];//��ַ�Ĵ����� 
}	GPR;
typedef GPR * GPR_PTR;

int codeMemory[X1]={0},dataMemory[X2]={0}; //����κ����ݶγ�ֵΪ0�� 
HANDLE hMutex,hMutex2;//���廥�����ľ���� 

/*�������ܣ���ΪFunProc���̺߳�����ʵ��ĳ�߳���ָ����桢���ú������к�������*/
unsigned __stdcall FunProc(void *pArguments);
/*�������ܣ���ȡ�����룬��ת����ʮ���ƣ�*/ 
int get_operationCode(char *ch,int i,int operCode);
/*�������ܣ���ȡ����������ת���ɳ�ʮ���ƣ�*/
int get_immediateValue(char *ch,int i,int immeValue);
/*�������ܣ���ȡÿһ��ָ���Ӧ�Ĵ���Σ�*/ 
int get_codeMemory(char *ch,int i,int codeMemory);
/*�������ܣ���ȡir��ֵ��*/ 
int get_ir(char *ch,int i,int ir);
/*�������ܣ�����ָ���������Ӧ������ɺ�������*/
void analyzeInstruction(int operCode,int immeValue,GPR_PTR gprPtr,
	SystemReg_PTR sysReg_ptr,char *ch);
/*�������ܣ��������򲢴�ӡ����κ����ݶΣ�*/ 
void exitPrint();
/*�������ܣ���ӡ�����Ĵ�����״̬��*/ 
void print(SystemReg_PTR sysReg_ptr,GPR_PTR gprPtr);
/*�������ܣ����ݴ���*/
void dataPass(GPR_PTR gprPtr,int immeValue,int data,int ptr);
/*�������ܣ�ʵ���������߼�����*/
void OPERATION(int operCode,GPR_PTR gprPtr,int immeValue,int data,int ptr);
/*�������ܣ��Ƚ�ָ���1�����ݼĴ������������Ƚϣ�2�����ݼĴ������ַָ���
	���ݱȽϣ������޸ı�־�Ĵ�����ֵ��*/ 
void COMP(GPR_PTR gprPtr,int immeValue,SystemReg_PTR sysReg_ptr
			,int data,int ptr);
/*�������ܣ�ʵ���������ָ�������˿�����һ�������浽���ݼĴ�����*/
void input_or_output(int operCode,GPR_PTR gprPtr,int data);
/*�������ܣ�ʵ����תָ���1����������תָ�2�����ݱ�־�Ĵ������ݣ��ж��Ƿ���ת����ִ�У�*/
void jumpInstruction(int immeValue,char *ch,SystemReg_PTR sysReg_ptr); 
/*�������ܣ����󻥳����������ס�������ƶ����ڴ棻*/
void lockMemory(int immeValue);
/*�������ܣ��ͷŻ�����������ͷ���ס��������ָ���ڴ�Ļ������*/
void unlockMemory(int immeValue); 
/*�������ܣ��������������룻*/
void dormancy(int immeValue); 

unsigned __stdcall FunProc(void *pArguments)
{
	Message *ptr;
	FILE *fPtr;
	ptr=(Message *)pArguments;//��ȡ�����̵߳Ĳ���ָ�루�ṹ�壩�� 
	SystemReg sysReg={0,0,0};//����ϵͳ�Ĵ���������ʼ��Ϊ0�� 
	GPR gpr={0,{0,0,0,0},{0,0,0,0}};//���ݼĴ����͵�ַ�Ĵ�����ֵΪ0�� 
	char ch[33],inMemory[4096][33];//���崢��ָ����ڴ棻 
	int operCode=0,immeValue=0,n=0;//��������롢�������ȣ� 
	gpr.id=ptr->id;//ȷ���Ĵ���id�� 
	sysReg.proCount=ptr->address*4;//ȷ�������������ʼֵ������2��256��ʼ���� 
	if((fPtr=fopen(ptr->filename,"r"))!=NULL){
		fgets(ch,sizeof(ch),fPtr);//��ȡһ��ָ� 
		while((n=fgetc(fPtr))!=-1){
			codeMemory[ptr->address]=get_codeMemory(ch,0,0);//�洢����Σ�
			strcpy(inMemory[ptr->address],ch);ptr->address++;//��ָ������ڴ棻
			fgets(ch,sizeof(ch),fPtr);//��ȡ��һ��ָ�
	    }fclose(fPtr);//�ر��ļ��� 
	}else printf("The file can't be opened.\n");
	while(1){
		strcpy(ch,inMemory[sysReg.proCount/4]);//��ȡ���������ָ���ָ�
		sysReg.proCount+=4;//ϵͳ�Ĵ����ڵĳ��������ָ����һ��ָ� 
		sysReg.instruRegister=get_ir(ch,0,0);//ָ��Ĵ�����ֵ�� 
		operCode=get_operationCode(ch,0,0);//��ȡ�����룻 
		immeValue=get_immediateValue(ch,16,0);//��ȡ�������� 
		analyzeInstruction(operCode,immeValue,&gpr,&sysReg,ch);//����ָ������ú�������������	
		print(&sysReg,&gpr);//��ӡ�Ĵ���״̬�� 
	} 
}

int main(int argc,char *argv[])
{
	dataMemory[0]=100;//��ʼ�����ݶ�16384���ڴ�Ϊ100�� 
	HANDLE hThread1,hThread2;//����� 
	unsigned ThreadId1=1,ThreadId2=2;//�̵߳ı�ʶ�ţ� 
	Message message1,message2;//����ṹ�壬�Ա㴫����̣߳� 
	strcpy(message1.filename,"dict1.dic");//�̴߳򿪵��ļ����� 
	strcpy(message2.filename,"dict2.dic");
	message1.id=1;message2.id=2; //������̵߳��߳�id�� 
	message1.address=0;//ȡָ��ʱ���̴߳������׵�ַ�� 
	message2.address=256/4;
	hMutex=CreateMutex(NULL,FALSE,NULL);//������ס�ڴ�Ļ������ 
	hMutex2=CreateMutex(NULL,FALSE,NULL);//������ס����Ļ������ 
	hThread1=(HANDLE)_beginthreadex(NULL,0,FunProc,(void *)&message1,0,&ThreadId1);
	hThread2=(HANDLE)_beginthreadex(NULL,0,FunProc,(void *)&message2,0,&ThreadId2); 
	WaitForSingleObject(hThread1,INFINITE);
	CloseHandle(hThread1);
	WaitForSingleObject(hThread2,INFINITE);
	CloseHandle(hThread2);
	CloseHandle(hMutex);CloseHandle(hMutex2);//ɾ���������ľ���� 
	exitPrint();//��ӡ���ݶκʹ���Σ� 
	return 0;
}

int get_ir(char *ch,int i,int ir)
{
	for(;i<15;i++)
		ir=2*(ir+ch[i]-'0');
	ir=ir+ch[i]-'0';
	return ir;
}

int get_codeMemory(char *ch,int i,int codeMemory)
{
	for(;i<31;i++)
		codeMemory=2*(codeMemory+(ch[i]-'0'));
	codeMemory=codeMemory+ch[i]-'0';
	return codeMemory;
}

int get_operationCode(char *ch,int i,int operCode)
{
	for(;i<7;i++)
		operCode=2*(operCode+(ch[i]-'0'));
	operCode=operCode+ch[i]-'0';
	return operCode;
}

int get_immediateValue(char *ch,int i,int immeValue)
{
	for(;i<31;i++)
		immeValue=2*(immeValue+ch[i]-'0');
	immeValue=immeValue+ch[i]-'0';
	return (short) immeValue;
}

void dataPass(GPR_PTR gprPtr,int immeValue,int data,int ptr)
{
	if(ptr==0){
		if(data<=4) 
			gprPtr->dataReg[data-1]=immeValue;//���������������ݼĴ����� 
		else if(data>=5) 
			gprPtr->ptrReg[data-5]=immeValue;//�������������ַ�Ĵ����� 
	}else if(data<ptr)//����ַ�Ĵ�����ָ�ڴ�������ݼĴ����� 
		gprPtr->dataReg[data-1]=dataMemory[((gprPtr->ptrReg[ptr-5])-16384)/2];
	else if(data>ptr)//�����ݼĴ���ֵ�����ַ�Ĵ�����ָ�ڴ棻 
		dataMemory[(gprPtr->ptrReg[data-5]-16384)/2]=gprPtr->dataReg[ptr-1];
}

void OPERATION(int operCode,GPR_PTR gprPtr,int immeValue,int data,int ptr)
{
	if(operCode==2&&ptr==0)//���ݼĴ���+����������ԭ���ݼĴ����� 
		gprPtr->dataReg[data-1]+=immeValue;
	else if(operCode==2&&data<ptr)//���ݼĴ���+��ַ�Ĵ�����ָ�ڴ棬����ԭ���ݼĴ����� 
		gprPtr->dataReg[data-1]+=dataMemory[(gprPtr->ptrReg[ptr-5]-16384)/2];
	if(operCode==3&&ptr==0)//���ݼĴ���-����������ԭ���ݼĴ����� 
		gprPtr->dataReg[data-1]-=immeValue;
	else if(operCode==3&&data<ptr)//���ݼĴ���-��ַ�Ĵ�����ָ�ڴ棬����ԭ���ݼĴ����� 
		gprPtr->dataReg[data-1]-=dataMemory[(gprPtr->ptrReg[ptr-5]-16384)/2];
	if(operCode==4&&ptr==0)//���ݼĴ���*����������ԭ���ݼĴ����� 
		gprPtr->dataReg[data-1]*=immeValue;
	else if(operCode==4&&data<ptr)//���ݼĴ���*��ַ�Ĵ�����ָ�ڴ棬����ԭ���ݼĴ����� 
		gprPtr->dataReg[data-1]*=dataMemory[(gprPtr->ptrReg[ptr-5]-16384)/2];
	if(operCode==5&&ptr==0)//���ݼĴ���/����������ԭ���ݼĴ����� 
		gprPtr->dataReg[data-1]/=immeValue;
	else if(operCode==5&&data<ptr)//���ݼĴ���/��ַ�Ĵ�����ָ�ڴ棬����ԭ���ݼĴ����� 
		gprPtr->dataReg[data-1]/=dataMemory[(gprPtr->ptrReg[ptr-5]-16384)/2];
	if(operCode==6&&ptr==0)//���ݼĴ��������������������ԭ���ݼĴ����� 
		gprPtr->dataReg[data-1]=gprPtr->dataReg[data-1]&&immeValue;
	else if(operCode==6&&data<ptr)//���ݼĴ�������ַ�Ĵ�����ָ�ڴ������㣬����ԭ���ݼĴ����� 
		(gprPtr->dataReg[data-1])=(gprPtr->dataReg[data-1])&&(dataMemory[(gprPtr->ptrReg[ptr-5]-16384)/2]);
	if(operCode==7&&ptr==0)//���ݼĴ��������������������ԭ���ݼĴ�����
		gprPtr->dataReg[data-1]=gprPtr->dataReg[data-1]||immeValue;
	else if(operCode==7&&data<ptr)//���ݼĴ�������ַ�Ĵ�����ָ�ڴ�����㣬����ԭ���ݼĴ�����
		gprPtr->dataReg[data-1]=(gprPtr->dataReg[data-1])||(dataMemory[(gprPtr->ptrReg[ptr-5]-16384)/2]);
	if(operCode==8&&data!=0)//���ݼĴ��������㣬����ԭ���ݼĴ����� 
		gprPtr->dataReg[data-1]=!(gprPtr->dataReg[data-1]);
	else if(operCode==8&&data==0)//ָ��Ĵ�����ָ�ڴ�����㣬������ڴ棻 
		dataMemory[(gprPtr->ptrReg[ptr-5]-16384)/2]=!dataMemory[(gprPtr->ptrReg[ptr-5]-16384)/2];
}

void COMP(GPR_PTR gprPtr,int immeValue,SystemReg_PTR sysReg_ptr
			,int data,int ptr)
{
	if(ptr==0){//���ݼĴ������������Ƚϣ� 
		if(gprPtr->dataReg[data-1]==immeValue)
			sysReg_ptr->signRegister=0;
		else if(gprPtr->dataReg[data-1]>immeValue)
			sysReg_ptr->signRegister=1;
		else if(gprPtr->dataReg[data-1]<immeValue)
			sysReg_ptr->signRegister=-1;
	}else{//��ַ�Ĵ�����ָ�ڴ������ݼĴ����ıȽϣ� 
		if(dataMemory[(gprPtr->ptrReg[ptr-5]-16384)/2]==gprPtr->dataReg[data-1])
			sysReg_ptr->signRegister=0;
		else if(dataMemory[(gprPtr->ptrReg[ptr-5]-16384)/2]<gprPtr->dataReg[data-1])
			sysReg_ptr->signRegister=1;
		else if(dataMemory[(gprPtr->ptrReg[ptr-5]-16384)/2]>gprPtr->dataReg[data-1])
			sysReg_ptr->signRegister=-1;
	}
}

void jumpInstruction(int immeValue,char *ch,SystemReg_PTR sysReg_ptr)
{
	int sign=0;
	int i;
	for(i=EIGHT;i<SIXTEEN-1;i++)
		sign=2*(sign+ch[i]-'0');
	sign=sign+ch[i]-'0';
	if(sign==0||(sign==1&&sysReg_ptr->signRegister==0)||(sign==2&&sysReg_ptr->signRegister==1)|| 
	(sign==3&&sysReg_ptr->signRegister==-1))//��������ת;��־�Ĵ���Ϊ0ʱ��ת;��־�Ĵ���Ϊ1ʱ��ת;��־�Ĵ���Ϊ-1ʱ��ת��  
		sysReg_ptr->proCount+=(immeValue-4);
}

void input_or_output(int operCode,GPR_PTR gprPtr,int data)
{
	if(operCode==12)
		printf("id = %d    out: %d\n",gprPtr->id,gprPtr->dataReg[data-1]);//����߳�id�����ݼĴ����ڵ�ֵ��
	else{printf("in:\n");
	scanf("%d",&gprPtr->dataReg[data-1]);//�����ֵ�������ݼĴ����� 
	}
}

void lockMemory(int immeValue)
{
	hMutex=CreateMutex(NULL,FALSE,"dataMemory[(immeValue-16384)/2]");//�������Ĵ����� 
	WaitForSingleObject(hMutex,INFINITE);//������������ 
}

void unlockMemory(int immeValue)
{
	hMutex=CreateMutex(NULL,FALSE,"dataMemory[(immeValue-16384)/2]");
	ReleaseMutex(hMutex);//���������ͷţ� 
}

void dormancy(int immeValue)
{
	Sleep(immeValue);//���ߣ� 
}

void analyzeInstruction(int operCode,int immeValue,GPR_PTR gprPtr,
	SystemReg_PTR sysReg_ptr,char *ch)
{
	int data=0,ptr=0,i=EIGHT,j=12;
	//�������������� 
	for(i=EIGHT;i<11;i++)
		data=2*(data+ch[i]-'0');
	data=data+ch[i]-'0'; 
	for(j=12;j<SIXTEEN-1;j++)
		ptr=2*(ptr+ch[j]-'0');
	ptr=ptr+ch[j]-'0';
	if(operCode==0){//ͣ��ָ� 
		print(sysReg_ptr,gprPtr);
		_endthreadex(0);//�����̣߳� 
	}
	if(operCode==1)//���ݴ���ָ� 
		dataPass(gprPtr,immeValue,data,ptr);
	if(operCode>=2&&operCode<=8)//�������߼����㣻 
		OPERATION(operCode,gprPtr,immeValue,data,ptr);
	if(operCode==9)//�Ƚ�ָ� 
		COMP(gprPtr,immeValue,sysReg_ptr,data,ptr);
	if(operCode==10)//��תָ� 
		jumpInstruction(immeValue,ch,sysReg_ptr); 
	if(operCode==11||operCode==12)//�������ָ� 
		input_or_output(operCode,gprPtr,data);
	if(operCode==13)
		lockMemory(immeValue);//�ӻ�����ָ� 
	if(operCode==14)
		unlockMemory(immeValue);//�ͷŻ�����ָ� 
	if(operCode==15)
		dormancy(immeValue);//����ָ� 
}

void exitPrint()
{
	int i;
	printf("\ncodeSegment :\n");
	for(i=1;i<X1;i++){ //�������Σ� 
		if(i%EIGHT!=0) printf("%d ",codeMemory[i-1]);
		else printf("%d\n",codeMemory[i-1]);
	}printf("\ndataSegment :\n");
	for(i=1;i<X2;i++){ //������ݶΣ� 
		if(i%SIXTEEN!=0) printf("%d ",dataMemory[i-1]);
		else printf("%d\n",dataMemory[i-1]);
	}exit(0);//�������� 
}

void print(SystemReg_PTR sysReg_ptr,GPR_PTR gprPtr)
{//���11���Ĵ�����״̬��
	WaitForSingleObject(hMutex2,INFINITE);//��֤��������� 
	printf("id = %d\n",gprPtr->id); 
	printf("ip = %d\n",sysReg_ptr->proCount);
	printf("flag = %d\n",sysReg_ptr->signRegister);
	printf("ir = %d\n",sysReg_ptr->instruRegister);
	printf("ax1 = %d ax2 = %d ax3 = %d ax4 = %d\n",gprPtr->dataReg[0],gprPtr->dataReg[1],gprPtr->dataReg[2],gprPtr->dataReg[3]);
	printf("ax5 = %d ax6 = %d ax7 = %d ax8 = %d\n",gprPtr->ptrReg[0],gprPtr->ptrReg[1],gprPtr->ptrReg[2],gprPtr->ptrReg[3]);
	ReleaseMutex(hMutex2);//�ͷŻ������ 
}
