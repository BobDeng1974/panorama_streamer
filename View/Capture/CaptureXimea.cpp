#include "CaptureXimea.h"
#include "CaptureProp.h"

#include <iostream>


//
// Enumerate connected devices
//

void CaptureXimea::init()
{
    xiGetNumberDevices( &numDevices );
	std::cout << "Num Devices: " << numDevices << std::endl;
    hmv = NULL;
    timeout = 0;
    memset( &m_xiimage, 0, sizeof( XI_IMG ) );
}


#define HandleXiResult( res ) if ( res!=XI_OK )  goto error;
// Initialize camera input
bool CaptureXimea::open( int wIndex )
{
    int mvret = XI_OK;

    if( numDevices == 0 )
        return false;

    if( (mvret = xiOpenDevice( wIndex, &hmv)) != XI_OK )
    {
        errMsg( "Open XI_DEVICE failed", mvret );
        return false;
    }

    int width   = 0;
    int height  = 0;
    int isColor = 0;

    // always use auto exposure/gain
    //mvret = xiSetParamInt( hmv, XI_PRM_AEAG, 1 );
    HandleXiResult( mvret );

    mvret = xiGetParamInt( hmv, XI_PRM_WIDTH, &width );
    HandleXiResult( mvret );

    mvret = xiGetParamInt( hmv, XI_PRM_HEIGHT, &height );
    HandleXiResult( mvret );

    mvret = xiGetParamInt( hmv, XI_PRM_IMAGE_IS_COLOR, &isColor );
    HandleXiResult( mvret );

    if( isColor ) // for color cameras
    {
        // default image format RGB24
        //mvret = xiSetParamInt( hmv, XI_PRM_IMAGE_DATA_FORMAT, XI_RGB24 );
		
		//
		// change to RAW8 - Ram
		//
		mvret = xiSetParamInt( hmv, XI_PRM_IMAGE_DATA_FORMAT, XI_RAW8 );
        HandleXiResult( mvret );

		xiSetParamInt( hmv, XI_PRM_AUTO_BANDWIDTH_CALCULATION, XI_OFF );

        // always use auto white balance for color cameras
        mvret = xiSetParamInt( hmv, XI_PRM_AUTO_WB, 1 );
        HandleXiResult( mvret );

		// 
		// Set the down sampling to non interpolated for better frame rate - Ram
		//
		xiSetParamInt( hmv, XI_PRM_DOWNSAMPLING_TYPE, XI_SKIPPING );

		float  fval = 5;

		xiGetParamFloat( hmv, XI_PRM_GAIN, &fval );

		std::cout << "Width: " << width << " Height " << height << std::endl;
		std::cout << "Created Image " << std::endl;
    }
    else // for mono cameras
    {
        // default image format MONO8
        mvret = xiSetParamInt( hmv, XI_PRM_IMAGE_DATA_FORMAT, XI_MONO8 );
        HandleXiResult( mvret )
    }

    //default capture timeout 10s
    timeout = 10000;

   
    return true;

error:
    errMsg( "Open XI_DEVICE failed", mvret );
    xiCloseDevice( hmv );
    hmv = NULL;
    return false;
}

bool CaptureXimea::start()
{
	 int mvret = XI_OK;
	//default capture timeout 10s
	timeout = 10000;

	mvret = xiStartAcquisition( hmv );
	if( mvret != XI_OK )
	{
		errMsg( "StartAcquisition XI_DEVICE failed", mvret );
		goto error;
	}
	return true;

error:
	errMsg( "Open XI_DEVICE failed", mvret );
	xiCloseDevice( hmv );
	hmv = NULL;
	return false;
}

void CaptureXimea::close()
{
    if( hmv )
    {
        xiStopAcquisition( hmv );
        xiCloseDevice( hmv );
    }
    hmv = NULL;
}


bool CaptureXimea::grabFrame(QImage& imageFrame)
{
    //memset( &m_xiimage, 0, sizeof( XI_IMG ) );
    m_xiimage.size = sizeof( XI_IMG );
    int mvret = xiGetImage( hmv, timeout, &m_xiimage );
	HandleXiResult( mvret );
	//std::cout << "Got Image " << std::endl;
	/*
    if( mvret == MM40_ACQUISITION_STOPED )
    {
        xiStartAcquisition( hmv );
        mvret = xiGetImage( hmv, timeout, &m_xiimage );
    }
    if( mvret != XI_OK )
    {
        errMsg( "Error during GetImage", mvret );
        return false;
    }
	*/

	bool retval;
error:
	if( mvret != XI_OK )
	{
		errMsg( "Camera Get Frame Error ", mvret );
		return false;
	}
	else
		return true;
}


