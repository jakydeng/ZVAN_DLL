#ifndef __ZVAN_VIA_SDK__
#define __ZVAN_VIA_SDK__

//位置，相对位置为%的值
struct Zvan_Point {
	char x; //0-100%
	char y; //0-100%
};



//矩形框，相对位置为%的值
struct Zvan_Rect {
	int x; //0-100%
	int y; //0-100%
	int w; //0-100%
	int h; //0-100%
};

enum Zvan_AnalysisType {
	AnalysisType_None = 0,	//无效
	AnalysisType_Color,   //污水烟雾颜色检测   PS：颜色检测Type改为分两个type，一个污水和一个烟雾
	AnalysisType_Height,	//液体高度检测      PS: 客户更改方式，改为放一个标识物检测水位
	AnalysisType_Object,	//物体形态检测
	AnalysisType_Activity,  //活动检测
	AnalysisType_Work,		//污水烟囱是否排放检测   PS：排放检测Type改为分两个type，一个污水和一个烟雾
	AnalysisType_Turbidity,    //浊度检测
};

enum Zvan_SourceType{
	SourceType_Water,	//水检测
	SourceType_Air,		//气检测
};


#define MAXANALYSISLINENUM	3
#define MAXFILENAME			255
#define MAXIMAGENUM			5
#define MAXANALYSISINFONUM	16


///////////////////////////////
#define  ANALYZE_TYPE_WATER_COLOR 1
#define  ANALYZE_TYPE_WATER_HEIGHT 2


struct Zvan_AnalysisInfo {
	Zvan_Rect			rect;	//区域位置
	Zvan_SourceType		source;	//分析源类型
	Zvan_AnalysisType	type;	//分析类型
	union {
		int			color;	//输出，颜色分析结果,结果值为0xRRGGBB,
		float       ratio;  //输出液位高度比例，  
		struct {
			bool		cover;	//输出，是否排放覆盖线条   //reserve
		}line[MAXANALYSISLINENUM];

		struct {			//物体形态检测
			bool	froth;	//输出，是否存在泡沫
			int		color;	//输出，物体颜色
			int     threshold;//阈值，像素个数
			int     pixel;//阈值，像素个数
			float     pixelpct; //输出，变化像素的百分比

		}object;

		bool		activity;//输出，活动异常检测，是否存在活动
		bool		work;	//输出，是否排放检测
		bool		turbidity;//输出，浊度检测
	}param;
};


//分析输入参数
struct Zvan_Analysis_Params {
	char images[MAXIMAGENUM][MAXFILENAME];	//输入图片，最大MAXIMAGENUM张，为空表示无效，图片文件路径
	Zvan_AnalysisInfo infos[MAXANALYSISINFONUM]; //需要分析的结构体，type=0表示无效,最大单次支持MAXANALYSISINFONUM种检测
};

/****************************************
* @brief 初始化智能分析模块
* @return true -- 初始化结果，true：成功，false：失败
****************************************/
bool Zvan_VIA_Init();

/****************************************
* @brief 获取模块版本号
* @return const char* -- 获取模块版本号，版本号格式：1.0.1
****************************************/
const char* Zvan_VIA_Version();

/****************************************
* @brief 反初始化智能分析模块
* @return true -- 反初始化结果，true：成功，false：失败
****************************************/
bool Zvan_VIA_CleanUp();

/****************************************
* @brief 当接口调用失败时，通过该接口获取接口失败错误描述
* @return const char* -- 获取错后一次错误描述，错误信息应按线程进行隔离
****************************************/
const char* Zvan_VIA_GetLastErrorMsg();

/****************************************
* @brief 视频智能分析
* @param Zvan_Analysis_Params 视频分析参数和结果
* @return bool 返回视频智能分析接口
* @remark 该接口支持多线程，并发调用个数不受限，
****************************************/
//_declspec(dllexport)  bool Zvan_VIA_Analysis(Zvan_Analysis_Params* param);


#endif // !__ZVAN_VIA_SDK__

