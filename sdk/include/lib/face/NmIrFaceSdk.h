#ifndef __NM_FACE_SDK_H__
#define __NM_FACE_SDK_H__

typedef struct _RECT {
    long left, top, right, bottom;
} RECT, *LPRECT;

typedef struct _POINT {
    long x, y;
} POINT, *LPPOINT;

typedef unsigned char BYTE;
typedef void* HANDLE;

#ifdef EXPORTS
  #define DLL_FUNC  __attribute__((visibility("default")))
#else
  #define DLL_FUNC
#endif

#ifdef __cplusplus
extern "C" {
#endif


// SDK初始化
// 输入参数： 无
// 输出参数： 无
// 返回值：成功返回0，许可无效返回-1，算法初始化失败返回-2。
// 备注：任何接口都必须在SDK初始化成功后才能调用。
DLL_FUNC int NMSDK_IRFACE_Init();


// SDK反初始化
// 输入参数： 无
// 输出参数： 无
// 返回值：无
// 备注：必须在初始化成功后调用；软件退出前必须调用本接口释放资源；反初始化后不能再调用除初始化外的任何接口。
DLL_FUNC void NMSDK_IRFACE_Uninit();

// 检测人脸
// 输入参数：
//   pGray ---- 图象的亮度/灰度/Y值数据（每象素1字节）
//   nWidth ---- 图象数据宽度（象素单位）
//   nHeight ---- 图象数据高度（象素单位）
// 输出参数：
//   lprcFace ---- 输出检测到的人脸坐标
//   lpptLeftEye ---- 输出检测到的左眼坐标
//   lpptRightEye ---- 输出检测到的右眼坐标
// 返回值：返回1表示检测到人脸，0表示无人脸，-1表示检测失败
// 备注：必须初始化成功后才能调用； 必须返回1时才会输出人脸及眼睛坐标。
DLL_FUNC int NMSDK_IRFACE_Detect(BYTE* pGray, int nWidth, int nHeight, LPRECT lprcFace, LPPOINT lpptLeftEye, LPPOINT lpptRightEye);

// 获取特征码大小
// 输入参数： 无
// 输出参数： 无
// 返回值：特征码大小。
// 备注：必须初始化成功后才能调用。
DLL_FUNC int NMSDK_IRFACE_FeatureSize();

// 提取特征码
// 输入参数：
//   pGray ---- 图象的亮度/灰度/Y值数据（每象素1字节）
//   nWidth ---- 图象数据宽度
//   nHeight ---- 图象数据高度
//   ptLeftEye ---- 左眼坐标
//   ptRightEye ---- 右眼坐标
// 输出参数：
//   pFeature ---- 输出提取的人脸特征码（调用前必须分配有效的空间）
// 返回值：返回0表示成功，返回-1表示失败
// 备注：必须初始化成功才能调用； 必须传入有效的图象及检测到的人眼坐标才能提取到有效的特征码。
DLL_FUNC int NMSDK_IRFACE_Feature(BYTE* pGray, int nWidth, int nHeight, POINT ptLeftEye, POINT ptRightEye, BYTE* pFeature);

// 一对一比对
// 输入参数：
//   pFeature1 ---- 第1张人脸的特征
//   pFeature2 ---- 第2张人脸的特征
// 输出参数：无
// 返回值：返回两个人脸特征对应的人脸的相似度（0-100）
// 备注：必须初始化成功才能调用。
DLL_FUNC BYTE NMSDK_IRFACE_Compare(BYTE* pFeature1, BYTE* pFeature2);


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// 以下为一对多比对接口
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// 创建一对多特征比对列表
// 输入参数：
//   nMaxFeatureNum ---- 比对列表需容纳的最大特征个数
// 输出参数：无
// 返回值：成功返回列表句柄，失败返回NULL
// 备注：必须初始化成功才能调用; 需容纳的特征个数越大，消耗的内存也越大；可容纳的最大特征个数受许可限制。
DLL_FUNC HANDLE NMSDK_IRFACE_CreateFeatureList(int nMaxFeatureNum);

// 向特征比对列表中加入模板库
// 输入参数：
//   hList ---- 创建的特征比对列表的句柄
//   pFeatures ---- 要加入的模板库的特征码
//   nFeatures ---- 要加入的模板库的特征个数
// 输出参数：无
// 返回值：返回特征比对列表中当前的模板总数
// 备注：每次调用可加入单个模板库，也可加入多个模板库，多个模板库的特征码必须是连续内存并且按特征码大小对齐； 新加入的模板库按顺序插入原模板库的后面。
DLL_FUNC int NMSDK_IRFACE_AddToFeatureList(HANDLE hList, unsigned char* pFeatures, int nFeatures);

// 删除特征比对列表中的所有模板
// 输入参数：
//   hList ---- 创建的特征比对列表的句柄
// 输出参数：无
// 返回值：无
// 备注：执行后模板库中所有模板被清除，可重新加入模板。
DLL_FUNC void NMSDK_IRFACE_ClearFeatureList(HANDLE hList);

// 一对多特征比对
// 输入参数：
//   hList ---- 创建的特征比对列表的句柄
//   pFeature ---- 要进行一对多比对的特征码
//   nPos ---- 要开始比对的位置码（0表示从头开始，n表示越过n个特征从第n+1个特征开始比对）
//   nCompareNum ---- 从nPos开始比对的特征个数（<1或大于模板库总数则比对nPos位置以后的全部模板）
// 输出参数：
//   pnScores ---- 输出与每个模板的比对分数，每个模板占一字节，其值表示相似度，范围为0-100；必须分配足够的内存
// 返回值：返回实际比对的模板个数，也是输出相似度的个数
// 备注：nPos及nCompareNum均为0或-1时表示与特征比对列表中的所有模板库进行比对； 指定合适的nPost和nCompareNum可实现对某用户（多个模板）实现一对一比对。
DLL_FUNC int NMSDK_IRFACE_CompareFeatureList(HANDLE hList, BYTE* pFeature, int nPosBegin, int nCompareNum, BYTE* pnScores);

// 销毁一对多特征比对列表
// 输入参数：
//   hList ---- 创建的特征比对列表的句柄
// 输出参数：无
// 返回值：无
// 备注：不再使用这个一对多特征比对列表时必须销毁以释放资源；列表销毁后句柄随之失效，如还需进行一对多比对必须重新创建一对多比对列表，不可再用这个句柄加入模板或进行一对多比对。
DLL_FUNC void NMSDK_IRFACE_DestroyFeatureList(HANDLE hList);


#ifdef __cplusplus
}
#endif

#endif // __NM_FACE_SDK_H__