void CaptureXimea::retrieveMatFrame(int channel, QImage& imageFrame)
{
    // update cvImage after format has changed
    //resetCvImage();

	//std::cout << "Bits Per Pixel: " << getBpp() << " Format: " << m_xiimage.frm << std::endl;

    // copy pixel data
    switch( m_xiimage.frm )
    {
		case XI_MONO8       :
		case XI_RAW8        : 
			imageFrame = QImage((uchar *)m_xiimage.bp, m_xiimage.width, m_xiimage.height, QImage::Format::Format_Grayscale8);
			//memcpy( matFrame.data, m_xiimage.bp, m_xiimage.width*m_xiimage.height );
			break;
		case XI_MONO16      :
		case XI_RAW16       : 
			imageFrame = QImage((uchar *)m_xiimage.bp, m_xiimage.width, m_xiimage.height, QImage::Format::Format_RGB16);
			//memcpy( matFrame.data, m_xiimage.bp, m_xiimage.width*m_xiimage.height*sizeof( WORD ) );
			break;
		case XI_RGB24       :
		case XI_RGB_PLANAR  : 
			imageFrame = QImage((uchar *)m_xiimage.bp, m_xiimage.width, m_xiimage.height, QImage::Format::Format_RGB888);
			//memcpy( matFrame.data, m_xiimage.bp, m_xiimage.width*m_xiimage.height*3 );
			break;
		case XI_RGB32       : 
			imageFrame = QImage((uchar *)m_xiimage.bp, m_xiimage.width, m_xiimage.height, QImage::Format::Format_RGBA8888);
			//memcpy( matFrame.data, m_xiimage.bp, m_xiimage.width*m_xiimage.height*4 ); break;
			break;
		default: break;
	}
}


bool CaptureXimea::retrieveFrame(int channel, QImage& imageFrame)
{
	retrieveMatFrame(channel, imageFrame);

	return true;
}



void CaptureXimea::reset(QImage& imageFrame)
{

    int width = 0, height = 0, format = 0;
    xiGetParamInt( hmv, XI_PRM_WIDTH, &width );
    xiGetParamInt( hmv, XI_PRM_HEIGHT, &height );
    xiGetParamInt( hmv, XI_PRM_IMAGE_DATA_FORMAT, &format );
	//std::cout << "Reset CV " << width << " " << height << " Format " << format << std::endl;
	//std::cout << "Frame Channels " << m_frame->nChannels << " " << m_xiimage.frm << std::endl;

	/*
    if( (int) m_xiimage.width != width || (int) m_xiimage.height != height || m_xiimage.frm != ( XI_IMG_FORMAT ) format )
    {
	*/
		m_xiimage.frm   = (XI_IMG_FORMAT) format;
		m_xiimage.width = width;
		m_xiimage.height = height;

        switch( m_xiimage.frm )
        {
        case XI_MONO8       :
        case XI_RAW8        : 
			imageFrame = QImage(m_xiimage.width, m_xiimage.height, QImage::Format::Format_Grayscale8);
			//matFrame.create( m_xiimage.width, m_xiimage.height, CV_8UC1 );
			break;
        case XI_MONO16      :
        case XI_RAW16       : 
			imageFrame = QImage(m_xiimage.width, m_xiimage.height, QImage::Format::Format_RGB16);
			//matFrame.create( m_xiimage.width, m_xiimage.height, CV_16UC1 );
			break;
        case XI_RGB24       :
        case XI_RGB_PLANAR  : 
			imageFrame = QImage(m_xiimage.width, m_xiimage.height, QImage::Format::Format_RGB888);
			//matFrame.create( m_xiimage.width, m_xiimage.height, CV_8UC3 );
			break;
        case XI_RGB32       : 
			imageFrame = QImage(m_xiimage.width, m_xiimage.height, QImage::Format::Format_RGBA8888);
			//matFrame.create( m_xiimage.width, m_xiimage.height, CV_16UC4 );
			break;
        default :
            return;
        }
		/*
    }
	*/

}

