
#include "stdafx.h"
#include "CKinectSensor.h"



STDMETHODIMP CKinectSensor::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
	if (riid == __uuidof(IMyKinectSensor)){
		return DLPGetInterface((IMyKinectSensor*)this, ppv);
	}

	if (riid == __uuidof(IImgTransform)) {
		return DLPGetInterface((IImgTransform*)this, ppv);
	}

	return __super::NonDelegatingQueryInterface(riid, ppv);
}




CKinectSensor::CKinectSensor() : m_waitColorHandle(0), m_waitDepthHandle(0), m_waitIRHandle(0), m_RegRadius(100)
{
	PIDLPUnknown p = CreateHandObj(__uuidof(ICalib));
	m_PICalib = GetDLPUnknown<ICalib>(p);
}

BOOL CKinectSensor::GetDepthHandle()
{

	IDepthFrameSource* pDepthFrameSource = nullptr;
	IDepthFrameReader* DepthFrameReader  = nullptr;

	if (!m_pKinectSensor)
		return false;

	HRESULT hr = m_pKinectSensor->get_DepthFrameSource(&pDepthFrameSource);

	if (FAILED(hr))
		return false;

	hr = pDepthFrameSource->OpenReader(&DepthFrameReader);
	SafeRelease(pDepthFrameSource);

	if (FAILED(hr))
		return false;

	m_pIDepthFrameReader = DepthFrameReader;
	SafeRelease(DepthFrameReader);

	m_pIDepthFrameReader->SubscribeFrameArrived(&m_waitDepthHandle);

	return SUCCEEDED(hr);
}




BOOL CKinectSensor::GetColorHandle()
{
	
	IColorFrameSource* pColorFrameSource = nullptr;
	IColorFrameReader* ColorFrameReader  = nullptr;

	if (!m_pKinectSensor)
		return false;

	HRESULT hr = m_pKinectSensor->get_ColorFrameSource(&pColorFrameSource);

	if (FAILED(hr))
		return false;

	hr = pColorFrameSource->OpenReader(&ColorFrameReader);
	SafeRelease(pColorFrameSource);

	if (FAILED(hr))
		return false;

	m_pIColorFrameReader = ColorFrameReader;
	SafeRelease(ColorFrameReader);

	hr = m_pIColorFrameReader->SubscribeFrameArrived(&m_waitColorHandle);

	return SUCCEEDED(hr);
}




BOOL CKinectSensor::GetIRHandle()
{
	IInfraredFrameSource* pInfraredFrameSource = nullptr;
	IInfraredFrameReader* pInfraredFrameReader = nullptr;

	if (!m_pKinectSensor)
		return false;

	HRESULT hr = m_pKinectSensor->get_InfraredFrameSource(&pInfraredFrameSource);
	if (FAILED(hr))
		return false;

	
	hr = pInfraredFrameSource->OpenReader(&pInfraredFrameReader);
	SafeRelease(pInfraredFrameSource);

	if (FAILED(hr))
		return false;

	m_pIInfraredFrameReader = pInfraredFrameReader;
	SafeRelease(pInfraredFrameReader);

	hr = m_pIInfraredFrameReader->SubscribeFrameArrived(&m_waitIRHandle);
	return SUCCEEDED(hr);
}




BOOL CKinectSensor::OpenDevice()
{
	if (m_pKinectSensor) 
		return true;
	
	IKinectSensor*  KinectSensor;
	HRESULT hr = GetDefaultKinectSensor(&KinectSensor);
	m_pKinectSensor =  KinectSensor;
	SafeRelease(KinectSensor);

	if (SUCCEEDED(hr))
		hr = m_pKinectSensor->Open();

	return SUCCEEDED(hr);
}




CKinectSensor::~CKinectSensor()
{
	Close();
} 


BOOL CKinectSensor::Connect(PIDLPUnknown pRcv, BOOL bConnect, eKinectImage ImageType){

	PIKinectImage    p1 = GetDLPUnknown<IKinectImage>(pRcv);
	PIKinectSkeleton p2 = GetDLPUnknown<IKinectSkeleton>(pRcv);
	
	if (!p1 && !p2) 
		return false;
	
	THREAD::CAutoCriticalSection cs(m_cs);
	if (bConnect) {
		if (p1) {
			switch (ImageType) {
			case  KI_COLOR:		m_ColorImageSink.push_back(p1);			break;
			case  KI_IR:		m_IRImageSink.push_back(p1);			break;
			case  KI_Depth:		m_DepthImageSink.push_back(p1);			break;
			default:
				BOOST_ASSERT(!"ImageType Error ");
			}
		}

		if (p2)
			m_SkeletonSink.push_back(p2);
	}

	else {
		if (p1) {
			switch (ImageType) {
			case KI_COLOR:
				if (m_ColorImageSink.empty())
					return false;
				for (auto it = m_ColorImageSink.begin(); it != m_ColorImageSink.end(); ++it){
					if (*it == p1)
					{
						it = m_ColorImageSink.erase(it);
						return true;
					}
				}
				return false;

			case KI_IR:
				if (m_IRImageSink.empty())
					return false;
				for (auto it = m_IRImageSink.begin(); it != m_IRImageSink.end(); ++it){
					if (*it == p1)
					{
						it = m_IRImageSink.erase(it);

						if (m_IRImageSink.empty())
							return true;

						return true;
					}
				}
				return false;

			case KI_Depth:
				if (m_DepthImageSink.empty())
					return false;
				for (auto it = m_DepthImageSink.begin(); it != m_DepthImageSink.end(); ++it){
					if (*it == p1) {
						it = m_DepthImageSink.erase(it);
						return true;

						if (m_DepthImageSink.empty())
							return true;
					}	 
				}
				return false;

			default:
				BOOST_ASSERT(!"ImageType Error");
			}
		}

		if (p2)
		{
			for (auto it = m_SkeletonSink.begin(); it != m_SkeletonSink.end(); ++it){
				if (*it == p2)
				{
					m_SkeletonSink.erase(it);

					if (m_SkeletonSink.empty())
						return true;

					return true;
				}
			}
			return false;
		}

	}
	return true;
}




