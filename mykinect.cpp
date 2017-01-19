#include "myKinect.h"
#include <iostream>
#include <math.h>

#define lengthof(a) (sizeof(a)/(sizeof(*(a))))

// define the face frame features required to be computed by this application
static const DWORD c_FaceFrameFeatures =
FaceFrameFeatures::FaceFrameFeatures_BoundingBoxInColorSpace
| FaceFrameFeatures::FaceFrameFeatures_PointsInColorSpace
| FaceFrameFeatures::FaceFrameFeatures_RotationOrientation
| FaceFrameFeatures::FaceFrameFeatures_Happy
| FaceFrameFeatures::FaceFrameFeatures_RightEyeClosed
| FaceFrameFeatures::FaceFrameFeatures_LeftEyeClosed
| FaceFrameFeatures::FaceFrameFeatures_MouthOpen
| FaceFrameFeatures::FaceFrameFeatures_MouthMoved
| FaceFrameFeatures::FaceFrameFeatures_LookingAway
| FaceFrameFeatures::FaceFrameFeatures_Glasses
| FaceFrameFeatures::FaceFrameFeatures_FaceEngagement;
/// Initializes the default Kinect sensor
HRESULT CBodyBasics::InitializeDefaultSensor()
{
	//by �����
	color[0] = cv::Scalar(255, 0, 0);
	color[1] = cv::Scalar(0, 255, 0);
	color[2] = cv::Scalar(0, 0, 255);
	color[3] = cv::Scalar(255, 255, 0);
	color[4] = cv::Scalar(255, 0, 255);
	color[5] = cv::Scalar(0, 255, 255);



	//�����ж�ÿ�ζ�ȡ�����ĳɹ����
	HRESULT hr;

	//����kinect
	hr = GetDefaultKinectSensor(&m_pKinectSensor);
	if (FAILED(hr)){
		return hr;
	}

	//�ҵ�kinect�豸
	if (m_pKinectSensor)
	{
		// Initialize the Kinect and get coordinate mapper and the body reader
		IColorFrameSource* pColorFrameSource = NULL;//��ȡ��ɫͼ
		IBodyFrameSource* pBodyFrameSource = NULL;//��ȡ�Ǽ�
		IDepthFrameSource* pDepthFrameSource = NULL;//��ȡ�����Ϣ
		IBodyIndexFrameSource* pBodyIndexFrameSource = NULL;//��ȡ������ֵͼ

		//��kinect
		hr = m_pKinectSensor->Open();

		//coordinatemapper
		if (SUCCEEDED(hr))
		{
			hr = m_pKinectSensor->get_CoordinateMapper(&m_pCoordinateMapper);
		}

		//colorframe
		if (SUCCEEDED(hr))
		{
			hr = m_pKinectSensor->get_ColorFrameSource(&pColorFrameSource);
		}

		if (SUCCEEDED(hr))
		{
			hr = pColorFrameSource->OpenReader(&m_pColorFrameReader);
		}

		//bodyframe
		if (SUCCEEDED(hr))
		{
			hr = m_pKinectSensor->get_BodyFrameSource(&pBodyFrameSource);
		}

		if (SUCCEEDED(hr))
		{
			hr = pBodyFrameSource->OpenReader(&m_pBodyFrameReader);
		}

		//depth frame
		if (SUCCEEDED(hr)){
			hr = m_pKinectSensor->get_DepthFrameSource(&pDepthFrameSource);
		}

		if (SUCCEEDED(hr)){
			hr = pDepthFrameSource->OpenReader(&m_pDepthFrameReader);
		}

		//body index frame
		if (SUCCEEDED(hr)){
			hr = m_pKinectSensor->get_BodyIndexFrameSource(&pBodyIndexFrameSource);
		}

		if (SUCCEEDED(hr)){
			hr = pBodyIndexFrameSource->OpenReader(&m_pBodyIndexFrameReader);
		}
		if (SUCCEEDED(hr))
		{
			// create a face frame source + reader to track each body in the fov
			for (int i = 0; i < BODY_COUNT; i++)
			{
				if (SUCCEEDED(hr))
				{
					// create the face frame source by specifying the required face frame features
					hr = CreateFaceFrameSource(m_pKinectSensor, 0, c_FaceFrameFeatures, &m_pFaceFrameSources[i]);
				}
				if (SUCCEEDED(hr))
				{
					// open the corresponding reader
					hr = m_pFaceFrameSources[i]->OpenReader(&m_pFaceFrameReaders[i]);
				}
			}
		}

		if (SUCCEEDED(hr))
		{
			// create a face frame source + reader to track each body in the fov
			for (int i = 0; i < BODY_COUNT; i++)
			{
				if (SUCCEEDED(hr))
				{
					// create the face frame source by specifying the required face frame features
					hr = CreateHighDefinitionFaceFrameSource(m_pKinectSensor, &m_pHDFaceFrameSources[i]);
				}
				if (SUCCEEDED(hr))
				{
					// open the corresponding reader
					hr = m_pHDFaceFrameSources[i]->OpenReader(&m_pHDFaceFrameReaders[i]);
				}
				if (SUCCEEDED(hr))
				{
					hr = m_pHDFaceFrameSources[i]->OpenModelBuilder(FaceModelBuilderAttributes_None, &m_pFaceModelBuilders[i]);
				}
				// ��ʼ�����ռ�  
				if (SUCCEEDED(hr) && m_pFaceModelBuilders[i])
				{
					hr = m_pFaceModelBuilders[i]->BeginFaceDataCollection();
				}
			}
		}
		SafeRelease(pColorFrameSource);
		SafeRelease(pBodyFrameSource);
		SafeRelease(pDepthFrameSource);
		SafeRelease(pBodyIndexFrameSource);
	}

	if (!m_pKinectSensor || FAILED(hr))
	{
		std::cout << "Kinect initialization failed!" << std::endl;
		return E_FAIL;
	}

	memset(bodylength, 50, 6);

	//skeletonImg,���ڻ��Ǽܡ�������ֵͼ��MAT
	skeletonImg.create(cDepthHeight, cDepthWidth, CV_8UC3);
	skeletonImg.setTo(0);

	//depthImg,���ڻ������Ϣ��MAT
	depthImg.create(cDepthHeight, cDepthWidth, CV_8UC1);
	depthImg.setTo(0);

	//ColorImg,���ڻ���ɫ��Ϣ��MAT
	colorImg.create(colorHeight / 2, colorWidth / 2, CV_8UC3);
	depthImg.setTo(0);

	return hr;
}


