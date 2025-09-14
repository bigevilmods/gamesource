// (c) 1999-2007 Electronic Arts Inc.
/****************************************************************************************************************\

	Wavelet.cpp: Implementation of the CWaveletImp class.

	July '99 - by Mark Feldman


	Images are converted from RGB to YPbPr using the following matrix:

	|Y | = | 0.299      0.587      0.114   | |R|
	|Pb| = |-0.168736  -0.331264   0.5     | |G|
	|Pr| = | 0.5       -0.418688  -0.081312| |B|

  Images are converted from YPbPr to RGB using the following matrix:

	|R| = | 1.0   0.         1.402    | |Y |
	|G| = | 1.0  -0.344136  -0.714136 | |Pb|
	|B| = | 1.0   1.772      0.0      | |Pr|

\****************************************************************************************************************/

// needs bull lib 81 in dir utils\bull_lib

#include "Wavelet.h"
#include <math.h>

#include "LbZLib.h"

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include "IgArray.h"
typedef CIgArray<float,float>CFloatArray;
typedef CIgArray<unsigned char,unsigned char>CByteArray;

//////////////////////////////////////////////////////////////////////
// Defines
//////////////////////////////////////////////////////////////////////

const float C0 = (float)((1 + sqrt(3)) / 4 / sqrt(2));
const float C1 = (float)((3 + sqrt(3)) / 4 / sqrt(2));
const float C2 = (float)((3 - sqrt(3)) / 4 / sqrt(2));
const float C3 = (float)((1 - sqrt(3)) / 4 / sqrt(2));

// The following macros convert from RGB to YPbPr
#define RGB2Y(R,G,B) ((R)*0.299f + (G)*0.587f + (B)*0.114f)
#define RGB2Pb(R,G,B) (-(R)*0.168736f - (G)*0.331264f + (B)*0.5f)
#define RGB2Pr(R,G,B) ((R)*0.5f - (G)*0.418688f - (B)*0.081312f)

// The following macros convert from YPbPr to RGB
#define YPbPr2R(Y,Pb,Pr) ((Y) + (Pr)*1.402f)
#define YPbPr2G(Y,Pb,Pr) ((Y) - (Pb)*0.344136f - (Pr)*0.714136f)
#define YPbPr2B(Y,Pb,Pr) ((Y) + (Pb)*1.772f)

#define FloatToByte(d) (((d) < 0) ? 0 : ((d) > 255) ? 255 : (unsigned char)(d))

#define DROP_RES

//#define RGB_SPACE

//////////////////////////////////////////////////////////////////////
// Wavelet class
//////////////////////////////////////////////////////////////////////

class CWaveletImp : virtual public CWavelet
{
public:

	CWaveletImp();

	// Compression/decompression functions
	virtual void Compress(const unsigned int dwWidth, const unsigned int dwHeight, const bool bColor, const unsigned char * pImage, unsigned char * pCompressedImage, unsigned int &dwSize, const float fYQuant=1.0f, const float fPbQuant=1.0f, const float fPrQuant=1.0f);
	virtual void Decompress(const unsigned int dwWidth, const unsigned int dwHeight, const bool bColor, const unsigned char * pCompressedImage, unsigned char * pImage, const float fYQuant=1.0f, const float fPbQuant=1.0f, const float fPrQuant=1.0f);

	// Reference counting
	virtual int AddRef();
	virtual int GetRefCount();
	virtual int Release();

protected:

	virtual ~CWaveletImp();
	
	inline void CompressColor(const unsigned int dwWidth, const unsigned int dwHeight, const unsigned char * pImage, unsigned char * pCompressedImage, unsigned int &dwSize, const float fYQuant, const float fPbQuant, const float fPrQuant);
	inline void CompressMono(const unsigned int dwWidth, const unsigned int dwHeight, const unsigned char * pImage, unsigned char * pCompressedImage, unsigned int &dwSize, const float fYQuant);
	inline void DecompressColor(const unsigned int dwWidth, const unsigned int dwHeight, const unsigned char * pCompressedImage, unsigned char * pImage, const float fYQuant, const float fPbQuant, const float fPrQuant);
	inline void DecompressColorFast(const unsigned int dwWidth, const unsigned int dwHeight, const unsigned char * pCompressedImage, unsigned char * pImage, const float fYQuant, const float fPbQuant, const float fPrQuant);
	inline void DecompressMono(const unsigned int dwWidth, const unsigned int dwHeight, const unsigned char * pCompressedImage, unsigned char * pImage, const float fYQuant);
	inline void DecompressMonoFast(const unsigned int dwWidth, const unsigned int dwHeight, const unsigned char * pCompressedImage, unsigned char * pImage, const float fYQuant);
	inline void ExpandChannels(const unsigned int dwWidth, const unsigned dwHeight, const unsigned dwPitch, const unsigned char * pImage);
	inline void BlendChannels(const unsigned int dwWidth, const unsigned dwHeight, const unsigned dwPitch, unsigned char * pImage);
	inline void ConvertAndBlend(const unsigned int dwWidth, const unsigned int dwHeight, const unsigned int dwPitch, unsigned char * pImage);
	inline void ForwardChannel(CFloatArray &image, const unsigned int dwPitch, const unsigned int dwWidth);
	inline void ReverseChannel(CFloatArray &image, const unsigned int dwPitch, const unsigned int dwWidth);
	inline void ReverseChannelFast(CFloatArray &image, const unsigned int dwPitch, const unsigned int dwWidth);
	inline void ForwardTransform(const float * pSource, float * pDest, const int nLength);
	inline void ReverseTransformHorz(const float * pSource, float * pDest, const int nLength);
	inline void ReverseTransformVert(const float * pSource, float * pDest, const int nLength, const int nPitch);
	inline void RGBToYPbPr(const unsigned int dwNumPixels);
	inline void YPbPrToRGB(const unsigned int dwNumPixels);
	inline void DropRes(CFloatArray &image, const unsigned int dwWidth);
	inline void UpRes(CFloatArray &image, const unsigned int dwWidth);
	inline unsigned int PowerUp(const unsigned int dwSize);
	inline void PackShort(const short nVal, unsigned char * &pData);
	inline float UnpackShort(const unsigned char * &pData);

private:
	int m_nRefCount;
	CFloatArray m_Reds, m_Greens, m_Blues;
	CByteArray m_PackedImage;
};

