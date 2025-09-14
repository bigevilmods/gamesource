// (c) 1999-2007 Electronic Arts Inc.
/****************************************************************************************************************\

	Wavelet.h: Interface for the CWavelet class.

	July '99 - by Mark Feldman


	Call CreateWavelet to create an object derived from CWavelet. The object is created with a reference count
	of 1. AddRef() increments the ref count, Release() decrements it. The object deletes itself when it's
	reference count hits 0.


	virtual void Compress(const unsigned int dwWidth,const unsigned int dwHeight, const bool bColor, const unsigned char * pImage, unsigned char * pCompressedImage, unsigned int &dwSize, const float fYQuant=1.0f, const float fPbQuant=1.0f, const float fPrQuant=1.0f) = 0;
	virtual bool Decompress(const unsigned int dwWidth, const unsigned int dwHeight, const bool bColor, unsigned char * pCompressedImage, const unsigned char * pImage, const float fYQuant=1.0f, const float fPbQuant=1.0f, const float fPrQuant=1.0f) = 0;

	Parameters:

	unsigned int dwWidth
		Image width.

	unsigned int dwHeight
		Image height

	bool bColor
		Image type. true if this image is 24-bit BGR, false if it's 8-bit greyscale or an alpha channel

	unsigned char * pImage
		Pointer to the raw image data. The code has been written to expect this image to be in BGR format.
		You can also pass in RGB format if you want, but you'll probably have to adjust the quantize values
		(see below). Which format compresses better seems to depend largely on the image being compressed.

	unsigned char * pCompressedImage
		Pointer to a buffer for the compresses image data. The size of this buffer must be at least
		as large as the raw image data.

	unsigned int &dwSize
		This variable is set to the size of the final compressed data stored in pCompressedImage

	float fYQuant=1.0f
		Quantize value for the intensity channel. This controls image quality and compression level.
		1 is the lowest level, the image typically starts degrading at about 5.

	float fPbQuant=1.0f, float fPrQuant
		Quantize values for the two chroma channels. These are ignored while compressing or decompressing
		monochome images.
	
	I find that the following values result in an aggressive compression level without too much image loss:
	
		fYQuant    = 6
		fPbQuant   = 10
		fPrQuant   = 2

\****************************************************************************************************************/

#ifndef WAVELET_H
#define WAVELET_H

class CWavelet
{
public:

	CWavelet() {}

	// Compression/decompression functions
	virtual void Compress(const unsigned int dwWidth, const unsigned int dwHeight, const bool bColor, const unsigned char * pImage, unsigned char * pCompressedImage, unsigned int &dwSize, const float fYQuant=1.0f, const float fPbQuant=1.0f, const float fPrQuant=1.0f) = 0;
	virtual void Decompress(const unsigned int dwWidth, const unsigned int dwHeight, const bool bColor, const unsigned char * pCompressedImage, unsigned char * pImage, const float fYQuant=1.0f, const float fPbQuant=1.0f, const float fPrQuant=1.0f) = 0;

	// Reference counting
	virtual int AddRef() = 0;
	virtual int GetRefCount() = 0;
	virtual int Release() = 0;

protected:

	// Private constructor to stop people deleting objects of this type or trying to instance it directly.
	// Advice to anyone thinking about complaining to me about this: go learn C++.
	virtual ~CWavelet() {}		
};

extern CWavelet * CreateWavelet();

#endif // #ifndef WAVELET_H
