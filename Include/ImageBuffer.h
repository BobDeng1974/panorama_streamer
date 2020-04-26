#pragma once

#include <QImage>

struct AlignedImage
{
	AlignedImage()
	{
		clear();
	}

	~AlignedImage()
	{
	}

	void setImage(QImage img, unsigned char* imgBuffer = 0)
	{
		if (imgBuffer == NULL)
			buffer = img.constBits();
		else
			buffer = imgBuffer;
		width = img.width();
		height = img.height();
		stride = img.bytesPerLine();
	}

	const unsigned char * buffer;
	int width;
	int height;
	int stride;

	void clear()
	{
		buffer = 0;
		width = 0;
		height = 0;
		stride = 0;
	}
	bool isValid() const
	{
		return buffer != NULL ? true : false;
	}
};

struct ImageBufferData
{
	AlignedImage mImageY;
	AlignedImage mImageU;
	AlignedImage mImageV;
	unsigned int mFrame;
	int msToWait;

	enum FrameFormat
	{
		NONE,
		YUV420,
		YUV422,
		RGB888
	} mFormat;

	ImageBufferData(FrameFormat format = NONE)
	{
		mFormat = format;
		mFrame = 0;
		msToWait = 0;
	}
	void clear()
	{
		mImageY.clear();
		mImageU.clear();
		mImageV.clear();
		mFrame = 0;
		msToWait = 0;
		mFormat = NONE;
	}
	bool isValid() const
	{
		if (mFormat != NONE && mImageY.isValid())
			return true;
		return false;
	}
};