BOOL CKinectSensor::GetDepthImage(SImageStruct &ImgStruct)
{
	INT64 nDepthTime = 0;
	UINT nDepthBufferSize = 0;
	// 深度临帧事件参数
	IDepthFrameArrivedEventArgs* pArgs = nullptr;
	// 深度帧引用
	IDepthFrameReference* pCFrameRef = nullptr;
	// 深度帧
	IDepthFrame* pDepthFrame = nullptr;
	// 帧描述
	IFrameDescription* pFrameDescription = nullptr;
	// 深度帧宽度数据
	int DepthWidth = 0;
	// 深度帧高度数据
	int DepthHeight = 0;

	// 帧缓存大小
	UINT nBufferSize = 0;
	// 帧缓存
	UINT16 *DepthBuffer = nullptr;

	if (!m_pIDepthFrameReader)
		return false;

	// 获取参数
	HRESULT hr = m_pIDepthFrameReader->GetFrameArrivedEventData(m_waitDepthHandle, &pArgs);
	// 获取引用

	if (FAILED(hr))
		return false;

	hr = pArgs->get_FrameReference(&pCFrameRef);
	SafeRelease(pArgs);

	if (FAILED(hr))
		return false;

	// 获取深度帧
	hr = pCFrameRef->AcquireFrame(&pDepthFrame);
	SafeRelease(pCFrameRef);
	
	if (FAILED(hr))
		return false;

	ImgStruct.m_pIFrame = pDepthFrame;

	// 获取帧描述
	hr = pDepthFrame->get_FrameDescription(&pFrameDescription);

	// 获取帧宽度
	if (SUCCEEDED(hr))
	    hr = pFrameDescription->get_Width(&DepthWidth);

	// 获取帧高度
	if (SUCCEEDED(hr))
	   hr = pFrameDescription->get_Height(&DepthHeight);
    
	SafeRelease(pFrameDescription);

	if (SUCCEEDED(hr))
	   hr = pDepthFrame->get_RelativeTime(&nDepthTime);

	if (SUCCEEDED(hr))
	   hr = pDepthFrame->AccessUnderlyingBuffer(&nDepthBufferSize, &DepthBuffer);

	SafeRelease(pDepthFrame);

	if (FAILED(hr))
		return false;

	ImgStruct.m_pBuffer = (void*)DepthBuffer;
	ImgStruct.m_height = DepthHeight;
	ImgStruct.m_width = DepthWidth;
	ImgStruct.m_uTimeStamp = nDepthTime;
	ImgStruct.m_kinectImageType = KI_Depth;

	return true;
}




BOOL CKinectSensor::GetIRImage(SImageStruct &ImgStruct)
{
	INT64 nIRTime = 0;
	UINT nIRBufferSize = 0;
	// 红外帧临帧事件参数
	IInfraredFrameArrivedEventArgs* pArgs = nullptr;
	// 红外帧帧引用
	IInfraredFrameReference* pCFrameRef = nullptr;
	// 红外帧帧
	IInfraredFrame* pInfraredFrame = nullptr;
	// 帧描述
	IFrameDescription* pFrameDescription = nullptr;
	// 红外帧宽度数据
	int InfraredWidth = 0;
	// 红外帧高度数据
	int InfraredHeight = 0;

	// 帧缓存大小
	UINT nBufferSize = 0;
	// 帧缓存
	UINT16 *InfraredBuffer = nullptr;

	// 获取参数
	if (!m_pIInfraredFrameReader)
		return false;

	HRESULT hr = m_pIInfraredFrameReader->GetFrameArrivedEventData(m_waitIRHandle, &pArgs);
	
	// 获取引用
	if (FAILED(hr))
		return false;

	hr = pArgs->get_FrameReference(&pCFrameRef);
	SafeRelease(pArgs);

	if (FAILED(hr))
		return false;

	// 获取红外帧
	hr = pCFrameRef->AcquireFrame(&pInfraredFrame);
	SafeRelease(pCFrameRef);
	
	if (FAILED(hr))
		return false;

	ImgStruct.m_pIFrame = pInfraredFrame;

	// 获取帧描述
	hr = pInfraredFrame->get_FrameDescription(&pFrameDescription);

	// 获取帧宽度 
	if (SUCCEEDED(hr))
	    hr = pFrameDescription->get_Width(&InfraredWidth);

	// 获取帧高度 
	if (SUCCEEDED(hr))
	    hr = pFrameDescription->get_Height(&InfraredHeight);

	SafeRelease(pFrameDescription);

	if (SUCCEEDED(hr))
	    hr = pInfraredFrame->get_RelativeTime(&nIRTime);
	
	if (SUCCEEDED(hr))
	    hr = pInfraredFrame->AccessUnderlyingBuffer(&nIRBufferSize, &InfraredBuffer);

	SafeRelease(pInfraredFrame);

	if (FAILED(hr))
		return false;
	 
	ImgStruct.m_pBuffer = (void*)InfraredBuffer;
	ImgStruct.m_height = InfraredHeight;
	ImgStruct.m_width = InfraredWidth;
	ImgStruct.m_uTimeStamp = nIRTime;
	ImgStruct.m_kinectImageType = KI_IR;

	return true;
}




