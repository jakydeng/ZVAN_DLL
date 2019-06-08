#ifndef __ZVAN_VIA_SDK__
#define __ZVAN_VIA_SDK__

//λ�ã����λ��Ϊ%��ֵ
struct Zvan_Point {
	char x; //0-100%
	char y; //0-100%
};



//���ο����λ��Ϊ%��ֵ
struct Zvan_Rect {
	int x; //0-100%
	int y; //0-100%
	int w; //0-100%
	int h; //0-100%
};

enum Zvan_AnalysisType {
	AnalysisType_None = 0,	//��Ч
	AnalysisType_Color,   //��ˮ������ɫ���   PS����ɫ���Type��Ϊ������type��һ����ˮ��һ������
	AnalysisType_Height,	//Һ��߶ȼ��      PS: �ͻ����ķ�ʽ����Ϊ��һ����ʶ����ˮλ
	AnalysisType_Object,	//������̬���
	AnalysisType_Activity,  //����
	AnalysisType_Work,		//��ˮ�̴��Ƿ��ŷż��   PS���ŷż��Type��Ϊ������type��һ����ˮ��һ������
	AnalysisType_Turbidity,    //�Ƕȼ��
};

enum Zvan_SourceType{
	SourceType_Water,	//ˮ���
	SourceType_Air,		//�����
};


#define MAXANALYSISLINENUM	3
#define MAXFILENAME			255
#define MAXIMAGENUM			5
#define MAXANALYSISINFONUM	16


///////////////////////////////
#define  ANALYZE_TYPE_WATER_COLOR 1
#define  ANALYZE_TYPE_WATER_HEIGHT 2


struct Zvan_AnalysisInfo {
	Zvan_Rect			rect;	//����λ��
	Zvan_SourceType		source;	//����Դ����
	Zvan_AnalysisType	type;	//��������
	union {
		int			color;	//�������ɫ�������,���ֵΪ0xRRGGBB,
		float       ratio;  //���Һλ�߶ȱ�����  
		struct {
			bool		cover;	//������Ƿ��ŷŸ�������   //reserve
		}line[MAXANALYSISLINENUM];

		struct {			//������̬���
			bool	froth;	//������Ƿ������ĭ
			int		color;	//�����������ɫ
			int     threshold;//��ֵ�����ظ���
			int     pixel;//��ֵ�����ظ���
			float     pixelpct; //������仯���صİٷֱ�

		}object;

		bool		activity;//�������쳣��⣬�Ƿ���ڻ
		bool		work;	//������Ƿ��ŷż��
		bool		turbidity;//������Ƕȼ��
	}param;
};


//�����������
struct Zvan_Analysis_Params {
	char images[MAXIMAGENUM][MAXFILENAME];	//����ͼƬ�����MAXIMAGENUM�ţ�Ϊ�ձ�ʾ��Ч��ͼƬ�ļ�·��
	Zvan_AnalysisInfo infos[MAXANALYSISINFONUM]; //��Ҫ�����Ľṹ�壬type=0��ʾ��Ч,��󵥴�֧��MAXANALYSISINFONUM�ּ��
};

/****************************************
* @brief ��ʼ�����ܷ���ģ��
* @return true -- ��ʼ�������true���ɹ���false��ʧ��
****************************************/
bool Zvan_VIA_Init();

/****************************************
* @brief ��ȡģ��汾��
* @return const char* -- ��ȡģ��汾�ţ��汾�Ÿ�ʽ��1.0.1
****************************************/
const char* Zvan_VIA_Version();

/****************************************
* @brief ����ʼ�����ܷ���ģ��
* @return true -- ����ʼ�������true���ɹ���false��ʧ��
****************************************/
bool Zvan_VIA_CleanUp();

/****************************************
* @brief ���ӿڵ���ʧ��ʱ��ͨ���ýӿڻ�ȡ�ӿ�ʧ�ܴ�������
* @return const char* -- ��ȡ���һ�δ���������������ϢӦ���߳̽��и���
****************************************/
const char* Zvan_VIA_GetLastErrorMsg();

/****************************************
* @brief ��Ƶ���ܷ���
* @param Zvan_Analysis_Params ��Ƶ���������ͽ��
* @return bool ������Ƶ���ܷ����ӿ�
* @remark �ýӿ�֧�ֶ��̣߳��������ø��������ޣ�
****************************************/
//_declspec(dllexport)  bool Zvan_VIA_Analysis(Zvan_Analysis_Params* param);


#endif // !__ZVAN_VIA_SDK__

