/** \file bitmap.h
 * Class managing bitmaps
 *
 * $Id: bitmap.h,v 1.10 2002/01/28 17:28:53 besson Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
 *
 * This file is part of NEVRAX NEL.
 * NEVRAX NEL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NEL is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NEL; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef NL_BITMAP_H
#define NL_BITMAP_H

#include "nel/misc/types_nl.h"
#include "nel/misc/rgba.h"
#include "nel/misc/debug.h"
#include <vector>


namespace NLMISC 
{


class IStream;

//------------------ DDS STUFFS --------------------

#ifndef NL_MAKEFOURCC
    #define NL_MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
                ((uint32)(uint8)(ch0) | ((uint32)(uint8)(ch1) << 8) |   \
                ((uint32)(uint8)(ch2) << 16) | ((uint32)(uint8)(ch3) << 24 ))
#endif 

const uint32	DDS = NL_MAKEFOURCC('D', 'D', 'S', ' ');
const uint32	DXT_ = NL_MAKEFOURCC('D','X', 'T', '\0');


// dwLinearSize is valid
#define DDSD_LINEARSIZE         0x00080000l


//---------------- END OF DDS STUFFS ------------------


const uint8	MAX_MIPMAP = 16;





/**
 * Class Bitmap
 *
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2000
 */
/* *** IMPORTANT ********************
 * *** IF YOU MODIFY THE STRUCTURE OF THIS CLASS, PLEASE INCREMENT IDriver::InterfaceVersion TO INVALIDATE OLD DRIVER DLL
 * **********************************
 */
class CBitmap
{
protected :
	std::vector<uint8> _Data[MAX_MIPMAP];

	// The number of mipmaps. base image IS a mipmap. 1 means a base image with no mipmaping.
	uint8	_MipMapCount;
	bool	_LoadGrayscaleAsAlpha;
	uint32	_Width;
	uint32	_Height;

private :
	

	/** 
	 * blend 2 integers between 0 and 255 .
	 * \param n0 first integer
	 * \param n1 second integer
	 * \param coef coefficient for the first integer (must be in [0,256])
	 */
	uint32 blend(uint32 &n0, uint32 &n1, uint32 coef0);


	/** 
	 * Read a DDS from an IStream. 
	 * The bitmap is readen as a set of bytes and stocked compressed.
	 * Width and Height are multiple of 4.
	 * \param IStream The stream must be in reading mode.
	 * \return image depth
	 * \throw EDDSBadHeader : surface is header is not valid.
	 */
	uint8 readDDS(NLMISC::IStream &f);


	/** 
	 * Read a TGA from an IStream.
	 * TGA pictures can be in 24 or 32 bits, RLE or uncompressed
	 * \param f IStream (must be a reading stream)
	 * \return image depth if succeed, 0 else
	 */
	uint8 readTGA(	NLMISC::IStream &f);



	/** 
	 * Change bitmap format 
	 *
	 * about DXTC1 to DXTC5 :
	 * Does nothing if the format is not DXTC1
	 * about alpha encoding :
	 *		alpha0 == alpha1
	 *		code(x,y) == 7 for every (x,y)
	 *
	 * about luminance to alpha and alpha to luminance :
	 *      the buffer keeps unchanged
	 *
	 */
	///@{
	bool convertToDXTC5();

	bool convertToRGBA();
	bool luminanceToRGBA();
	bool alphaToRGBA();
	bool alphaLuminanceToRGBA();
	
	bool convertToLuminance();
	bool rgbaToLuminance();
	bool alphaToLuminance();
	bool alphaLuminanceToLuminance();
	
	bool convertToAlpha();
	bool rgbaToAlpha();
	bool luminanceToAlpha();
	bool alphaLuminanceToAlpha();

	bool convertToAlphaLuminance();
	bool rgbaToAlphaLuminance();
	bool luminanceToAlphaLuminance();
	bool alphaToAlphaLuminance();
	
	///@}

	/** 
	 * Decompress bitmap compressed with S3TC DXT1 algorithm. 
	 * \param alpha if alpha is true there's alpha.
	 */
	bool decompressDXT1(bool alpha);

	/** 
	 * Decompress bitmap compressed with S3TC DXT3 algorithm. 
	 * \throw EAllocationFailure : can't allocate memory.
	 */
	bool decompressDXT3();


	/** 
	 * Decompress bitmap compressed with S3TC DXT3 algorithm. 
	 * \throw EAllocationFailure : can't allocate memory.
	 */
	bool decompressDXT5();


	/** 
	 * Extracting RGBA infos from a 16bits word. (used by S3TC decompression)
	 * \param color a 16bits integer
	 * \param r a CRGBA
	 */
	void uncompress(uint16 color, NLMISC::CRGBA &);