BOOL CKinectSensor::GetColorImage(SImageStruct &ImgStruct)
{
	INT64 nColorTime = 0;

	// 彩色临帧事件参数
	IColorFrameArrivedEventArgs* pArgs = nullptr;
	// 彩色帧引用
	IColorFrameReference* pCFrameRef = nullptr;
	// 彩色帧
	IColorFrame* pColorFrame = nullptr;
	// 帧描述
	IFrameDescription* pFrameDescription = nullptr;
	// 彩色帧宽度数据
	int ColorWidth = 0;
	// 彩色帧高度数据
	int ColorHeight = 0;
	// 帧格式
	ColorImageFormat imageFormat = ColorImageFormat_None;
	// 帧缓存大小
	UINT nBufferSize = 0;
	// 帧缓存
	RGBQUAD *ColorBuffer = nullptr;

	if (!m_pIColorFrameReader) 
		return false;
	HRESULT hr = m_pIColorFrameReader->GetFrameArrivedEventData(m_waitColorHandle, &pArgs);

	if (FAILED(hr))
		return false;

	// 获取引用
	hr = pArgs->get_FrameReference(&pCFrameRef);
	SafeRelease(pArgs);

	if (FAILED(hr))
		return false;

	// 获取彩色帧
	hr = pCFrameRef->AcquireFrame(&pColorFrame);
	SafeRelease(pCFrameRef);

	if (FAILED(hr))
		return false;

	ImgStruct.m_pIFrame = pColorFrame;
	
	// 获取帧描述
	hr = pColorFrame->get_FrameDescription(&pFrameDescription);
	// 获取帧宽度

	if (SUCCEEDED(hr))
	    hr = pFrameDescription->get_Width(&ColorWidth);

	// 获取帧高度
	if (SUCCEEDED(hr))
	    hr = pFrameDescription->get_Height(&ColorHeight);

	SafeRelease(pFrameDescription);

	// 获取帧格式
	if (SUCCEEDED(hr))
	    hr = pColorFrame->get_RawColorImageFormat(&imageFormat);

	if (SUCCEEDED(hr))
	    hr = pColorFrame->get_RelativeTime(&nColorTime);

	// 在已经是BGRA的情况下 直接获取源数据
	if (SUCCEEDED(hr)) {
		if (imageFormat == ColorImageFormat_Bgra) {
			hr = pColorFrame->AccessRawUnderlyingBuffer(&nBufferSize, reinterpret_cast<BYTE**>(&ColorBuffer));
		}

		else {
			if (m_vOutColorBuffer.empty())
				m_vOutColorBuffer.resize(ColorWidth * ColorHeight);
			ColorBuffer = &m_vOutColorBuffer[0];
			nBufferSize = ColorWidth * ColorHeight * sizeof(RGBQUAD);
			hr          = pColorFrame->CopyConvertedFrameDataToArray(nBufferSize, reinterpret_cast<BYTE*>(ColorBuffer), ColorImageFormat_Bgra);
		}
	}

	SafeRelease(pColorFrame);

	if (FAILED(hr))
		return false;

	ImgStruct.m_pBuffer         = (void*)ColorBuffer;
	ImgStruct.m_height          = ColorHeight;
	ImgStruct.m_width           = ColorWidth;
	ImgStruct.m_uTimeStamp      = nColorTime;
	ImgStruct.m_kinectImageType = KI_COLOR;

	//测试彩色帧数据//
	return true;
}