/// Main processing function
void CBodyBasics::Update()
{
	//ÿ�������skeletonImg
	skeletonImg.setTo(0);

	//�����ʧ��kinect���򲻼�������
	if (!m_pBodyFrameReader)
	{
		return;
	}

	IBodyFrame* pBodyFrame = NULL;//�Ǽ���Ϣ
	IColorFrame* pColorFrame = NULL;//��ɫ��Ϣ
	IDepthFrame* pDepthFrame = NULL;//�����Ϣ
	IBodyIndexFrame* pBodyIndexFrame = NULL;//������ֵͼ

	//��¼ÿ�β����ĳɹ����
	HRESULT hr = S_OK;

	//---------------------------------------��ȡ������ֵͼ����ʾ---------------------------------
	if (SUCCEEDED(hr)){
		hr = m_pBodyIndexFrameReader->AcquireLatestFrame(&pBodyIndexFrame);//��ñ�����ֵͼ��Ϣ
	}
	/*
	if (SUCCEEDED(hr)){
	BYTE *bodyIndexArray = new BYTE[cDepthHeight * cDepthWidth];//������ֵͼ��8Ϊuchar�������Ǻ�ɫ��û���ǰ�ɫ
	pBodyIndexFrame->CopyFrameDataToArray(cDepthHeight * cDepthWidth, bodyIndexArray);

	//�ѱ�����ֵͼ����MAT��
	uchar* skeletonData = (uchar*)skeletonImg.data;
	for (int j = 0; j < cDepthHeight * cDepthWidth; ++j){
	*skeletonData = bodyIndexArray[j]; ++skeletonData;
	*skeletonData = bodyIndexArray[j]; ++skeletonData;
	*skeletonData = bodyIndexArray[j]; ++skeletonData;
	}
	delete[] bodyIndexArray;
	}*/
	//by �����
	if (SUCCEEDED(hr)){
		unsigned int bufferSize = 0;
		unsigned char* buffer = nullptr;
		hr = pBodyIndexFrame->AccessUnderlyingBuffer(&bufferSize, &buffer);
		if (SUCCEEDED(hr)){
			for (int y = 0; y < cDepthHeight; y++){
				for (int x = 0; x < cDepthWidth; x++){
					unsigned int index = y * cDepthWidth + x;
					if (buffer[index] != 0xff){
						skeletonImg.at<cv::Vec3b>(y, x) = cv::Vec3b(255, 255, 255); //color[buffer[index]];
					}
					else{
						skeletonImg.at<cv::Vec3b>(y, x) = cv::Vec3b(0, 0, 0);
					}
				}
			}
		}
	}

	SafeRelease(pBodyIndexFrame);//����Ҫ�ͷţ�����֮���޷�����µ�frame����

	//-----------------------��ȡ������ݲ���ʾ--------------------------
	if (SUCCEEDED(hr)){
		hr = m_pDepthFrameReader->AcquireLatestFrame(&pDepthFrame);//����������
	}
	if (SUCCEEDED(hr)){
		UINT16 *depthArray = new UINT16[cDepthHeight * cDepthWidth];//���������16λunsigned int
		pDepthFrame->CopyFrameDataToArray(cDepthHeight * cDepthWidth, depthArray);

		//��������ݻ���MAT��
		uchar* depthData = (uchar*)depthImg.data;
		for (int j = 0; j < cDepthHeight * cDepthWidth; ++j){
			*depthData = depthArray[j];
			++depthData;
		}
		delete[] depthArray;
	}
	SafeRelease(pDepthFrame);//����Ҫ�ͷţ�����֮���޷�����µ�frame����
	imshow("depthImg", depthImg);
	cv::waitKey(1);

	//-----------------------��ȡ��ɫ���ݲ���ʾ--------------------------
	unsigned int bufferSize = colorWidth * colorHeight * 4 * sizeof(unsigned char);
	cv::Mat bufferMat(colorHeight, colorWidth, CV_8UC4);
	cv::namedWindow("Body");

	if (SUCCEEDED(hr)){
		hr = m_pColorFrameReader->AcquireLatestFrame(&pColorFrame);//����������
	}

	if (SUCCEEDED(hr)){
		hr = pColorFrame->CopyConvertedFrameDataToArray(bufferSize, reinterpret_cast<BYTE*>(bufferMat.data), ColorImageFormat_Bgra);
		if (SUCCEEDED(hr)){
			cv::resize(bufferMat, colorImg, cv::Size(), 0.5, 0.5);
		}
	}
	SafeRelease(pColorFrame);
	// Show Window

	//-----------------------------��ȡ�Ǽܲ���ʾ----------------------------
	if (SUCCEEDED(hr)){
		hr = m_pBodyFrameReader->AcquireLatestFrame(&pBodyFrame);//��ȡ�Ǽ���Ϣ
	}
	if (SUCCEEDED(hr))
	{
		IBody* ppBodies[BODY_COUNT] = { 0 };//ÿһ��IBody����׷��һ���ˣ��ܹ�����׷��������

		if (SUCCEEDED(hr))
		{
			//��kinect׷�ٵ����˵���Ϣ���ֱ�浽ÿһ��IBody��
			hr = pBodyFrame->GetAndRefreshBodyData(_countof(ppBodies), ppBodies);
		}

		if (SUCCEEDED(hr))
		{
			//��ÿһ��IBody�������ҵ����ĹǼ���Ϣ�����һ�����
			ProcessBody(BODY_COUNT, ppBodies);
		}

		for (int i = 0; i < _countof(ppBodies); ++i)
		{
			SafeRelease(ppBodies[i]);//�ͷ�����
		}
	}
	SafeRelease(pBodyFrame);//����Ҫ�ͷţ�����֮���޷�����µ�frame����

}

