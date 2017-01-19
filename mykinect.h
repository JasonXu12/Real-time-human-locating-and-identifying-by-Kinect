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
	//kinect 2.0 ����ȿռ�ĸ�*���� 424 * 512���ڹ�������˵��
	static const int        cDepthWidth = 512;
	static const int        cDepthHeight = 424;
	int colorWidth = 1920;
	int colorHeight = 1080;
	cv::Scalar color[6]; //by �����
public:
	CBodyBasics();
	~CBodyBasics();
	void                    Update();//��ùǼܡ�������ֵͼ�������Ϣ
	HRESULT                 InitializeDefaultSensor();//���ڳ�ʼ��kinect
	int X[BODY_COUNT];
	int Y[BODY_COUNT];
	int Z[BODY_COUNT];
	bool myState[BODY_COUNT];
	int bodylength[BODY_COUNT];//�����¼ÿ���˵����
	float Length(Joint p1, Joint p2);
	float Height(Joint joints[JointType_Count]);
	float Distance(Joint joint1, Joint joint2, Joint joint3, Joint joint4);
	int NumberOfTrackedJoints(Joint joint1, Joint joint2, Joint joint3, Joint joint4);

	int avgB[BODY_COUNT];
	int avgG[BODY_COUNT];
	int avgR[BODY_COUNT];
	float                   sd[BODY_COUNT][FaceShapeDeformations_Count];
private:
	IKinectSensor*          m_pKinectSensor;//kinectԴ
	ICoordinateMapper*      m_pCoordinateMapper;//��������任
	IBodyFrameReader*       m_pBodyFrameReader;//���ڹǼ����ݶ�ȡ
	IDepthFrameReader*      m_pDepthFrameReader;//����������ݶ�ȡ
	IColorFrameReader*      m_pColorFrameReader;//���ڲ�ɫ���ݶ�ȡ
	IBodyIndexFrameReader*  m_pBodyIndexFrameReader;//���ڱ�����ֵͼ��ȡ
	IFaceFrameSource*	   m_pFaceFrameSources[BODY_COUNT];
	// Face readers
	IFaceFrameReader*	   m_pFaceFrameReaders[BODY_COUNT];
	IHighDefinitionFaceFrameSource*  m_pHDFaceFrameSources[BODY_COUNT];
	IHighDefinitionFaceFrameReader*  m_pHDFaceFrameReaders[BODY_COUNT];
	IFaceModelBuilder* m_pFaceModelBuilders[BODY_COUNT];
	IFaceModel*        m_pFaceModels[BODY_COUNT];

	//ͨ����õ�����Ϣ���ѹǼܺͱ�����ֵͼ������
	void                    ProcessBody(int nBodyCount, IBody** ppBodies);
	//���Ǽܺ���
	void DrawBone(const Joint* pJoints, const DepthSpacePoint* depthSpacePosition, JointType joint0, JointType joint1);
	//���ֵ�״̬����
	void DrawHandState(const DepthSpacePoint depthSpacePosition, HandState handState);

	//��ʾͼ���Mat
	cv::Mat skeletonImg;
	cv::Mat depthImg;
	cv::Mat colorImg;

};