void CKinectSensor::OnThread(int nEventID)
{
// 	static bool bInit = false;
// 	if (!bInit){
// 		CoInitializeEx(0, 0);
// 		bInit = true;
// 	}

	SImageStruct ImgStruct;

	THREAD::CAutoCriticalSection cs(m_cs);

	switch (nEventID){

		case EVENT_COLOR:
			//获取图像
			if (!m_ColorImageSink.empty())
			{
				//需要判断数据是否为空//
				if (GetColorImage(ImgStruct))
				{

					for (auto it = m_ColorImageSink.begin(); it != m_ColorImageSink.end(); ++it)
						(*it)->OnImage(ImgStruct.m_pBuffer, ImgStruct.m_width, ImgStruct.m_height, ImgStruct.m_pitch, ImgStruct.m_kinectImageType, ImgStruct.m_uTimeStamp);
				}
			}
			break;

		case EVENT_DEPTH:

		{

			if (!m_DepthImageSink.empty())
			{
				if (GetDepthImage(ImgStruct))
				{
					for (auto it = m_DepthImageSink.begin(); it != m_DepthImageSink.end(); ++it)
						(*it)->OnImage(ImgStruct.m_pBuffer, ImgStruct.m_width, ImgStruct.m_height, ImgStruct.m_pitch, ImgStruct.m_kinectImageType, ImgStruct.m_uTimeStamp);

				}


			}
			break;
		}


		case EVENT_IR:
			if (!m_IRImageSink.empty())
			{
				if (GetIRImage(ImgStruct))
				{
					for (auto it = m_IRImageSink.begin(); it != m_IRImageSink.end(); ++it)
							(*it)->OnImage(ImgStruct.m_pBuffer, ImgStruct.m_width, ImgStruct.m_height, ImgStruct.m_pitch, ImgStruct.m_kinectImageType, ImgStruct.m_uTimeStamp);
				}
			}
			break;

		default:
			BOOST_ASSERT(!"thread message error");


	}

#ifdef DEBUG
	int time2 = clock();
	if ((time2 - time1) >9)
	    std::cout << (time2 - time1) << std::endl;

#endif

// 	//获取骨骼
//   if (!m_SkeletonSink.empty())
//   {
// 		for (auto it = m_SkeletonSink.begin(); it != m_SkeletonSink.end(); ++it)
// 			(*it)->OnSkeleton();
// 		
//    }
}




UINT CKinectSensor::GetCount()
{
	if (m_pKinectSensor)
		return 1;
	return 0;
}

BOOL CKinectSensor::Init(UINT uIndex)
{
	Close();

	if (!OpenDevice())
		return false;
	
	std::vector<shared_Handle>  events;

	if (GetColorHandle()) {
		shared_Handle ColorHandle = FromHandle((HANDLE)m_waitColorHandle, false);
		events.push_back(ColorHandle);
	}

	if (GetDepthHandle()) {
		shared_Handle DepthHandle = FromHandle((HANDLE)m_waitDepthHandle, false);
		events.push_back(DepthHandle);
	}

	if (GetIRHandle()) {
		shared_Handle IRHandle = FromHandle((HANDLE)m_waitIRHandle, false);
		events.push_back(IRHandle);
	}

	if (events.empty())
		return false;

	return m_thread.Start(boost::bind(&CKinectSensor::OnThread, this, _1), events);
}




void CKinectSensor::Close()
{
	m_thread.ShutDown();  

	THREAD::CAutoCriticalSection cs(m_cs);
	if (m_pIColorFrameReader) {
		m_pIColorFrameReader->UnsubscribeFrameArrived(m_waitColorHandle);
		m_pIColorFrameReader.reset();
	}

	if (m_pIDepthFrameReader) {
		m_pIDepthFrameReader->UnsubscribeFrameArrived(m_waitDepthHandle);
		m_pIDepthFrameReader.reset();
	}

	if (m_pIInfraredFrameReader) {
		m_pIInfraredFrameReader->UnsubscribeFrameArrived(m_waitIRHandle);
		m_pIInfraredFrameReader.reset();
	}

	if (m_pKinectSensor){
		m_pKinectSensor->Close();
		m_pKinectSensor.reset();
	}
}


BOOL CKinectSensor::ColorPixelToDepthPos(const void * depthBuffer, int depthwidth, int depthheight, const void* colorBuffer, int colorwidth, int colorheight, const sColorDiffIndex &colorDiffIndex, std::vector<nvmath::Vec2f> &CalibPixelPoints, std::vector<nvmath::Vec3f> &vMarkKinectPos)
{

	if (!m_pKinectSensor)
		return false;

	ICoordinateMapper*      coordinateMapper = NULL;
	m_pKinectSensor->get_CoordinateMapper(&coordinateMapper);
	DepthSpacePoint*  pDepthCoordinates = NULL;

	std::vector<DepthSpacePoint> vdepthSpacePoint;
	vdepthSpacePoint.resize(colorwidth * colorheight);
	pDepthCoordinates = &vdepthSpacePoint[0];

/*	BOOL bColorToDepthTransform = ProcessFrame(depthBuffer, depthwidth, depthheight, colorBuffer, colorwidth, colorheight, coordinateMapper, pDepthCoordinates, colorDiffIndex, vMarkKinectPos);*/

	SetCalibPixelPoints(CalibPixelPoints);
	BOOL bColorToDepthTransform = ProcessFrame_4(depthBuffer, depthwidth, depthheight, colorBuffer, colorwidth, colorheight, coordinateMapper, pDepthCoordinates, vMarkKinectPos);

	SafeRelease(coordinateMapper);
	return bColorToDepthTransform;
}