/// Handle new body data
void CBodyBasics::ProcessBody(int nBodyCount, IBody** ppBodies)
{
	//��¼��������Ƿ�ɹ�
	HRESULT hr;

	//����ÿһ��IBody
	for (int i = 0; i < nBodyCount; ++i)
	{
		myState[i] = 0;
		IBody* pBody = ppBodies[i];
		if (pBody)//��û�и���������pBody�������bTracked��ʲô����
		{
			BOOLEAN bTracked = false;
			hr = pBody->get_IsTracked(&bTracked);

			if (SUCCEEDED(hr) && bTracked)
			{
				Joint joints[JointType_Count];//�洢�ؽڵ���
				HandState leftHandState = HandState_Unknown;//����״̬
				HandState rightHandState = HandState_Unknown;//����״̬
				//��ȡ������״̬
				pBody->get_HandLeftState(&leftHandState);
				pBody->get_HandRightState(&rightHandState);

				//�洢�������ϵ�еĹؽڵ�λ��
				DepthSpacePoint *depthSpacePosition = new DepthSpacePoint[_countof(joints)];
				ColorSpacePoint *colorSpacePosition = new ColorSpacePoint[_countof(joints)];

				//��ùؽڵ���
				hr = pBody->GetJoints(_countof(joints), joints);

				if (SUCCEEDED(hr))
				{
					myState[i] = 1;
					CameraSpacePoint p_SpineMid = joints[JointType_SpineMid].Position; //by����� ���嶨λ������
					//if (p_SpineMid.X>-2 && p_SpineMid.Y>-1.2)
					X[i] = p_SpineMid.X * 100 + 350;  //��λcm
					Y[i] = p_SpineMid.Y * 100 + 430;
					Z[i] = abs(p_SpineMid.Z * 100);
					/*
					int temp_x = X[i];
					int temp_y = Y[i];
					int temp_z = Z[i];
					X[i] = -0.9584*X[i] - 0.0055*Y[i] + 0.0326*Z[i] + 686.59;
					Y[i] = 0.022*temp_x + 0.0866*Y[i] - 0.0328*Z[i] + 100.8751;
					Z[i] = 0.0579*temp_x - 0.2781*temp_y - 0.9982*Z[i] + 605.53;
					*/

					float height = Height(joints);
					if (height > 1 && p_SpineMid.Z > 1 && p_SpineMid.Z < 4.50)
					{
						bodylength[i] = height * 100;
					}

					for (int j = 0; j < _countof(joints); ++j)
					{
						//���ؽڵ���������������ϵ��-1~1��ת���������ϵ��424*512��
						m_pCoordinateMapper->MapCameraPointToDepthSpace(joints[j].Position, &depthSpacePosition[j]);
						m_pCoordinateMapper->MapCameraPointToColorSpace(joints[j].Position, &colorSpacePosition[j]);


						//------------------------central position-------------------------------
						//circle(skeletonImg,
							//cvPoint(depthSpacePosition[JointType_SpineMid].X, depthSpacePosition[JointType_SpineMid].Y),
							//8, color[i], -1); //by����� ���嶨λ������
						/*
						int rect_height = abs(colorSpacePosition[JointType_SpineMid].Y - colorSpacePosition[JointType_SpineBase].Y) / 2;
						int rect_width = abs(colorSpacePosition[JointType_SpineShoulder].X - colorSpacePosition[JointType_ShoulderLeft].X) / 2;
						int rect_x = colorSpacePosition[JointType_SpineMid].X / 2 - rect_width / 2;
						int rect_y = colorSpacePosition[JointType_SpineMid].Y / 2 - rect_height / 2;
						if ((rect_x > 0) && (rect_x + rect_width < colorWidth) && (rect_y > 0) && (rect_y + rect_height< colorHeight) && (rect_width>0) && (rect_height>0))
						{
						cv::Mat color_roi(colorImg, cvRect(rect_x, rect_y, rect_width, rect_height));
						//cv::Mat hsv_roi;
						rectangle(colorImg, cvPoint(rect_x, rect_y), cvPoint(rect_x + rect_width, rect_y + rect_height), color[i], 0.5);
						//cvtColor(color_roi, hsv_roi, CV_BGR2HSV);
						CvMat mat_roi = color_roi;
						//color_average[i] = cvAvg(&mat_roi);
						CvScalar avgChannels = cvAvg(&mat_roi);
						avgB[i] = avgChannels.val[0];
						avgG[i] = avgChannels.val[1];
						avgR[i] = avgChannels.val[2];
						}*/
					}

					//------------------------hand state left-------------------------------
					//DrawHandState(depthSpacePosition[JointType_HandLeft], leftHandState);
					//DrawHandState(depthSpacePosition[JointType_HandRight], rightHandState);

					if (leftHandState == 2)
					{
						std::cout << "Position of No." << i << " user: " << X[i] << " "  << Z[i] << std::endl;
						std::cout << "Body height of No." << i << " user:" << bodylength[i] << std::endl;
						std::cout << " B:" << avgB[i] << " G:" << avgG[i] << " R:" << avgR[i] << std::endl;
					}

					/*//---------------------------body-------------------------------
					DrawBone(joints, depthSpacePosition, JointType_Head, JointType_Neck);
					DrawBone(joints, depthSpacePosition, JointType_Neck, JointType_SpineShoulder);
					DrawBone(joints, depthSpacePosition, JointType_SpineShoulder, JointType_SpineMid);
					DrawBone(joints, depthSpacePosition, JointType_SpineMid, JointType_SpineBase);
					DrawBone(joints, depthSpacePosition, JointType_SpineShoulder, JointType_ShoulderRight);
					DrawBone(joints, depthSpacePosition, JointType_SpineShoulder, JointType_ShoulderLeft);
					DrawBone(joints, depthSpacePosition, JointType_SpineBase, JointType_HipRight);
					DrawBone(joints, depthSpacePosition, JointType_SpineBase, JointType_HipLeft);

					// -----------------------Right Arm ------------------------------------ 
					DrawBone(joints, depthSpacePosition, JointType_ShoulderRight, JointType_ElbowRight);
					DrawBone(joints, depthSpacePosition, JointType_ElbowRight, JointType_WristRight);
					DrawBone(joints, depthSpacePosition, JointType_WristRight, JointType_HandRight);
					DrawBone(joints, depthSpacePosition, JointType_HandRight, JointType_HandTipRight);
					DrawBone(joints, depthSpacePosition, JointType_WristRight, JointType_ThumbRight);

					//----------------------------------- Left Arm--------------------------
					DrawBone(joints, depthSpacePosition, JointType_ShoulderLeft, JointType_ElbowLeft);
					DrawBone(joints, depthSpacePosition, JointType_ElbowLeft, JointType_WristLeft);
					DrawBone(joints, depthSpacePosition, JointType_WristLeft, JointType_HandLeft);
					DrawBone(joints, depthSpacePosition, JointType_HandLeft, JointType_HandTipLeft);
					DrawBone(joints, depthSpacePosition, JointType_WristLeft, JointType_ThumbLeft);

					// ----------------------------------Right Leg--------------------------------
					DrawBone(joints, depthSpacePosition, JointType_HipRight, JointType_KneeRight);
					DrawBone(joints, depthSpacePosition, JointType_KneeRight, JointType_AnkleRight);
					DrawBone(joints, depthSpacePosition, JointType_AnkleRight, JointType_FootRight);

					// -----------------------------------Left Leg---------------------------------
					DrawBone(joints, depthSpacePosition, JointType_HipLeft, JointType_KneeLeft);
					DrawBone(joints, depthSpacePosition, JointType_KneeLeft, JointType_AnkleLeft);
					DrawBone(joints, depthSpacePosition, JointType_AnkleLeft, JointType_FootLeft);*/
				}

				delete[] depthSpacePosition;
				delete[] colorSpacePosition;

				////////////////////////////////////////////////////////////////////////////////////////////////////////
				// retrieve the latest face frame from this reader

				IFaceFrame* pFaceFrame = nullptr;
				hr = m_pFaceFrameReaders[i]->AcquireLatestFrame(&pFaceFrame);

				BOOLEAN bFaceTracked = false;
				if (SUCCEEDED(hr) && nullptr != pFaceFrame)
				{
					// check if a valid face is tracked in this face frame
					hr = pFaceFrame->get_IsTrackingIdValid(&bFaceTracked);
				}

				if (SUCCEEDED(hr))
				{
					if (bFaceTracked)
					{
						IFaceFrameResult* pFaceFrameResult = nullptr;
						RectI faceBox = { 0 };
						PointF facePoints[FacePointType::FacePointType_Count];
						Vector4 faceRotation;
						DetectionResult faceProperties[FaceProperty::FaceProperty_Count];
						D2D1_POINT_2F faceTextLayout;

						hr = pFaceFrame->get_FaceFrameResult(&pFaceFrameResult);

						// need to verify if pFaceFrameResult contains data before trying to access it
						if (SUCCEEDED(hr) && pFaceFrameResult != nullptr)
						{
							hr = pFaceFrameResult->get_FaceBoundingBoxInColorSpace(&faceBox);
							//std::cout << faceBox.Left << "��" << faceBox.Right << "��" << faceBox.Top << "��" << faceBox.Bottom << "��" << std::endl;
							if (SUCCEEDED(hr))
							{
								hr = pFaceFrameResult->GetFacePointsInColorSpace(FacePointType::FacePointType_Count, facePoints);
							}

							if (SUCCEEDED(hr))
							{
								hr = pFaceFrameResult->get_FaceRotationQuaternion(&faceRotation);
							}

							if (SUCCEEDED(hr))
							{
								hr = pFaceFrameResult->GetFaceProperties(FaceProperty::FaceProperty_Count, faceProperties);
								//std::cout << faceProperties[0] << "��" << faceProperties[1] << "��" << faceProperties[2] << "��" << faceProperties[3] << "��" << std::endl;
								if (faceProperties[2] == 3)
									rectangle(colorImg, cvPoint(faceBox.Left / 2, faceBox.Top / 2), cvPoint(faceBox.Right / 2, faceBox.Bottom / 2), color[i], 5);
								else
									rectangle(colorImg, cvPoint(faceBox.Left / 2, faceBox.Top / 2), cvPoint(faceBox.Right / 2, faceBox.Bottom / 2), color[i], 0.5);

							}


						}

						SafeRelease(pFaceFrameResult);
					}
					else
					{
						// face tracking is not valid - attempt to fix the issue
						// a valid body is required to perform this step
						//if (bHaveBodyData)
						{
							// check if the corresponding body is tracked 
							// if this is true then update the face frame source to track this body
							IBody* pBody = ppBodies[i];
							if (pBody != nullptr)
							{
								BOOLEAN bTracked = false;
								hr = pBody->get_IsTracked(&bTracked);

								UINT64 bodyTId;
								if (SUCCEEDED(hr) && bTracked)
								{
									// get the tracking ID of this body
									hr = pBody->get_TrackingId(&bodyTId);
									if (SUCCEEDED(hr))
									{
										// update the face frame source with the tracking ID
										m_pFaceFrameSources[i]->put_TrackingId(bodyTId);
									}
								}
							}
						}
					}
				}

				SafeRelease(pFaceFrame);
			}
		}
						else
				{
					myState[i] = 0;
					bodylength[i] = 0;
					avgB[i] = 0;
					avgG[i] = 0;
					avgR[i] = 0;
					X[i] = 0;
					Y[i] = 0;
					Z[i] = 0;
				}

	}
	cv::imshow("Body", colorImg);
	cv::imshow("skeletonImg", skeletonImg);
	cv::waitKey(5);
}