	/** 
	 * The resample function
	 * \param pSrc CRGBA array
	 * \param pDest CRGBA array for storing resampled texture
	 * \param nSrcWidth original width
	 * \param nSrcHeight original height
	 * \param nDestWidth width after resample
	 * \param nDestHeight height after resample
	 */	
	void resamplePicture32 (const NLMISC::CRGBA *pSrc, NLMISC::CRGBA *pDest, 
							 sint32 nSrcWidth, sint32 nSrcHeight, 
							 sint32 nDestWidth, sint32 nDestHeight);


	/** 
	 * Quadratic interpolator
	 * \return the interpolation in (x,y) of the values (xy**)
	 */
	float getColorInterp (float x, float y, float xy00, float xy01, float xy10, float xy11);

public:

	enum TType 
	{ 
		RGBA=0,
		Luminance,
		Alpha,
		AlphaLuminance,
		DXTC1,
		DXTC1Alpha,
		DXTC3,
		DXTC5,
		DsDt,
		ModeCount,
		DonTKnow=0xffffffff
	} PixelFormat;

	static const uint32 bitPerPixels[ModeCount];
	static const uint32 DXTC1HEADER;
	static const uint32 DXTC3HEADER;
	static const uint32 DXTC5HEADER;

	CBitmap()
	{
		_MipMapCount = 1;
		_Width = 0;
		_Height = 0;
		PixelFormat = RGBA;
		_LoadGrayscaleAsAlpha = true;
	}



	/** 
	 * Read a bitmap(TGA or DDS) from an IStream. 
	 * Bitmap supported are DDS (DXTC1, DXTC1 with Alpha, DXTC3, DXTC5, and
	 * uncompressed TGA (24 and 32 bits).
	 * \param IStream The stream must be in reading mode.
	 * \return image depth (24 or 32), or 0 if load failed
	 * \throw ESeekFailed : seek has failed
	 */
	uint8	load(NLMISC::IStream &f);


	/** 
	 * Determinate the bitmap size from a bitmap(TGA or DDS) from an IStream. load just header of the file.
	 * Bitmap supported are DDS (DXTC1, DXTC1 with Alpha, DXTC3, DXTC5, and
	 * uncompressed TGA (24 and 32 bits).
	 * NB: at the end, f is seeked to begin.
	 * \param IStream The stream must be in reading mode.
	 * \param width the width of the image. 0 if fails.
	 * \param height the height of the image. 0 if fails.
	 * \throw ESeekFailed : seek has failed
	 */
	static void		loadSize(NLMISC::IStream &f, uint32 &width, uint32 &height);


	/** same than other loadSize(), but with a pathName. 
	 * \see loadSize()
	 */
	static void		loadSize(const std::string &path, uint32 &retWidth, uint32 &retHeight);


	/** 
	 * Make a dummy "?" texture. Usefull for file not found. Mode is rgba.
	 */
	void	makeDummy();


	/** 
	 * Return the pixels buffer of the image, or of one of its mipmap.
	 * Return a reference of an array in pixel format get with getPixelFormat().
	 * \return vector<uint8>& RGBA pixels
	 */	
	///@{
	std::vector<uint8>& getPixels(uint32 numMipMap = 0) 
	{ 
		//nlassert (numMipMap<=_MipMapCount);
		return _Data[numMipMap];
	}
	const std::vector<uint8>& getPixels(uint32 numMipMap = 0) const
	{
		//nlassert (numMipMap<=_MipMapCount);
		return _Data[numMipMap]; 
	}
	///@}
	
	
	/**
	 * Convert bitmap to another type
	 * conversion to rgba always work. No-op if already rgba.
	 * \param type new type for the bitmap
	 * \return true if conversion succeeded, false else
	 */
	bool convertToType (TType type);



	/** 
	 * Return the format of pixels stored at the present time in the object buffer.
	 * \return Pixel format (rgba luminance alpha alphaLuminance dxtc1 dxtc1Alpha dxtc3 dxtc5)
	 */	
	TType getPixelFormat() const
	{
		return PixelFormat; 
	}


	/** 
	 * Return the image width, or a mipmap width.
	 * \param mipMap mipmap level 
	 * \return image width (0 if mipmap not found)
	 */	
	virtual uint32 getWidth(uint32 numMipMap = 0) const;


	/** 
	 * Return the image height, or a mipmap height.
	 * \param mipMap mipmap level 
	 * \return image height (0 if mipmap not found)
	 */	
	virtual uint32 getHeight(uint32 numMipMap = 0) const;


	/** 
	 * Return the size (in pixels) of the image: <=> getHeight()*getWidth().
	 * \param mipMap mipmap level 
	 * \return image size (0 if mipmap not found)
	 */	
	uint32 getSize(uint32 numMipMap = 0) const;