void CKinectSensor::SetCalibPixelPoints(std::vector<nvmath::Vec2f> & vCalibPixelPoints)
{
	m_CalibPixelPoints = vCalibPixelPoints;
}



BOOL CKinectSensor::ProcessFrame(const void* pDepthBuffer, int nDepthWidth, int nDepthHeight, const void* pColorBuffer, int nColorWidth, int nColorHeight, ICoordinateMapper* pCoordinateMapper, DepthSpacePoint* pDepthCoordinates, const sColorDiffIndex &colorDiffIndex, std::vector<nvmath::Vec3f> &vMarkKinectPos)
{
	std::vector<nvmath::Vec3f> vMarkPixelPos;

	// Make sure we've received valid data

	if (!pCoordinateMapper)
		return false;


	HRESULT hr = pCoordinateMapper->MapColorFrameToDepthSpace(nDepthWidth * nDepthHeight, (UINT16*)pDepthBuffer, nColorWidth * nColorHeight, pDepthCoordinates);

	if (FAILED(hr))
		return false;


	//通过搜索获取到标记圆的圆心像素值//
	std::vector<std::vector<nvmath::Vec2i>> vPixelPoint(3);
	std::vector<std::vector<nvmath::Vec3f>> vValidPoint(3);

	int red = 0, blue = 0, green = 0;
	RGBQUAD pColor;

	for (int colorIndex = 0; colorIndex < (nColorWidth*nColorHeight); ++colorIndex)
	{

		pColor = *((RGBQUAD*)pColorBuffer + colorIndex);
		red = pColor.rgbRed;
		blue = pColor.rgbBlue;
		green = pColor.rgbGreen;

		//识别的图形的顺序为:红蓝绿//

		if (red >(green + colorDiffIndex.nRedDiffIndex) && red > (blue + colorDiffIndex.nRedDiffIndex))
			vPixelPoint[0].push_back(nvmath::Vec2i(colorIndex - (int)(colorIndex / nColorWidth) *nColorWidth, (int)(colorIndex / nColorWidth)));

		else if (blue > (red + colorDiffIndex.nBlueDiffIndex) && blue > (green + colorDiffIndex.nBlueDiffIndex))
			vPixelPoint[1].push_back(nvmath::Vec2i(colorIndex - (int)(colorIndex / nColorWidth) *nColorWidth, (int)(colorIndex / nColorWidth)));

		else if (green > (red + colorDiffIndex.nGreenDiffIndex) && green > (blue + colorDiffIndex.nGreenDiffIndex))
			vPixelPoint[2].push_back(nvmath::Vec2i(colorIndex - (int)(colorIndex / nColorWidth) *nColorWidth, (int)(colorIndex / nColorWidth)));
	}


	if (vPixelPoint[0].empty() || vPixelPoint[1].empty() || vPixelPoint[2].empty())
		return false;

	int depthX = 0, depthY = 0;
	int depth = 0;
	for (int k = 0; k < vPixelPoint.size(); ++k)
	{
		int nvalidIndex = 0;
		nvmath::Vec3f CircleCenter(0, 0, 0);

		for (int i = 0; i < vPixelPoint[k].size(); ++i)
		{
			int colorIndex = vPixelPoint[k][i][1] * nColorWidth + vPixelPoint[k][i][0];
			DepthSpacePoint p = pDepthCoordinates[colorIndex];
			if (p.X != -std::numeric_limits<float>::infinity() && p.Y != -std::numeric_limits<float>::infinity())
			{
				depthX = static_cast<int>(p.X + 0.5f);
				depthY = static_cast<int>(p.Y + 0.5f);

				if ((depthX >= 0 && depthX < nDepthWidth) && (depthY >= 0 && depthY < nDepthHeight))
					depth = *((UINT16*)pDepthBuffer + depthX + (depthY * nDepthWidth));

				if (depth != 0) {
					nvalidIndex++;
					vValidPoint[k].push_back(nvmath::Vec3f(depthX, depthY, depth));
					CircleCenter[0] += depthX;
					CircleCenter[1] += depthY;
					CircleCenter[2] += depth;
					depth = 0;
				}
			}
		}


		//delete for contemporary//
// 		if (!nvalidIndex)
// 			return false;
// 
// 		CircleCenter[0] = (int)(CircleCenter[0] / nvalidIndex);
// 		CircleCenter[1] = (int)(CircleCenter[1] / nvalidIndex);
// 		CircleCenter[2] = (int)(CircleCenter[2] / nvalidIndex);
// 
// 		ReviseAvgPixel(vValidPoint[k], CircleCenter);
// 
// 
// /*		vMarkPixelPos.push_back(CircleCenter);*/
	}

// 	if (vMarkPixelPos.size() != 3)
// 		return false;

/*#ifdef DEBUG*/
	vMarkPixelPos.clear();
	vMarkPixelPos.resize(4);
	vMarkPixelPos[0] = nvmath::Vec3f(149, 307, 1161);
	vMarkPixelPos[1] = nvmath::Vec3f(443, 309, 1137);
	vMarkPixelPos[2] = nvmath::Vec3f(376, 251, 1770);
	vMarkPixelPos[3] = nvmath::Vec3f(193, 250, 1809);
/*#endif*/


	vMarkKinectPos.resize(vMarkPixelPos.size());
	for (int i = 0; i < vMarkPixelPos.size(); i++)
	{
		getKinectPos(nDepthWidth, nDepthHeight,vMarkPixelPos[i], vMarkKinectPos[i]);
	}

// 	nvmath::Vec3f vec1 = vMarkKinectPos[0] - vMarkKinectPos[1];
// 	nvmath::Vec3f vec2 = vMarkKinectPos[2] - vMarkKinectPos[1];
// 	nvmath::Vec3f markkinectPos = vec1 + vec2 + vMarkKinectPos[1];
// 	vMarkKinectPos.push_back(markkinectPos);

	return true;
}