//���ֵ�״̬
void CBodyBasics::DrawHandState(const DepthSpacePoint depthSpacePosition, HandState handState)
{
	//����ͬ�����Ʒ��䲻ͬ��ɫ
	CvScalar color;
	switch (handState){
	case HandState_Open:
		color = cvScalar(255, 0, 0);
		break;
	case HandState_Closed:
		color = cvScalar(0, 255, 0);
		break;
	case HandState_Lasso:
		color = cvScalar(0, 0, 255);
		break;
	default://���û��ȷ�������ƣ��Ͳ�Ҫ��
		return;
	}

	circle(skeletonImg,
		cvPoint(depthSpacePosition.X, depthSpacePosition.Y),
		20, color, -1);
}


/// Draws one bone of a body (joint to joint)
void CBodyBasics::DrawBone(const Joint* pJoints, const DepthSpacePoint* depthSpacePosition, JointType joint0, JointType joint1)
{
	TrackingState joint0State = pJoints[joint0].TrackingState;
	TrackingState joint1State = pJoints[joint1].TrackingState;

	// If we can't find either of these joints, exit
	if ((joint0State == TrackingState_NotTracked) || (joint1State == TrackingState_NotTracked))
	{
		return;
	}

	// Don't draw if both points are inferred
	if ((joint0State == TrackingState_Inferred) && (joint1State == TrackingState_Inferred))
	{
		return;
	}

	CvPoint p1 = cvPoint(depthSpacePosition[joint0].X, depthSpacePosition[joint0].Y),
		p2 = cvPoint(depthSpacePosition[joint1].X, depthSpacePosition[joint1].Y);

	// We assume all drawn bones are inferred unless BOTH joints are tracked
	if ((joint0State == TrackingState_Tracked) && (joint1State == TrackingState_Tracked))
	{
		//�ǳ�ȷ���ĹǼܣ��ú�ɫֱ��
		line(skeletonImg, p1, p2, cvScalar(0, 0, 0));
	}
	else
	{
		//��ȷ���ĹǼܣ��ú�ɫֱ��
		line(skeletonImg, p1, p2, cvScalar(255, 0, 0));
	}
}


