#pragma once
#include <Kinect.h>
#include <d2d1.h>
#include <Kinect.Face.h>
#include <opencv2\opencv.hpp>

// Safe release for interfaces
template<class Interface>
inline void SafeRelease(Interface *& pInterfaceToRelease)
{
	if (pInterfaceToRelease != NULL)
	{
		pInterfaceToRelease->Release();
		pInterfaceToRelease = NULL;
	}
}

class CBodyBasics
{
	//kinect 2.0 的深度空间的高*宽是 424 * 512，在官网上有说明
	static const int        cDepthWidth = 512;
	static const int        cDepthHeight = 424;
	int colorWidth = 1920;
	int colorHeight = 1080;
	cv::Scalar color[6]; //by 许稼轩
public:
	CBodyBasics();
	~CBodyBasics();
	void                    Update();//获得骨架、背景二值图和深度信息
	HRESULT                 InitializeDefaultSensor();//用于初始化kinect
	int X[BODY_COUNT];
	int Y[BODY_COUNT];
	int Z[BODY_COUNT];
	bool myState[BODY_COUNT];
	int bodylength[BODY_COUNT];//数组记录每个人的身高
	float Length(Joint p1, Joint p2);
	float Height(Joint joints[JointType_Count]);
	float Distance(Joint joint1, Joint joint2, Joint joint3, Joint joint4);
	int NumberOfTrackedJoints(Joint joint1, Joint joint2, Joint joint3, Joint joint4);

	int avgB[BODY_COUNT];
	int avgG[BODY_COUNT];
	int avgR[BODY_COUNT];
	float                   sd[BODY_COUNT][FaceShapeDeformations_Count];
private:
	IKinectSensor*          m_pKinectSensor;//kinect源
	ICoordinateMapper*      m_pCoordinateMapper;//用于坐标变换
	IBodyFrameReader*       m_pBodyFrameReader;//用于骨架数据读取
	IDepthFrameReader*      m_pDepthFrameReader;//用于深度数据读取
	IColorFrameReader*      m_pColorFrameReader;//用于彩色数据读取
	IBodyIndexFrameReader*  m_pBodyIndexFrameReader;//用于背景二值图读取
	IFaceFrameSource*	   m_pFaceFrameSources[BODY_COUNT];
	// Face readers
	IFaceFrameReader*	   m_pFaceFrameReaders[BODY_COUNT];
	IHighDefinitionFaceFrameSource*  m_pHDFaceFrameSources[BODY_COUNT];
	IHighDefinitionFaceFrameReader*  m_pHDFaceFrameReaders[BODY_COUNT];
	IFaceModelBuilder* m_pFaceModelBuilders[BODY_COUNT];
	IFaceModel*        m_pFaceModels[BODY_COUNT];

	//通过获得到的信息，把骨架和背景二值图画出来
	void                    ProcessBody(int nBodyCount, IBody** ppBodies);
	//画骨架函数
	void DrawBone(const Joint* pJoints, const DepthSpacePoint* depthSpacePosition, JointType joint0, JointType joint1);
	//画手的状态函数
	void DrawHandState(const DepthSpacePoint depthSpacePosition, HandState handState);

	//显示图像的Mat
	cv::Mat skeletonImg;
	cv::Mat depthImg;
	cv::Mat colorImg;

};