void CKinectSensor::ReviseAvgPixel(std::vector<nvmath::Vec3f> & validPoints, nvmath::Vec3f &avgPixel)
{
		for (auto it = validPoints.begin(); it != validPoints.end(); ++it)
		{
			if (fabs((*it)[0] - avgPixel[0]) > m_RegRadius || fabs((*it)[1] - avgPixel[1]) > m_RegRadius)
 				validPoints.erase(it);
		}

		avgPixel = nvmath::Vec3f(0, 0, 0);
		for (int i = 0; i < validPoints.size(); i++) {
			avgPixel[0] += validPoints[i][0];
			avgPixel[1] += validPoints[i][1];
			avgPixel[2] += validPoints[i][2];
		}
		avgPixel = avgPixel / validPoints.size();
}


void  CKinectSensor::getKinectPos(const int & ImgWidth, const int & ImgHeight,const nvmath::Vec3f& PixelPos, nvmath::Vec3f & KinectPos)
{
	KinectPos[0] = (PixelPos[0] - ImgWidth/2) *PixelPos[2] / foc;
	KinectPos[1] = (PixelPos[1] - ImgHeight/2)  *PixelPos[2] / foc;
	KinectPos[2] = PixelPos[2];
}

//V2
//将深度手势轮廓转换成彩色轮廓//
BOOL CKinectSensor::ProcessFrame_1(const void* pDepthBuffer, int nDepthWidth, int nDepthHeight, const void* pColorBuffer, int nColorWidth, int nColorHeight, ICoordinateMapper*pCoordinateMapper, ColorSpacePoint* ColorCoordinates, nvmath::Vec2f & vColorPixelPoint)
{
	if (!pCoordinateMapper)
		return false;

	HRESULT hr = pCoordinateMapper->MapDepthFrameToColorSpace(nDepthWidth * nDepthHeight, (UINT16*)pDepthBuffer, nDepthWidth * nDepthHeight , ColorCoordinates);

	if (FAILED(hr))
		return false;
 
	UINT testPos1 = vColorPixelPoint[1] * nDepthWidth + vColorPixelPoint[0];

	ColorSpacePoint testp1 = ColorCoordinates[testPos1];

	if (testp1.X != -std::numeric_limits<float>::infinity() && testp1.Y != -std::numeric_limits<float>::infinity()) {
		int testdepthX = static_cast<int>(testp1.X + 0.5);
		int testdepthY = static_cast<int>(testp1.Y + 0.5);

		int testm = 10;

	}
}

//V2  
//color 2 depth//
BOOL CKinectSensor::ProcessFrame_2(const void* pDepthBuffer, int nDepthWidth, int nDepthHeight, const void* pColorBuffer, int nColorWidth, int nColorHeight, ICoordinateMapper*pCoordinateMapper, DepthSpacePoint* DepthCoordinates,  const std::vector<nvmath::Vec2f> & vColorPixelPoint, std::vector<nvmath::Vec3f> & vKinectPos)
{
	if (!pCoordinateMapper)
		return false;

	HRESULT hr = pCoordinateMapper->MapColorFrameToDepthSpace(nDepthWidth * nDepthHeight, (UINT16*)pDepthBuffer, nColorWidth * nColorHeight, DepthCoordinates);

	if (FAILED(hr))
		return false;

	vKinectPos.resize(vColorPixelPoint.size());

	for (int i = 0; i < vColorPixelPoint.size(); ++i)
	{
		if (vColorPixelPoint[i][0] == 0 && vColorPixelPoint[i][1] == 0)
			break;

		int colorIndex = vColorPixelPoint[i][1] * nColorWidth + vColorPixelPoint[i][0];
		
		DepthSpacePoint p = DepthCoordinates[colorIndex];
		nvmath::Vec3f kinectPos(0, 0, 0);
		BOOL bflag = GetKinectPos(p, nDepthWidth, nDepthHeight, pDepthBuffer, kinectPos);
		int Count = 0, Count1 = 0;
		while (!bflag) {
			colorIndex = (vColorPixelPoint[i][1] + Count) * nColorWidth + (vColorPixelPoint[i][0] + Count1);
			DepthSpacePoint p = DepthCoordinates[colorIndex];
			bflag = GetKinectPos(p, nDepthWidth, nDepthHeight, pDepthBuffer, kinectPos);
			Count++;
			if (vColorPixelPoint[i][1] + Count > nColorHeight - 50 || Count > 50);
			Count = -1;
			if (Count = -1) {
				Count1++;
				if (Count1 > 50)
					return false;
			}
		}
		vKinectPos[i] = kinectPos;


		
	}

	std::vector<nvmath::Vec3f>  vPixelPos(vKinectPos.size());
	for (int i = 0; i < vKinectPos.size(); i++) {
		getPixelPos(vKinectPos[i], vPixelPos[i], nDepthWidth, nDepthHeight);
	}

	m_PICalib->SetScreenPos_1(vKinectPos, vPixelPos);


	return (vKinectPos.size() == 4);
}