/// Constructor
CBodyBasics::CBodyBasics() :
m_pKinectSensor(NULL),
m_pCoordinateMapper(NULL),
m_pBodyFrameReader(NULL){}

/// Destructor
CBodyBasics::~CBodyBasics()
{
	SafeRelease(m_pBodyFrameReader);
	SafeRelease(m_pCoordinateMapper);

	if (m_pKinectSensor)
	{
		m_pKinectSensor->Close();
	}
	SafeRelease(m_pKinectSensor);
}


float CBodyBasics::Length(Joint p1, Joint p2)
{
	return sqrt(
		pow(p1.Position.X - p2.Position.X, 2) +
		pow(p1.Position.Y - p2.Position.Y, 2) +
		pow(p1.Position.Z - p2.Position.Z, 2));
}

int CBodyBasics::NumberOfTrackedJoints(Joint joint1, Joint joint2, Joint joint3, Joint joint4)
{
	int trackedJoints = 0;

	trackedJoints = joint1.TrackingState + joint2.TrackingState + joint3.TrackingState + joint4.TrackingState;

	return trackedJoints;
}

float CBodyBasics::Distance(Joint joint1, Joint joint2, Joint joint3, Joint joint4)
{
	float distance = 0;

	distance = Length(joint1, joint2) + Length(joint2, joint3) + Length(joint3, joint4);

	return distance;
}