	/** 
	 * Return the number of mipmaps. Level0 is a mipmap...
	 * \return number of mipmaps. 0 if no image at all. 1 if no mipmaping (for the base level).
	 */	
	uint32 getMipMapCount() const
	{
		return _MipMapCount; 
	}


	/** 
	 * Rotate a bitmap in CCW mode.
	 *
	 * \see releaseMipMaps().
	 */	
	void rotateCCW();

	/** 
	 * Build the mipmaps of the bitmap if they don't exist.
	 * Work only in RGBA mode...
	 * \see releaseMipMaps().
	 */	
	void buildMipMaps();

	/** 
	 * Release the mipmaps of the bitmap if they exist.
	 * Work for any mode.
	 * \see buildMipMaps().
	 */	
	void releaseMipMaps();

	/** 
	 * Reset the buffer. Mipmaps are deleted and bitmap is not valid anymore.
	 *
	 * \param type is the new type used for this texture
	 */	
	void reset(TType type=RGBA);
	
		
	/** 
	 * Resample the bitmap. If mipmaps exist they are deleted, then rebuilt
	 * after resampling.
	 * \param nNewWidth width after resample
	 * \param nNewHeight height after resample
	 */	
	void resample (sint32 nNewWidth, sint32 nNewHeight);
	
		
	/** 
	 * Resize the bitmap. If mipmaps exist they are deleted and not rebuilt.
	 * This is not a crop. Pixels are lost after resize.
	 *
	 * \param nNewWidth width after resize
	 * \param nNewHeight height after resize
	 * \param newType is the new type of the bitmap. If don_t_know, keep the same pixel format that before.
	 */	
	void resize (sint32 nNewWidth, sint32 nNewHeight, TType newType=DonTKnow);


	/** 
	 * Write a TGA (24 or 32 bits) from the object pixels buffer.
	 * If the current pixel format is not rgba then the method does nothing
	 * \param f IStream (must be a reading stream)
	 * \param d depth : 16 or 24 or 32
	 * \param upsideDown if true, the bitmap will be saved with the upside down
	 * \return true if succeed, false else
	 */	
	bool writeTGA(NLMISC::IStream &f, uint32 d, bool upsideDown = false);


	/**
	 * Tell the bitmap to load grayscale bitmap as alpha or luminance format.
	 *
	 * \param loadAsAlpha is true to load grayscale bitmaps as alpha. false to load grayscale bitmaps as luminance.
	 * default value is true.
	 */
	void loadGrayscaleAsAlpha (bool loadAsAlpha)
	{
		_LoadGrayscaleAsAlpha=loadAsAlpha;
	}


	/**
	 * Tell if the bitmap loads grayscale bitmap as alpha or luminance format.
	 *
	 * \return true is the bitmap loads grayscale bitmaps as alpha, false if it loads grayscale bitmaps as luminance.
	 */
	bool isGrayscaleAsAlpha () const
	{
		_LoadGrayscaleAsAlpha;
	}


	/**
	 * Perform a simple blit from the source to this bitmap at the (x, y) pos
	 * The dimension of the original bitmap are preserved
	 * For now, this texture and the source must have the same format
	 * With DXTC format, the dest coordinates must be a multiple of 4
	 * mipmap are noit rebuild when present
	 * \return true if the params were corrects and if the blit occures. In debug build there's an assertion
	 */
	bool blit(const CBitmap *src, sint32 x, sint32 y) ;


	/**
	 * Get the color in the bitmap given a pixel size
	 * The mipmaps must be built. If not just return the bilinear at the given point.
	 * The input x and y must be clamped between 0 and 1
	 */
	CRGBAF getColor (float x,float y);


	/**
	 * Horizontal flip (all the columns are flipped)
	 */
	void flipH();


	/**
	 * Vertical flip (all the rows are flipped)
	 */
	void flipV();

	/**
	 * Rotation of the bitmap of 90 degree in clockwise
	 */
	void rot90CW();

	/**
	 * Rotation of the bitmap of 90 degree in counter clockwise
	 */
	void rot90CCW();

	/** Set this bitmap as the result of the blend bewteen 2 bitmap
	  * REQUIRE : - Bm0 and Bm1 should have the same size.
	  *           - Both bitmap should be convertible to RGBA pixel format.
	  * The result is a RGBA bitmap.
	  * NB: this just works with the first mipmaps
	  * \param factor The blend factor. 0 means the result is equal to Bm0, 256 means the result is equal to Bm1
	  */
	void blend(const CBitmap &Bm0, const CBitmap &Bm1, uint16 factor);

};


} // NLMISC


#endif // NL_BITMAP_H

/* End of bitmap.h */