//V2
BOOL CKinectSensor::GetKinectPos(const DepthSpacePoint p, const int &nDepthWidth, const int & nDepthHeight, const void* pDepthBuffer, nvmath::Vec3f & kinectPos)
{
	int depthX = 0, depthY = 0;
	float depth = 0;
	if (p.X != -std::numeric_limits<float>::infinity() && p.Y != -std::numeric_limits<float>::infinity())
	{
		depthX = static_cast<int>(p.X + 0.5f);
		depthY = static_cast<int>(p.Y + 0.5f);

		if ((depthX >= 0 && depthX < nDepthWidth) && (depthY >= 0 && depthY < nDepthHeight))
			depth = *((UINT16*)pDepthBuffer + depthX + (depthY * nDepthWidth));

		kinectPos = nvmath::Vec3f(depthX, depthY, depth);
		return depth != 0;
	}
}

//V2

//Input:手势投影到大屏幕上的深度像素点//
//Output:标定点的深度像素点//
BOOL CKinectSensor::ProcessFrame_3(const void* pDepthBuffer, int nDepthWidth, int nDepthHeight, const void* pColorBuffer, int nColorWidth, int nColorHeight, ICoordinateMapper*pCoordinateMapper, DepthSpacePoint* DepthCoordinates,ColorSpacePoint* ColorCoordinates, const nvmath::Vec3f & vScreemDepthPixelPos, nvmath::Vec3f & vMarkDepthPixelPoint)
{

	std::vector<nvmath::Vec2i> vPixelPoint(1);
	std::vector<nvmath::Vec3f> vValidPoint(1);

	//1:depth 2 color//
	nvmath::Vec2f vColorPixelPoint(0, 0);
	ProcessFrame_1(pDepthBuffer, nDepthWidth, nDepthHeight, pColorBuffer, nColorWidth, nColorHeight, pCoordinateMapper, ColorCoordinates, vColorPixelPoint);

	//设置偏离的识别区域的像素点：25//
	int MinPixelX = (vColorPixelPoint[0] - 25) > 0            ? (vColorPixelPoint[0] - 25):  0;
	int MaxPixelX = (vColorPixelPoint[0] + 25) < nColorWidth  ? (vColorPixelPoint[0] + 25) : 0;
	int MinPixelY = (vColorPixelPoint[1] - 25) > 0            ? (vColorPixelPoint[1] - 25) : 0;
	int MaxPixelY = (vColorPixelPoint[1] + 25) < nColorHeight ? (vColorPixelPoint[1] + 25) : 0;

	//2: get Color Center//
	int red = 0, blue = 0, green = 0;
	RGBQUAD pColor;

	for (int colorIndex = 0; colorIndex < (nColorWidth*nColorHeight); ++colorIndex)
	{

		pColor = *((RGBQUAD*)pColorBuffer + colorIndex);
		red = pColor.rgbRed;
		blue = pColor.rgbBlue;
		green = pColor.rgbGreen;

		//识别的图形的顺序为:红//

		sColorDiffIndex s;
		if (red >(green + s.nRedDiffIndex) && red > (blue + s.nRedDiffIndex))
			vPixelPoint.push_back(nvmath::Vec2i(colorIndex - (int)(colorIndex / nColorWidth) *nColorWidth, (int)(colorIndex / nColorWidth)));

	}

	//3: color center 2 depth center//
	if (vPixelPoint.empty())
		return false;

	int depthX = 0, depthY = 0;
	int depth = 0;
	int nvalidIndex = 0;
	nvmath::Vec3f CircleCenter(0, 0, 0);
	for (int k = 0; k < vPixelPoint.size(); ++k)
	{
		int colorIndex = vPixelPoint[k][1] * nColorWidth + vPixelPoint[k][0];
		DepthSpacePoint p = DepthCoordinates[colorIndex];
		if (p.X != -std::numeric_limits<float>::infinity() && p.Y != -std::numeric_limits<float>::infinity())
		{
			depthX = static_cast<int>(p.X + 0.5f);
			depthY = static_cast<int>(p.Y + 0.5f);

			if ((depthX >= 0 && depthX < nDepthWidth) && (depthY >= 0 && depthY < nDepthHeight))
				depth = *((UINT16*)pDepthBuffer + depthX + (depthY * nDepthWidth));

			if (depth != 0) {
				nvalidIndex++;
				vValidPoint.push_back(nvmath::Vec3f(depthX, depthY, depth));
				CircleCenter[0] += depthX;
				CircleCenter[1] += depthY;
				CircleCenter[2] += depth;
				depth = 0;
			}
		}
	}

	if (!nvalidIndex)
		return false;

	CircleCenter[0] = (int)(CircleCenter[0] / nvalidIndex);
	CircleCenter[1] = (int)(CircleCenter[1] / nvalidIndex);
	CircleCenter[2] = (int)(CircleCenter[2] / nvalidIndex);

	ReviseAvgPixel(vValidPoint, CircleCenter);
	vMarkDepthPixelPoint = CircleCenter;

}