double CaptureXimea::getProperty( int property_id )
{
    if(hmv == NULL)
        return 0;

    int ival = 0;
    float fval = 0;

    switch( property_id )
    {
    // OCV parameters
    case CV_CAP_PROP_POS_FRAMES   : return (double) m_xiimage.nframe;
    case CV_CAP_PROP_FRAME_WIDTH  : xiGetParamInt( hmv, XI_PRM_WIDTH, &ival); return ival;
    case CV_CAP_PROP_FRAME_HEIGHT : xiGetParamInt( hmv, XI_PRM_HEIGHT, &ival); return ival;
    case CV_CAP_PROP_FPS          : xiGetParamFloat( hmv, XI_PRM_FRAMERATE, &fval); return fval;
    case CV_CAP_PROP_GAIN         : xiGetParamFloat( hmv, XI_PRM_GAIN, &fval); return fval;
    case CV_CAP_PROP_EXPOSURE     : xiGetParamInt( hmv, XI_PRM_EXPOSURE, &ival); return ival;

    // XIMEA camera properties
    case CV_CAP_PROP_XI_DOWNSAMPLING  : xiGetParamInt( hmv, XI_PRM_DOWNSAMPLING, &ival); return ival;
    case CV_CAP_PROP_XI_DATA_FORMAT   : xiGetParamInt( hmv, XI_PRM_IMAGE_DATA_FORMAT, &ival); return ival;
    case CV_CAP_PROP_XI_OFFSET_X      : xiGetParamInt( hmv, XI_PRM_OFFSET_X, &ival); return ival;
    case CV_CAP_PROP_XI_OFFSET_Y      : xiGetParamInt( hmv, XI_PRM_OFFSET_Y, &ival); return ival;
    case CV_CAP_PROP_XI_TRG_SOURCE    : xiGetParamInt( hmv, XI_PRM_TRG_SOURCE, &ival); return ival;
    case CV_CAP_PROP_XI_GPI_SELECTOR  : xiGetParamInt( hmv, XI_PRM_GPI_SELECTOR, &ival); return ival;
    case CV_CAP_PROP_XI_GPI_MODE      : xiGetParamInt( hmv, XI_PRM_GPI_MODE, &ival); return ival;
    case CV_CAP_PROP_XI_GPI_LEVEL     : xiGetParamInt( hmv, XI_PRM_GPI_LEVEL, &ival); return ival;
    case CV_CAP_PROP_XI_GPO_SELECTOR  : xiGetParamInt( hmv, XI_PRM_GPO_SELECTOR, &ival); return ival;
    case CV_CAP_PROP_XI_GPO_MODE      : xiGetParamInt( hmv, XI_PRM_GPO_MODE, &ival); return ival;
    case CV_CAP_PROP_XI_LED_SELECTOR  : xiGetParamInt( hmv, XI_PRM_LED_SELECTOR, &ival); return ival;
    case CV_CAP_PROP_XI_LED_MODE      : xiGetParamInt( hmv, XI_PRM_LED_MODE, &ival); return ival;
    case CV_CAP_PROP_XI_AUTO_WB       : xiGetParamInt( hmv, XI_PRM_AUTO_WB, &ival); return ival;
    case CV_CAP_PROP_XI_AEAG          : xiGetParamInt( hmv, XI_PRM_AEAG, &ival); return ival;
    case CV_CAP_PROP_XI_EXP_PRIORITY  : xiGetParamFloat( hmv, XI_PRM_EXP_PRIORITY, &fval); return fval;
    case CV_CAP_PROP_XI_AE_MAX_LIMIT  : xiGetParamInt( hmv, XI_PRM_EXP_PRIORITY, &ival); return ival;
    case CV_CAP_PROP_XI_AG_MAX_LIMIT  : xiGetParamFloat( hmv, XI_PRM_AG_MAX_LIMIT, &fval); return fval;
    case CV_CAP_PROP_XI_AEAG_LEVEL    : xiGetParamInt( hmv, XI_PRM_AEAG_LEVEL, &ival); return ival;
    case CV_CAP_PROP_XI_TIMEOUT       : return timeout;

    }
    return 0;
}