unsigned char ClipTable[512];


////////////////////////////////////////////////////////////////////////////////////////////////////////
// Construction/Destruction
////////////////////////////////////////////////////////////////////////////////////////////////////////

CWaveletImp::CWaveletImp() : m_nRefCount(1)
{
	for (int i=0; i<512; i++)
	{
		int j = i - 128;
		ClipTable[i] = j < 0 ? 0 : j > 255 ? 255 : j;
	}
}

CWaveletImp::~CWaveletImp()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: PowerUp
// DESCRIPTION: Calculates next highest power of 2
unsigned int CWaveletImp::PowerUp(unsigned int dwSize)
{
	unsigned int dwResult = 1;
	dwSize--;
	dwSize >>= 1;
	while (dwSize)
	{
		dwResult++;
		dwSize >>= 1;
	}
	if (dwResult < 3) dwResult = 3;
	return 1 << dwResult;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: PackShort
// DESCRIPTION: A very simple macro to pack short integers into an array of bytes. The values that
//              the wavelet transform spits out are mostly in the signed char range, so most values
//              are stored as unsigned chars. Values falling outside this range are preceded by -128.
void CWaveletImp::PackShort(short nVal, unsigned char * &pData)
{
	if ((nVal >= -127) && (nVal <= 127))
		*pData++ = (unsigned char)nVal;
	else
	{
		*pData++ = -128;
		*(short *)pData = nVal;
		pData += sizeof(short);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: UnpackShort
// DESCRIPTION: Unpacks shorts stored by the PackShort function.
float CWaveletImp::UnpackShort(const unsigned char * &pData)
{
	short nVal = (char)(*pData++);
	if (nVal == -128)
	{
		nVal = *(short *)pData;
		pData += sizeof(short);
	}
	return nVal;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: AddRef
// DESCRIPTION: Increments then returns the reference count.
int CWaveletImp::AddRef()
{
	return ++m_nRefCount;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: GetRefCount
// DESCRIPTION: Returns the object's current reference count
int CWaveletImp::GetRefCount()
{
	return m_nRefCount;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: Release
// DESCRIPTION: Releases and returns the objects reference count. If the reference count falls below 1
//              then the object deletes itself.
int CWaveletImp::Release()
{
	int nResult = --m_nRefCount;
	if (nResult <= 0)
		delete this;
	return nResult;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: Compress
// DESCRIPTION: Public compression function. Dispatches control to the color and mono versions.
void CWaveletImp::Compress(const unsigned int dwWidth, const unsigned int dwHeight, const bool bColor, const unsigned char * pImage, unsigned char * pCompressedImage, unsigned int &dwSize, const float fYQuant, const float fPbQuant, const float fPrQuant)
{
	if (bColor)
		CompressColor(dwWidth, dwHeight, pImage, pCompressedImage, dwSize, fYQuant, fPbQuant, fPrQuant);
	else
		CompressMono(dwWidth, dwHeight, pImage, pCompressedImage, dwSize, fYQuant);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: CompressColor
// DESCRIPTION: Compresses 24-bit BGR images.
void CWaveletImp::CompressColor(const unsigned int dwWidth, const unsigned int dwHeight, const unsigned char * pImage, unsigned char * pCompressedImage, unsigned int &dwSize, const float fYQuant, const float fPbQuant, const float fPrQuant)
{
	unsigned int i;

	unsigned int dwPowerUpWidth = PowerUp(dwWidth);
	unsigned int dwPowerUpHeight = PowerUp(dwHeight);
	unsigned int dwPitch = dwPowerUpWidth > dwPowerUpHeight ? dwPowerUpWidth : dwPowerUpHeight;

	// Do some preliminary calculations
	unsigned int dwNumPixels = dwPitch*dwPitch;

#ifdef DROP_RES
	unsigned int dwNumPixelsLow = dwNumPixels >> 2;
#else
	unsigned int dwNumPixelsLow = dwNumPixels;
#endif

	// Set up buffers for the float images
	m_Reds.SetSize(dwNumPixels+1);	// Need to allocate one more for asm routine later on so we don't corrupt the heap
	m_Greens.SetSize(dwNumPixels+1);
	m_Blues.SetSize(dwNumPixels+1);
	ZeroMemory(m_Reds.GetData(), m_Reds.GetSize()*sizeof(float));
	ZeroMemory(m_Greens.GetData(), m_Greens.GetSize()*sizeof(float));
	ZeroMemory(m_Blues.GetData(), m_Blues.GetSize()*sizeof(float));

	// Separate the channels and convert them to floats
	ExpandChannels(dwWidth, dwHeight, dwPitch, pImage);


	// Convert to YPbPr space
#ifndef RGB_SPACE
	RGBToYPbPr(dwNumPixels);
#endif

	// Lower the resolution of the Pb and Pr channels
#ifdef DROP_RES
	DropRes(m_Greens, dwPitch);
	DropRes(m_Reds, dwPitch);
	ForwardChannel(m_Blues,  dwPitch, dwPitch);
	ForwardChannel(m_Greens, dwPitch>>1, dwPitch>>1);
	ForwardChannel(m_Reds,   dwPitch>>1, dwPitch>>1);
#else // #ifdef DROP_RES
	ForwardChannel(m_Blues,  dwPitch, dwPitch);
	ForwardChannel(m_Greens, dwPitch, dwPitch);
	ForwardChannel(m_Reds,   dwPitch, dwPitch);
#endif // #ifdef DROP_RES

	// Quantize each channel
	float fYQuantFactor = 1.0f / fYQuant;
	float fPbQuantFactor = 1.0f / fPbQuant;
	float fPrQuantFactor = 1.0f / fPrQuant;
	for (i=0; i<dwNumPixels; i++)
		m_Blues[i] *= fYQuantFactor;
	for (i=0; i<dwNumPixelsLow; i++)
	{
		m_Greens[i] *= fPbQuantFactor;
		m_Reds[i] *= fPrQuantFactor;
	}

	// Convert all values to shorts and pack them into a single buffer
	m_PackedImage.SetSize(dwNumPixels*3*sizeof(short));
	unsigned char * pDest = m_PackedImage.GetData();
	for (i=0; i<dwNumPixels; i++)
		PackShort((short)m_Blues[i], pDest);
	for (i=0; i<dwNumPixelsLow; i++)
		PackShort((short)m_Greens[i], pDest);
	for (i=0; i<dwNumPixelsLow; i++)
		PackShort((short)m_Reds[i], pDest);
	unsigned int size = pDest - m_PackedImage.GetData();

	// LZW compress the data
	dwSize = LbZLib_MemCompress(m_PackedImage.GetData(), pCompressedImage, size, NULL);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: CompressMono
// DESCRIPTION: Compresses 8-bit greyscale images.
void CWaveletImp::CompressMono(const unsigned int dwWidth, const unsigned int dwHeight, const unsigned char * pImage, unsigned char * pCompressedImage, unsigned int &dwSize, const float fYQuant)
{
	unsigned int i;

	unsigned int dwPowerUpWidth = PowerUp(dwWidth);
	unsigned int dwPowerUpHeight = PowerUp(dwHeight);
	unsigned int dwPitch = dwPowerUpWidth > dwPowerUpHeight ? dwPowerUpWidth : dwPowerUpHeight;

	// Do some preliminary calculations
	unsigned int dwNumPixels = dwPitch*dwPitch;

	// Set up a buffer for the floats image (we'll use the blue buffers for mono stuff).
	m_Blues.SetSize(dwNumPixels);
	ZeroMemory(m_Blues.GetData(), m_Blues.GetSize()*sizeof(float));

	// Convert channel to floats
	float * pBlues = m_Blues.GetData();
	unsigned int x,y;
	for (i=0,y=0; y<dwHeight; y++, pBlues+=dwPitch)
		for (x=0; x<dwWidth; x++)
			pBlues[x] = pImage[i++];

	// Compress it
	ForwardChannel(m_Blues,  dwPitch, dwPitch);

	// Quantize the values
	float fYQuantFactor = 1.0f / fYQuant;
	for (i=0; i<dwNumPixels; i++)
		m_Blues[i] *= fYQuantFactor;

	// Convert them to shorts and pack them
	m_PackedImage.SetSize(dwNumPixels*sizeof(short));
	unsigned char * pDest = m_PackedImage.GetData();
	for (i=0; i<dwNumPixels; i++)
		PackShort((short)m_Blues[i], pDest);
	unsigned int size = pDest - m_PackedImage.GetData();

	// LZW compress the data
	dwSize = LbZLib_MemCompress(m_PackedImage.GetData(), pCompressedImage, size, NULL);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: Decompress
// DESCRIPTION: Public compression function. Dispatches control to the color and mono versions.
void CWaveletImp::Decompress(const unsigned int dwWidth, const unsigned int dwHeight, const bool bColor, const unsigned char * pCompressedImage, unsigned char * pImage, const float fYQuant, const float fPbQuant, const float fPrQuant)
{
	if (bColor)
		DecompressColorFast(dwWidth, dwHeight, pCompressedImage, pImage, fYQuant, fPbQuant, fPrQuant);
	else
		DecompressMonoFast(dwWidth, dwHeight, pCompressedImage, pImage, fYQuant);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: DecompressColor
// DESCRIPTION: Decompresses 24-bit BGR images.
void CWaveletImp::DecompressColor(const unsigned int dwWidth, const unsigned int dwHeight, const unsigned char * pCompressedImage, unsigned char * pImage, const float fYQuant, const float fPbQuant, const float fPrQuant)
{
	unsigned int dwPowerUpWidth = PowerUp(dwWidth);
	unsigned int dwPowerUpHeight = PowerUp(dwHeight);
	unsigned int dwPitch = dwPowerUpWidth > dwPowerUpHeight ? dwPowerUpWidth : dwPowerUpHeight;

	// Do some preliminary calculations
	unsigned int dwNumPixels = dwPitch*dwPitch;
#ifdef DROP_RES
	unsigned int dwNumPixelsLow = dwNumPixels >> 2;
#else
	unsigned int dwNumPixelsLow = dwNumPixels;
#endif

	// Decompress the data
	m_PackedImage.SetSize(dwNumPixels * sizeof(short));
	unsigned int test = LbZLib_MemDecompress(const_cast<unsigned char *>(pCompressedImage), m_PackedImage.GetData(), 0);

	// Create the buffers for the floats images
	m_Blues.SetSize(dwNumPixels+1);	// Need to allocate one more for asm routine later on so we don't corrupt the heap
	m_Greens.SetSize(dwNumPixels+1);
	m_Reds.SetSize(dwNumPixels+1);

	// Unpack the channels
	const unsigned char * pDest = m_PackedImage.GetData();
	for (unsigned int i=0; i<dwNumPixels; i++)
		m_Blues[i] = UnpackShort(pDest) * fYQuant;
	for (i=0; i<dwNumPixelsLow; i++)
		m_Greens[i] = UnpackShort(pDest) * fPbQuant;
	for (i=0; i<dwNumPixelsLow; i++)
		m_Reds[i] = UnpackShort(pDest) * fPrQuant;

	// Transform each channel
#ifdef DROP_RES
	ReverseChannel(m_Greens, dwPitch>>1, dwPitch>>1);
	ReverseChannel(m_Reds,   dwPitch>>1, dwPitch>>1);
	ReverseChannel(m_Blues,  dwPitch, dwPitch);
	UpRes(m_Greens, dwPitch);
	UpRes(m_Reds, dwPitch);
#else // #ifdef DROP_RES
	ReverseChannel(m_Blues,  dwPitch, dwPitch);
	ReverseChannel(m_Greens, dwPitch, dwPitch);
	ReverseChannel(m_Reds,   dwPitch, dwPitch);
#endif // #ifdef DROP_RES

#ifndef RGB_SPACE
	YPbPrToRGB(dwNumPixels);
#endif
		
	// Convert channels to RGB and merge them into a single image
	BlendChannels(dwWidth, dwHeight, dwPitch, pImage);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: DecompressMono
// DESCRIPTION: Decompresses 8-bit greyscale images.
void CWaveletImp::DecompressMono(const unsigned int dwWidth, const unsigned int dwHeight, const unsigned char * pCompressedImage, unsigned char * pImage, const float fYQuant)
{
	unsigned int dwPowerUpWidth = PowerUp(dwWidth);
	unsigned int dwPowerUpHeight = PowerUp(dwHeight);
	unsigned int dwPitch = dwPowerUpWidth > dwPowerUpHeight ? dwPowerUpWidth : dwPowerUpHeight;

	// Do some preliminary calculations
	unsigned int dwNumPixels = dwPitch*dwPitch;

	// Decompress the data
	m_PackedImage.SetSize(dwNumPixels * sizeof(short));
	unsigned int test = LbZLib_MemDecompress(const_cast<unsigned char *>(pCompressedImage), m_PackedImage.GetData(), NULL);
	
	// Create the buffers for the floats images
	m_Blues.SetSize(dwNumPixels);

	// Unpack the channels
	unsigned char * pDest = m_PackedImage.GetData();
	for (unsigned int i=0; i<dwNumPixels; i++)
		m_Blues[i] = UnpackShort(pDest);

	// Dequantize each channel
	for (i=0; i<dwNumPixels; i++)
		m_Blues[i] *= fYQuant;

	// Untransform
	ReverseChannel(m_Blues,  dwPitch, dwPitch);

	// Store
	float * pBlues = m_Blues.GetData();
	unsigned int x,y;
	for (i=0,y=0; y<dwHeight; y++, pBlues+=dwPitch)
		for (x=0; x<dwWidth; x++)
			pImage[i++] = FloatToByte(pBlues[x]);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: RGBToYPbPr
// DESCRIPTION: Converts a buffer from RGB space to YPbPr space
void CWaveletImp::RGBToYPbPr(const unsigned int dwNumPixels)
{
	for (unsigned int i=0; i<dwNumPixels; i++)
	{
		// Get each component
		float b = m_Blues[i];
		float g = m_Greens[i];
		float r = m_Reds[i];

		// Now convert and store it
		m_Blues[i]  = RGB2Y(r,g,b);
		m_Greens[i] = RGB2Pb(r,g,b);
		m_Reds[i] = RGB2Pr(r,g,b);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: RGBToYPbPr
// DESCRIPTION: Converts a buffer from YPbPr space to RGB space

float CONVERT_VAL_1 = 1.402f;
float CONVERT_VAL_2 = -0.344136f;
float CONVERT_VAL_3 = -0.714136f;
float CONVERT_VAL_4 = 1.772f;

void CWaveletImp::YPbPrToRGB(const unsigned int dwNumPixels)
{
	float * pBlues = m_Blues.GetData();
	float * pGreens = m_Greens.GetData();
	float * pReds = m_Reds.GetData();
	float dummy;

	_asm mov esi,pBlues
	_asm mov edi,pGreens
	_asm mov ebx,pReds
	_asm sub esi,4
	_asm sub edi,4
	_asm sub ebx,4
	_asm mov ecx,dwNumPixels
	_asm fld dword ptr CONVERT_VAL_4
	_asm fld dword ptr CONVERT_VAL_3
	_asm fld dword ptr CONVERT_VAL_2
	_asm fld dword ptr CONVERT_VAL_1
YPbPrToRGB_loop:
	
	_asm fld dword ptr [esi+ecx*4]				// ST0=Y
	_asm fld dword ptr [edi+ecx*4]				// ST0=Pb, ST1=Y
	_asm fld dword ptr [ebx+ecx*4]				// ST0=Pr, ST1=Pb, ST2=Y

	_asm fld ST(0)								// ST0=Pr, ST1=Pr, ST2=Pb, ST3=Y
	_asm fmul ST(0),ST(4)						// ST0=Pr*CONVERT_VAL_1, ST1=Pr, ST2=Pb, ST3=Y
	_asm fadd ST(0),ST(3)						// ST0=Pr*CONVERT_VAL_1+Y, ST1=Pr, ST2=Pb, ST3=Y
	_asm fstp dword ptr [ebx+ecx*4]				// ST0=Pr, ST1=Pb, ST2=Y
	
	_asm fld ST(1)								// ST0=Pb, ST1=Pr, ST2=Pb, ST3=Y
	_asm fmul ST(0),ST(7)						// ST0=Pb*CONVERT_VAL_4, ST1=Pr, ST2=Pb, ST3=Y
	_asm fadd ST(0),ST(3)						// ST0=Pb*CONVERT_VAL_4+Y, ST1=Pr, ST2=Pb, ST3=Y
	_asm fstp dword ptr [esi+ecx*4]				// ST0=Pr, ST1=Pb, ST2=Y

	_asm fmul ST(0),ST(5)						// ST0=Pr*CONVERT_VAL_3, ST1=Pb, ST2=Y
	_asm fxch ST(1)								// ST0=Pb, ST1=Pr*CONVERT_VAL_2, ST2=Y
	_asm fmul ST(0),ST(4)						// ST0=Pb*CONVERT_VAL_2, ST1=Pr*CONVERT_VAL_2, ST2=Y
	_asm fadd ST(0),ST(1)
	_asm fadd ST(0),ST(2)
	_asm fstp dword ptr [edi+ecx*4]
	_asm fstp dummy
	_asm fstp dummy

	_asm dec ecx
	_asm jnz YPbPrToRGB_loop
	_asm fstp dword ptr dummy
	_asm fstp dword ptr dummy
	_asm fstp dword ptr dummy
	_asm fstp dword ptr dummy
}

/*
void CWaveletImp::YPbPrToRGB(const unsigned int dwNumPixels)
{
	for (unsigned int i=0; i<dwNumPixels; i++)
	{
		// Now store it
		float y  = m_Blues[i];
		float pb = m_Greens[i];
		float pr = m_Reds[i];

		// Convert to RGB
		m_Blues[i]  = YPbPr2B(y,pb,pr);
		m_Greens[i] = YPbPr2G(y,pb,pr);
		m_Reds[i]   = YPbPr2R(y,pb,pr);
	}
}
*/

////////////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: ForwardTransform
// DESCRIPTION: Runs a 1D array of data through a forward 4th-order wavelet transform
void CWaveletImp::ForwardTransform(const float * pSource, float * pDest, int nLength)
{
	/* Order 2 Haar Wavelet
	int nMiddle = nLength >> 1;
	int nMask = nLength - 1;
	for (int i=0, j=0; j<nLength; i++, j+=2)
	{
		if (i > nLength) _asm int 3
		if ((i+nMiddle) > nLength) _asm int 3
		pDest[i] = (pSource[j] + pSource[j+1]) * 0.5f;
		pDest[i+nMiddle] = (pSource[j+1] - pSource[j]);
	}
	 */

	/* Order 4 transform */
	int nMiddle = nLength >> 1;
	int nMask = nLength - 1;
	for (int i=0, j=0; j<nLength; i++, j+=2)
	{
		pDest[i] =	
			C0*pSource[j] +
			C1*pSource[j+1] +
			C2*pSource[(j+2)&nMask] +
			C3*pSource[(j+3)&nMask];
		pDest[i+nMiddle] =
			C3*pSource[j] -
			C2*pSource[j+1] +
			C1*pSource[(j+2)&nMask] -
			C0*pSource[(j+3)&nMask];
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: ReverseTransformHorz
// DESCRIPTION: Runs a row of data through a reverse 4th-order wavelet transform
void CWaveletImp::ReverseTransformHorz(const float * pSource, float * pDest, int nLength)
{
	/* Order 2 Haar wavelet
	int nMask = nLength - 1;
	int nMiddle = nLength >> 1;
	int nHalfMask = nMiddle-1;
	for (int i=0, j=0; i<nMiddle; i++, j+=2)
	{
		if (j > nLength) _asm int 3
		if ((j+1) > nLength) _asm int 3

		pDest[j] = pSource[i] - 0.5f * pSource[i+nMiddle];
		pDest[j+1] = pSource[i] + 0.5f * pSource[i+nMiddle];
	}
	*/

	int nMask = nLength - 1;
	int nMiddle = nLength >> 1;
	int nHalfMask = nMiddle-1;
	for (int i=0, j=2; i<nHalfMask; i++, j+=2)
	{
		pDest[j] =
			C2*pSource[i] +
			C0*pSource[i+1] +
			C1*pSource[i+nMiddle] +
			C3*pSource[i+nMiddle+1];
		pDest[j+1] =
			C3*pSource[i] +
			C1*pSource[i+1] -
			C0*pSource[i+nMiddle] -
			C2*pSource[i+nMiddle+1];
	}

	// Last two wrap around
	pDest[0] =
		C2*pSource[i] +
		C0*pSource[0] +
		C3*pSource[nMiddle] +
		C1*pSource[nMiddle+i];
	pDest[1] =
		C3*pSource[i] +
		C1*pSource[0] -
		C0*pSource[nMiddle+i] -
		C2*pSource[nMiddle];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: ReverseTransformVert
// DESCRIPTION: Runs a collumn of data through a reverse 4th-order wavelet transform
void CWaveletImp::ReverseTransformVert(const float * pSource, float * pDest, int nLength, const int nPitch)
{
	/* Order 2 Haar Wavelet
	int nMask = nLength - 1;
	int nHalfMask = (nLength >> 1) - 1;
	int nMiddle = (nLength >> 1) * nPitch;
	int nLastVal = nHalfMask * nPitch;
	for (int i=0, j=0; i<nLastVal; i+=nPitch, j+=2)
	{
		if (j > nLength) _asm int 3
		if ((j+1) > nLength) _asm int 3

		pDest[j] = pSource[i] - 0.5f * pSource[i+nMiddle];
		pDest[j+1] = pSource[i] + 0.5f * pSource[i+nMiddle];
	}
	*/

	int nMask = nLength - 1;
	int nHalfMask = (nLength >> 1) - 1;
	int nMiddle = (nLength >> 1) * nPitch;
	int nLastVal = nHalfMask * nPitch;
	for (int i=0, j=2; i<nLastVal; i+=nPitch, j+=2)
	{
		pDest[j] =
			C2*pSource[i] +
			C0*pSource[i+nPitch] +
			C1*pSource[i+nMiddle] +
			C3*pSource[i+nPitch+nMiddle];
		pDest[j+1] =
			C3*pSource[i] +
			C1*pSource[i+nPitch] -
			C0*pSource[i+nMiddle] -
			C2*pSource[i+nPitch+nMiddle];
	}

	// Last two wrap around
	pDest[0] =
		C2*pSource[i] +
		C0*pSource[0] +
		C3*pSource[nMiddle] +
		C1*pSource[i+nMiddle];
	pDest[1] =
		C3*pSource[i] +
		C1*pSource[0] -
		C0*pSource[i+nMiddle] -
		C2*pSource[nMiddle];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: ForwardChannel
// DESCRIPTION: Performs a 2D forward wavelet transform. This is done by transforming first along the
//              horizontal axis followed by the vertical axis.
void CWaveletImp::ForwardChannel(CFloatArray &image, const unsigned int dwPitch, const unsigned int dwWidth)
{
	unsigned int x,y;

	// Set up buffers
	CFloatArray source1, source2, dest1, dest2;
	source1.SetSize(dwWidth);
	dest1.SetSize(dwWidth);
	source2.SetSize(dwWidth);
	dest2.SetSize(dwWidth);

	// Loop across each collumn
	unsigned int dwHalfWidth = dwWidth >> 1;
	unsigned int dwHalfHeight = dwWidth >> 1;
	for (x=0; x<dwWidth; x++)
	{
		// Get this collumn
		float * pThisPixel = image.GetData() + x;
		for (y=0; y<dwWidth; y++, pThisPixel += dwPitch)
			source2[y] = *pThisPixel;

		// Forward transform
		ForwardTransform(source2.GetData(), dest2.GetData(), dwWidth);

		// Copy it back to my image
		pThisPixel = image.GetData() + x;
		for (y=0; y<dwWidth; y++, pThisPixel += dwPitch)
			*pThisPixel = dest2[y];
	}

	// Loop down each row
	float * pThisRow = image.GetData();
	for (y=0; y<dwWidth; y++, pThisRow+=dwPitch)
	{
		// Get this row
		for (x=0; x<dwWidth; x++)
			source1[x] = pThisRow[x];

		// Forward transform
		ForwardTransform(source1.GetData(), dest1.GetData(), dwWidth);

		// Copy it back to my image
		for (x=0; x<dwWidth; x++)
			pThisRow[x] = dest1[x];
	}

}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: ReverseChannel
// DESCRIPTION: Performs a 2D reverse wavelet transform. This is done by reverse transforming first
//              along the vertical axis followed by the horizontal axis.
void CWaveletImp::ReverseChannel(CFloatArray &image, const unsigned int dwPitch, const unsigned int dwWidth)
{
	unsigned int x,y;

	// Loop down each row
	CFloatArray horzbuff;
	horzbuff.SetSize(dwWidth);
	float * pThisRow = image.GetData();
	for (y=0; y<dwWidth; y++, pThisRow+=dwPitch)
	{
		// Forward transform into the temporary buffer
		ReverseTransformHorz(pThisRow, horzbuff.GetData(), dwWidth);

		// Copy result back into the image
		memcpy(pThisRow, horzbuff.GetData(), dwWidth*sizeof(float));
	}

	/* Older and slightly slower code
	// Loop across each collumn
	CFloatArray vertbuff;
	vertbuff.SetSize(dwWidth);
	unsigned int dwHalfWidth = dwWidth >> 1;
	unsigned int dwHalfHeight = dwWidth >> 1;
	for (x=0; x<dwWidth; x++)
	{
		// Forward transform into a temporary buffer
		ReverseTransformVert(image.GetData()+x, vertbuff.GetData(), dwWidth, dwPitch);

		// Copy result back into the image
		float * pThisPixel = image.GetData() + x;
		float * pTemp = vertbuff.GetData();
		for (y=0; y<dwWidth; y++, pThisPixel += dwPitch)
			*pThisPixel = pTemp[y];
	}
	*/

	// Loop across each collumn
	CFloatArray vertbuff;
	vertbuff.SetSize(dwWidth*8);
	unsigned int dwHalfWidth = dwWidth >> 1;
	unsigned int dwHalfHeight = dwWidth >> 1;
	int ofs0 = 0;
	int ofs1 = dwWidth;
	int ofs2 = ofs1 + dwWidth;
	int ofs3 = ofs2 + dwWidth;
	int ofs4 = ofs3 + dwWidth;
	int ofs5 = ofs4 + dwWidth;
	int ofs6 = ofs5 + dwWidth;
	int ofs7 = ofs6 + dwWidth;
	for (x=0; x<dwWidth; x+=8)
	{
		// Forward transform into a temporary buffer
		for (int i=0; i<8; i++)
			ReverseTransformVert(image.GetData()+x+i, vertbuff.GetData()+i*dwWidth, dwWidth, dwPitch);

		// Copy result back into the image
		float * pThisPixel = image.GetData() + x;
		float * pSource = vertbuff.GetData();
		for (y=0; y<dwWidth; y++, pThisPixel += dwPitch)
		{
			pThisPixel[0] = pSource[ofs0+y];
			pThisPixel[1] = pSource[ofs1+y];
			pThisPixel[2] = pSource[ofs2+y];
			pThisPixel[3] = pSource[ofs3+y];
			pThisPixel[4] = pSource[ofs4+y];
			pThisPixel[5] = pSource[ofs5+y];
			pThisPixel[6] = pSource[ofs6+y];
			pThisPixel[7] = pSource[ofs7+y];
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: ExpandChannels
// DESCRIPTION: Seperates the channels of a BGR image
void CWaveletImp::ExpandChannels(const unsigned int dwWidth, const unsigned dwHeight, const unsigned dwPitch, const unsigned char * pImage)
{
	for (unsigned int y=0; y<dwHeight; y++)
		for (unsigned int x=0; x<dwWidth; x++)
		{
			m_Blues[y*dwPitch+x]    = *pImage++;
			m_Greens[y*dwPitch+x] = *pImage++;
			m_Reds[y*dwPitch+x]   = *pImage++;
		}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: BlendChannels
// DESCRIPTION: Blends BGR channels into a single 24-bit image

void CWaveletImp::BlendChannels(const unsigned int dwWidth, const unsigned dwHeight, const unsigned dwPitch, unsigned char * pImage)
{
	float * pBlues = m_Blues.GetData();
	float * pGreens = m_Greens.GetData();
	float * pReds = m_Reds.GetData();
	for (unsigned int y=0; y<dwHeight; y++)
	{
		for (unsigned int x=0; x<dwWidth; x++)
		{
			pBlues[x] += 0xC00000;
			*pImage++ = ClipTable[*(short *)(pBlues+x)+128];
			pGreens[x] += 0xC00000;
			*pImage++ = ClipTable[*(short *)(pGreens+x)+128];
			pReds[x] += 0xC00000;
			*pImage++ = ClipTable[*(short *)(pReds+x)+128];
		}
		pBlues += dwPitch;
		pGreens += dwPitch;
		pReds += dwPitch;
	}
}

/* Older slightly slower version
void CWaveletImp::BlendChannels(const unsigned int dwWidth, const unsigned dwHeight, const unsigned dwPitch, unsigned char * pImage)
{
	float * pBlues = m_Blues.GetData();
	float * pGreens = m_Greens.GetData();
	float * pReds = m_Reds.GetData();
	for (unsigned int y=0; y<dwHeight; y++)
	{
		for (unsigned int x=0; x<dwWidth; x++)
		{
			*pImage++ = FloatToByte(pBlues[x]);
			*pImage++ = FloatToByte(pGreens[x]);
			*pImage++ = FloatToByte(pReds[x]);
		}
		pBlues += dwPitch;
		pGreens += dwPitch;
		pReds += dwPitch;
	}
}
*/

/* Older and slower version of BlendChannels *
for (unsigned int i=0; i<dwNumPixels; i++)
{
	*pImage++ = FloatToByte(m_Blues[i]);
	*pImage++ = FloatToByte(m_Greens[i]);
	*pImage++ = FloatToByte(m_Reds[i]);
}
*/

////////////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: DropRes
// DESCRIPTION: Drops an image's resolution to 1/4th it's original size. The human eye is more sensitive
//              to subtle changes in intensity than it is to changes in chroma, so I lower the resolution
//              of the two chroma channels before compressing them.
void CWaveletImp::DropRes(CFloatArray &image, const unsigned int dwWidth)
{
	unsigned int dwNewWidth = dwWidth >> 1;
	unsigned int dwNewHeight = dwWidth>> 1;
	for (unsigned int y=0; y<dwNewHeight; y++)
		for (unsigned int x=0; x<dwNewWidth; x++)
			image[y*dwNewWidth+x] =
			(
			image[(y<<1)*dwWidth+(x<<1)] +
			image[(y<<1)*dwWidth+((x<<1)+1)] +
			image[((y<<1)+1)*dwWidth+(x<<1)] +
			image[((y<<1)+1)*dwWidth+((x<<1)+1)]
			) / 4;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: UpRes
// DESCRIPTION: Increase the resolution of a channel. See DropRes for details.
void CWaveletImp::UpRes(CFloatArray &image, const unsigned int dwWidth)
{
	unsigned int dwNewWidth = dwWidth >> 1;
	float * pSourceLine = image.GetData() + (((dwWidth-1)>>1)*dwNewWidth);
	float * pDestLine = image.GetData() + ((dwWidth - 1) * dwWidth);
	for (int y=static_cast<int>(dwWidth)-1; y>=0; y--)
	{
		for (int x=static_cast<int>(dwWidth)-1; x>=0; x--)
			pDestLine[x] = pSourceLine[x>>1];
		pDestLine-=dwWidth;
		if (!(y&1))
			pSourceLine -= dwNewWidth;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: CreateWavelet
// DESCRIPTION: Create a pointer to a CWavelet implementation object
// RETURNS: A pointer to the new object
CWavelet * CreateWavelet()
{
	return new CWaveletImp;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: DecompressColorFast
// DESCRIPTION: Optimized decompression
void CWaveletImp::DecompressColorFast(const unsigned int dwWidth, const unsigned int dwHeight, const unsigned char * pCompressedImage, unsigned char * pImage, const float fYQuant, const float fPbQuant, const float fPrQuant)
{
	unsigned int dwPowerUpWidth = PowerUp(dwWidth);
	unsigned int dwPowerUpHeight = PowerUp(dwHeight);
	unsigned int dwPitch = dwPowerUpWidth > dwPowerUpHeight ? dwPowerUpWidth : dwPowerUpHeight;

	// Do some preliminary calculations
	unsigned int dwNumPixels = dwPitch*dwPitch;
#ifdef DROP_RES
	unsigned int dwNumPixelsLow = dwNumPixels >> 2;
	unsigned int dwPitchLow = dwPitch >> 1;
#else
	unsigned int dwNumPixelsLow = dwNumPixels;
	unsigned int dwPitchLow = dwPitch;
#endif

	// Decompress the data
	m_PackedImage.SetSize(dwNumPixels * sizeof(short));
	unsigned int test = LbZLib_MemDecompress(const_cast<unsigned char *>(pCompressedImage), m_PackedImage.GetData(), 0);

	// Create the buffers for the floats images
	m_Blues.SetSize(dwNumPixels);
	m_Greens.SetSize(dwNumPixels);
	m_Reds.SetSize(dwNumPixels);
	CFloatArray horzbuff;
	horzbuff.SetSize(dwPitch);
	float * pDest;
	unsigned int x,y;

	// Unpack the channels
	const unsigned char * pPacked = m_PackedImage.GetData();
	
	pDest = m_Blues.GetData();
	for (y=0; y<dwPitch; y++, pDest+=dwPitch)
	{
		for (x=0; x<dwPitch; x++)
			horzbuff[x] = UnpackShort(pPacked) * fYQuant;
		ReverseTransformHorz(horzbuff.GetData(), pDest, dwPitch);
	}
	ReverseChannelFast(m_Blues, dwPitch, dwPitch);
	pDest = m_Greens.GetData();
	for (y=0; y<dwPitchLow; y++, pDest+=dwPitchLow)
	{
		for (x=0; x<dwPitchLow; x++)
			horzbuff[x] = UnpackShort(pPacked) * fPbQuant;
		ReverseTransformHorz(horzbuff.GetData(), pDest, dwPitchLow);
	}
	ReverseChannelFast(m_Greens, dwPitchLow, dwPitchLow);
	pDest = m_Reds.GetData();
	for (y=0; y<dwPitchLow; y++, pDest+=dwPitchLow)
	{
		for (x=0; x<dwPitchLow; x++)
			horzbuff[x] = UnpackShort(pPacked) * fPrQuant;
		ReverseTransformHorz(horzbuff.GetData(), pDest, dwPitchLow);
	}
	ReverseChannelFast(m_Reds, dwPitchLow, dwPitchLow);

	// Transform each channel
#ifdef DROP_RES
	UpRes(m_Greens, dwPitch);
	UpRes(m_Reds, dwPitch);
#endif // #ifdef DROP_RES

#ifndef RGB_SPACE
	ConvertAndBlend(dwWidth, dwHeight, dwPitch, pImage);
#else
	BlendChannels(dwWidth, dwHeight, dwPitch, pImage);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: DecompressMonoFast
// DESCRIPTION: Optimized decompression
void CWaveletImp::DecompressMonoFast(const unsigned int dwWidth, const unsigned int dwHeight, const unsigned char * pCompressedImage, unsigned char * pImage, const float fYQuant)
{
	unsigned int dwPowerUpWidth = PowerUp(dwWidth);
	unsigned int dwPowerUpHeight = PowerUp(dwHeight);
	unsigned int dwPitch = dwPowerUpWidth > dwPowerUpHeight ? dwPowerUpWidth : dwPowerUpHeight;

	// Do some preliminary calculations
	unsigned int dwNumPixels = dwPitch*dwPitch;

	// Decompress the data
	m_PackedImage.SetSize(dwNumPixels * sizeof(short));
	unsigned int test = LbZLib_MemDecompress(const_cast<unsigned char *>(pCompressedImage), m_PackedImage.GetData(), NULL);
	
	// Create the buffers for the floats images
	m_Blues.SetSize(dwNumPixels);
	CFloatArray horzbuff;
	horzbuff.SetSize(dwPitch);
	float * pDest;
	unsigned int i,x,y;

	// Unpack the channels
	const unsigned char * pPacked = m_PackedImage.GetData();
	
	pDest = m_Blues.GetData();
	for (y=0; y<dwPitch; y++, pDest+=dwPitch)
	{
		for (x=0; x<dwPitch; x++)
			horzbuff[x] = UnpackShort(pPacked) * fYQuant;
		ReverseTransformHorz(horzbuff.GetData(), pDest, dwPitch);
	}
	ReverseChannelFast(m_Blues, dwPitch, dwPitch);

	// Store
	float * pBlues = m_Blues.GetData();
	for (i=0,y=0; y<dwHeight; y++, pBlues+=dwPitch)
		for (x=0; x<dwWidth; x++)
			pImage[i++] = FloatToByte(pBlues[x]);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: ReverseChannel
// DESCRIPTION: Performs a 2D reverse wavelet transform. This is done by reverse transforming first
//              along the vertical axis followed by the horizontal axis.
void CWaveletImp::ReverseChannelFast(CFloatArray &image, const unsigned int dwPitch, const unsigned int dwWidth)
{
	unsigned int x,y;

	/* Older and slightly slower code
	// Loop across each collumn
	CFloatArray vertbuff;
	vertbuff.SetSize(dwWidth);
	unsigned int dwHalfWidth = dwWidth >> 1;
	unsigned int dwHalfHeight = dwWidth >> 1;
	for (x=0; x<dwWidth; x++)
	{
		// Forward transform into a temporary buffer
		ReverseTransformVert(image.GetData()+x, vertbuff.GetData(), dwWidth, dwPitch);

		// Copy result back into the image
		float * pThisPixel = image.GetData() + x;
		float * pTemp = vertbuff.GetData();
		for (y=0; y<dwWidth; y++, pThisPixel += dwPitch)
			*pThisPixel = pTemp[y];
	}
	*/

	// Loop across each collumn
	CFloatArray vertbuff;
	vertbuff.SetSize(dwWidth*8);
	unsigned int dwHalfWidth = dwWidth >> 1;
	unsigned int dwHalfHeight = dwWidth >> 1;
	int ofs0 = 0;
	int ofs1 = dwWidth;
	int ofs2 = ofs1 + dwWidth;
	int ofs3 = ofs2 + dwWidth;
	int ofs4 = ofs3 + dwWidth;
	int ofs5 = ofs4 + dwWidth;
	int ofs6 = ofs5 + dwWidth;
	int ofs7 = ofs6 + dwWidth;
	for (x=0; x<dwWidth; x+=8)
	{
		// Forward transform into a temporary buffer
		for (int i=0; i<8; i++)
			ReverseTransformVert(image.GetData()+x+i, vertbuff.GetData()+i*dwWidth, dwWidth, dwPitch);

		// Copy result back into the image
		float * pThisPixel = image.GetData() + x;
		float * pSource = vertbuff.GetData();
		for (y=0; y<dwWidth; y++, pThisPixel += dwPitch)
		{
			pThisPixel[0] = pSource[ofs0+y];
			pThisPixel[1] = pSource[ofs1+y];
			pThisPixel[2] = pSource[ofs2+y];
			pThisPixel[3] = pSource[ofs3+y];
			pThisPixel[4] = pSource[ofs4+y];
			pThisPixel[5] = pSource[ofs5+y];
			pThisPixel[6] = pSource[ofs6+y];
			pThisPixel[7] = pSource[ofs7+y];
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: ConvertAndBlend
// DESCRIPTION: Converts to RGB and blends color channels
void CWaveletImp::ConvertAndBlend(const unsigned int dwWidth, const unsigned int dwHeight, const unsigned int dwPitch, unsigned char * pImage)
{
	float * pBlues = m_Blues.GetData();
	float * pReds = m_Reds.GetData();
	float * pGreens = m_Greens.GetData();
	for (unsigned int i=0, y=0; y<dwHeight; y++,pBlues+=dwPitch, pGreens+=dwPitch, pReds+=dwPitch)
	{
		for (unsigned int x=0; x<dwWidth; x++)
		{
			float fBlue,fGreen,fRed, fY, fPb, fPr;
			
			fY = pBlues[x];
			fPb = pGreens[x];
			fPr = pReds[x];

			// The following macros convert from YPbPr to RGB
			fBlue = fY + fPb*1.772f + 0xC00000;
			fRed  =  fY + fPr*1.402f + 0xC00000;
			fGreen = fY - fPb*0.344136f - fPr*0.714136f + 0xC00000;
		
			*pImage++ = ClipTable[*(short *)(&fBlue)+128];
			*pImage++ = ClipTable[*(short *)(&fGreen)+128];
			*pImage++ = ClipTable[*(short *)(&fRed)+128];
		}
	}
}