void CKinectSensor::getPixelPos(const nvmath::Vec3f & pos, nvmath::Vec3f &pixelPoint, int DepthWidth, int DepthHeight)
{
	pixelPoint[0] = pos[0] * foc / pos[2] + DepthWidth / 2;
	pixelPoint[1] = pos[1] * foc / pos[2] + DepthHeight / 2;
	pixelPoint[2] = pos[2];
}

BOOL CKinectSensor::ProcessFrame_4(const void* pDepthBuffer, int nDepthWidth, int nDepthHeight, const void* pColorBuffer, int nColorWidth, int nColorHeight, ICoordinateMapper*pCoordinateMapper, DepthSpacePoint* pDepthCoordinates, std::vector<nvmath::Vec3f> &vMarkKinectPos)
{
	vMarkKinectPos.clear();

	std::vector<nvmath::Vec3f> vMarkPixelPos;

	// Make sure we've received valid data

	if (!pCoordinateMapper)
		return false;

	HRESULT hr = pCoordinateMapper->MapColorFrameToDepthSpace(nDepthWidth * nDepthHeight, (UINT16*)pDepthBuffer, nColorWidth * nColorHeight, pDepthCoordinates);

	if (FAILED(hr))
		return false;

	int depthX = 0, depthY = 0;
	float depth = 0;
	for (int i = 0; i < m_CalibPixelPoints.size(); i++)
	{
		int colorIndex = m_CalibPixelPoints[i][1] * nColorWidth + m_CalibPixelPoints[i][0];
		DepthSpacePoint p = pDepthCoordinates[colorIndex];
		if (p.X != -std::numeric_limits<float>::infinity() && p.Y != -std::numeric_limits<float>::infinity())
		{
			depthX = static_cast<int>(p.X + 0.5f);
			depthY = static_cast<int>(p.Y + 0.5f);

			if ((depthX >= 0 && depthX < nDepthWidth) && (depthY >= 0 && depthY < nDepthHeight))
				depth = *((UINT16*)pDepthBuffer + depthX + (depthY * nDepthWidth));
        }

		nvmath::Vec2f vpixelPoints = nvmath::Vec2f(depthX, depthY);

		BOOL bdepth = true;
		if (depth == 0)
			BOOL bdepth = GetDepth(pCoordinateMapper, pDepthCoordinates, nColorWidth, nColorHeight, vpixelPoints, depth, nDepthWidth, nDepthHeight, pDepthBuffer);

		if (bdepth)
			vMarkPixelPos.push_back(nvmath::Vec3f(vpixelPoints[0], vpixelPoints[1], depth));

	}

	vMarkKinectPos.resize(vMarkPixelPos.size());
	for (int i = 0; i < vMarkPixelPos.size(); i++)
		getKinectPos(nDepthWidth, nDepthHeight, vMarkPixelPos[i], vMarkKinectPos[i]);

	return true;
}


BOOL CKinectSensor::GetDepth(ICoordinateMapper*pCoordinateMapper, DepthSpacePoint* pDepthCoordinates, const int & nColorWidth, const int & nColorHeight, nvmath::Vec2f & vpixelPoints, float &depth, const int & nDepthWidth, const int & nDepthHeight, const void* pDepthBuffer)
{
	std::vector<nvmath::Vec2f> vDir(4);
	int depthX = 0, depthY = 0;
	float fdepth = 0;
	nvmath::Vec2f vUp(0, 1), vDown(0, -1), vLeft(-1, 0), vRight(1, 0);
	vDir[0] = vUp;
	vDir[1] = vDown;
	vDir[2] = vLeft;
	vDir[3] = vRight;
	int length = 1;
	int k = 0;

	float deta = 0;

	while (depth == 0 && deta < 4) {
		
		if (k == 4) {
			k = 0;
			deta++;
		}
			

		vpixelPoints = vpixelPoints + deta * vDir[k];
		int colorIndex = vpixelPoints[1] * nColorWidth + vpixelPoints[0];
		DepthSpacePoint p = pDepthCoordinates[colorIndex];

		if (p.X != -std::numeric_limits<float>::infinity() && p.Y != -std::numeric_limits<float>::infinity())
		{
			depthX = static_cast<int>(p.X + 0.5f);
			depthY = static_cast<int>(p.Y + 0.5f);

			if ((depthX >= 0 && depthX < nDepthWidth) && (depthY >= 0 && depthY < nDepthHeight)) 
				fdepth = *((UINT16*)pDepthBuffer + depthX + (depthY * nDepthWidth));
		}

	}
	
	return deta < 4;

}