bool CaptureXimea::setProperty( int property_id, double value )
{
    int ival = (int) value;
    float fval = (float) value;
	int errval = 0;

    int mvret = XI_OK;
	float fps = 0;
	float min_fps, max_fps;
    switch( property_id )
    {
	case RISINGEDGE:
		mMode = RISINGEDGE;
		// set acquisition mode
		mvret = xiSetParamInt( hmv, XI_PRM_TRG_SOURCE, XI_TRG_EDGE_RISING );
		HandleXiResult( mvret );
		
		// select input pin 1 mode
		mvret = xiSetParamInt( hmv, XI_PRM_GPI_SELECTOR, 1 );
		HandleXiResult( mvret );

		// set GPI used as trigger
		xiSetParamInt( hmv, XI_PRM_GPI_MODE, XI_GPI_TRIGGER );
		HandleXiResult( mvret );

		break;
	case FRAMERATE:
		mvret = xiSetParamInt( hmv,XI_PRM_ACQ_TIMING_MODE, XI_ACQ_TIMING_MODE_FRAME_RATE );
		mMode = FRAMERATE;
		break;
	case FREERUN:
		mMode = FREERUN;
		mvret = xiSetParamInt( hmv,XI_PRM_ACQ_TIMING_MODE, XI_ACQ_TIMING_MODE_FREE_RUN );
		HandleXiResult( mvret );
		break;

    // OCV parameters
    case CV_CAP_PROP_FRAME_WIDTH  : 
		mvret = xiSetParamInt( hmv, XI_PRM_WIDTH, ival ); 
		HandleXiResult( mvret );
		break;
    case CV_CAP_PROP_FRAME_HEIGHT : 
		mvret = xiSetParamInt( hmv, XI_PRM_HEIGHT, ival ); 
		HandleXiResult( mvret );
		break;
    case CV_CAP_PROP_FPS          : 
		if( mMode == FRAMERATE )
		{
			std::cout << "Camera is set to Frame Rate" << std::endl;
			xiGetParamFloat( hmv, XI_PRM_FRAMERATE XI_PRM_INFO_MIN, &min_fps );
			xiGetParamFloat( hmv, XI_PRM_FRAMERATE XI_PRM_INFO_MAX, &max_fps );
			xiSetParamInt( hmv ,XI_PRM_ACQ_TIMING_MODE, XI_ACQ_TIMING_MODE_FRAME_RATE );
		
			mvret = xiSetParamFloat( hmv, XI_PRM_FRAMERATE, fval ); 
			HandleXiResult( mvret );
			fps = xiGetParamFloat( hmv, XI_PRM_FRAMERATE, &fval );
		}
		//std::cout << "Frame Rate " << fval << std::endl;
		break;
    case CV_CAP_PROP_GAIN         : 
		mvret = xiSetParamFloat( hmv, XI_PRM_GAIN, fval );
		HandleXiResult( mvret );
		//std::cout << "mvret " << mvret << std::endl;
		break;
    case CV_CAP_PROP_EXPOSURE     : 
		mvret = xiSetParamInt( hmv, XI_PRM_EXPOSURE, ival ); 
		HandleXiResult( mvret );
		break;
    // XIMEA camera properties
    case CV_CAP_PROP_XI_DOWNSAMPLING  : 
		mvret = xiSetParamInt( hmv, XI_PRM_DOWNSAMPLING, ival ); 
		HandleXiResult( mvret );
		break;
    case CV_CAP_PROP_XI_DATA_FORMAT   : 
		mvret = xiSetParamInt( hmv, XI_PRM_IMAGE_DATA_FORMAT, ival ); 
		HandleXiResult( mvret );
		break;
    case CV_CAP_PROP_XI_OFFSET_X      : 
		mvret = xiSetParamInt( hmv, XI_PRM_OFFSET_X, ival ); 
		HandleXiResult( mvret );
		break;
    case CV_CAP_PROP_XI_OFFSET_Y      : 
		mvret = xiSetParamInt( hmv, XI_PRM_OFFSET_Y, ival ); 
		HandleXiResult( mvret );
		break;
    case CV_CAP_PROP_XI_TRG_SOURCE    : 
		mvret = xiSetParamInt( hmv, XI_PRM_TRG_SOURCE, ival ); 
		HandleXiResult( mvret );
		break;
    case CV_CAP_PROP_XI_GPI_SELECTOR  : 
		mvret = xiSetParamInt( hmv, XI_PRM_GPI_SELECTOR, ival ); 
		HandleXiResult( mvret );
		break;
    case CV_CAP_PROP_XI_TRG_SOFTWARE  : 
		mvret = xiSetParamInt( hmv, XI_PRM_TRG_SOURCE, 1 ); 
		HandleXiResult( mvret );
		break;
    case CV_CAP_PROP_XI_GPI_MODE      : 
		mvret = xiSetParamInt( hmv, XI_PRM_GPI_MODE, ival ); 
		HandleXiResult( mvret );
		break;
    case CV_CAP_PROP_XI_GPI_LEVEL     : 
		mvret = xiSetParamInt( hmv, XI_PRM_GPI_LEVEL, ival ); 
		HandleXiResult( mvret );
		break;
    case CV_CAP_PROP_XI_GPO_SELECTOR  : 
		mvret = xiSetParamInt( hmv, XI_PRM_GPO_SELECTOR, ival );
		HandleXiResult( mvret );
		break;
    case CV_CAP_PROP_XI_GPO_MODE      : 
		mvret = xiSetParamInt( hmv, XI_PRM_GPO_MODE, ival ); 
		HandleXiResult( mvret );
		break;
    case CV_CAP_PROP_XI_LED_SELECTOR  : 
		mvret = xiSetParamInt( hmv, XI_PRM_LED_SELECTOR, ival );
		HandleXiResult( mvret );
		break;
    case CV_CAP_PROP_XI_LED_MODE      : 
		mvret = xiSetParamInt( hmv, XI_PRM_LED_MODE, ival ); 
		HandleXiResult( mvret );
		break;
    case CV_CAP_PROP_XI_AUTO_WB       : 
		mvret = xiSetParamInt( hmv, XI_PRM_AUTO_WB, ival );
		HandleXiResult( mvret );
		break;
    case CV_CAP_PROP_XI_MANUAL_WB     : 
		mvret = xiSetParamInt( hmv, XI_PRM_LED_MODE, ival ); 
		HandleXiResult( mvret );
		break;
    case CV_CAP_PROP_XI_AEAG          : 
		mvret = xiSetParamInt( hmv, XI_PRM_AEAG, ival ); 
		HandleXiResult( mvret );
		break;
    case CV_CAP_PROP_XI_EXP_PRIORITY  :
		mvret = xiSetParamFloat( hmv, XI_PRM_EXP_PRIORITY, fval ); 
		HandleXiResult( mvret );
		break;
    case CV_CAP_PROP_XI_AE_MAX_LIMIT  : 
		mvret = xiSetParamInt( hmv, XI_PRM_EXP_PRIORITY, ival ); 
		HandleXiResult( mvret );
		break;
    case CV_CAP_PROP_XI_AG_MAX_LIMIT  : 
		mvret = xiSetParamFloat( hmv, XI_PRM_AG_MAX_LIMIT, fval );
		HandleXiResult( mvret );
		break;
    case CV_CAP_PROP_XI_AEAG_LEVEL    : 
		mvret = xiSetParamInt( hmv, XI_PRM_AEAG_LEVEL, ival ); 
		HandleXiResult( mvret );
		break;
    case CV_CAP_PROP_XI_TIMEOUT       : timeout = ival; break;
    }

error:
    if( mvret != XI_OK )
    {
        errMsg( "Set parameter error", mvret );
        return false;
    }
    else
        return true;

}

void CaptureXimea::errMsg(const char* msg, int errNum)
{
#if defined WIN32 || defined _WIN32
    char buf[512]="";
    sprintf( buf, "%s : %d\n", msg, errNum);
    OutputDebugString(buf);
#else
    fprintf( stderr, "%s : %d\n", msg, errNum );
#endif
}

int  CaptureXimea::getBpp()
{
    switch( m_xiimage.frm )
    {
    case XI_MONO8       :
    case XI_RAW8        : 
		return 1;
    case XI_MONO16      :
    case XI_RAW16       : 
		return 2;
    case XI_RGB24       :
    case XI_RGB_PLANAR  : 
		return 3;
    case XI_RGB32       : 
		return 4;
    default :
        return 0;
    }
}