float CBodyBasics::Height(Joint joints[JointType_Count])
{
	const double HEAD_DIVERGENCE = 0.1;

	Joint head = joints[JointType_Head];
	Joint neck = joints[JointType_Neck];
	Joint shoulder = joints[JointType_SpineShoulder];
	Joint spine = joints[JointType_SpineMid];
	Joint waist = joints[JointType_SpineBase];
	Joint hipLeft = joints[JointType_HipLeft];
	Joint hipRight = joints[JointType_HipRight];
	Joint kneeLeft = joints[JointType_KneeLeft];
	Joint kneeRight = joints[JointType_KneeRight];
	Joint ankleLeft = joints[JointType_AnkleLeft];
	Joint ankleRight = joints[JointType_AnkleRight];
	Joint footLeft = joints[JointType_FootLeft];
	Joint footRight = joints[JointType_FootRight];

	// Find which leg is tracked more accurately.
	int legLeftTrackedJoints = NumberOfTrackedJoints(hipLeft,
		kneeLeft,
		ankleLeft,
		footLeft);
	int legRightTrackedJoints = NumberOfTrackedJoints(hipRight,
		kneeRight,
		ankleRight,
		footRight);
	int BodyTrackedJoints = NumberOfTrackedJoints(head,
		shoulder,
		neck,
		spine);
	if ((legLeftTrackedJoints < 8 && legRightTrackedJoints < 8) || BodyTrackedJoints<8)
		return -1;
	else
	{
		double legLength = legLeftTrackedJoints > legRightTrackedJoints ?
			Distance(hipLeft, kneeLeft, ankleLeft, footLeft) :
			Distance(hipRight, kneeRight, ankleRight, footRight);

		return Distance(head, neck, shoulder, spine) + Length(spine, waist) + legLength + HEAD_DIVERGENCE;
	}
}