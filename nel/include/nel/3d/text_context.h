/** \file text_context.h
 * <File description>
 *
 * $Id: text_context.h,v 1.21 2001/03/27 12:10:17 berenguier Exp $
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

#ifndef NL_TEXT_CONTEXT_H
#define NL_TEXT_CONTEXT_H

#include "nel/3d/font_manager.h"
#include "nel/3d/font_generator.h"
#include "nel/3d/computed_string.h"


namespace NL3D {


/**
 * CTextContext
 *
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2000
 */
class CTextContext
{
	/// Driver
	IDriver		*_Driver;

	/// font manager
	NL3D::CFontManager	*_FontManager;
	
	/// font generator
	NL3D::CFontGenerator * _FontGen;
	
	/// font size;
	uint32 _FontSize;
	
	/// current text color
	NLMISC::CRGBA _Color;

	/// hotspot
	NL3D::CComputedString::THotSpot _HotSpot;

	/// X scale 
	float _ScaleX;
	
	/// Z scale
	float _ScaleZ;

	/// Y axe rotation angle
	float _RotateY;

	/// computed strings map
	std::map<uint32,CComputedString> _StringList;
	
	/// max x coordinate of last string printed
	float _XBound;

	/// maximum index reached
	sint32 _MaxIndex;

	/// true if text is shaded
	bool _Shaded;

	/// shade's extent
	float _ShadeExtent;


public:

	/// Constructor
	CTextContext()
	{
		_Driver= NULL;
		_FontManager= NULL;

		_FontGen = NULL;

		_FontSize = 12;

		_Color = NLMISC::CRGBA(0,0,0);

		_HotSpot = NL3D::CComputedString::BottomLeft;

		_ScaleX = 1;
		_ScaleZ = 1;
		_RotateY = 0;

		_XBound = 0;

		_MaxIndex = -1;

		_Shaded = false;
		_ShadeExtent = 0.001f;
	}

	/// set the driver.
	void	init(IDriver *drv, CFontManager *fmg)
	{
		nlassert(drv && fmg);
		_Driver= drv;
		_FontManager= fmg;
	}

	/**
	 * init the font generator. Must be called before any print
	 * \param (cf CFontGenerator constructor parameters)
	 */
	void setFontGenerator(const std::string fontFileName, const std::string fontExFileName = "")
	{
		_FontGen = new NL3D::CFontGenerator(fontFileName, fontExFileName);
	}


	/**
	 * set the font color
	 * \param color the font color
	 */
	void setColor(NLMISC::CRGBA color)
	{
		_Color = color;
	}

	/**
	 * set the font size. Should be called before the first print
	 * \param fonSize the font size
	 */
	void setFontSize(uint32 fontSize)
	{
		_FontSize = fontSize;
	}

	/**
	 * get the font size
	 * \return the font size
	 */
	uint32 getFontSize() const
	{
		return _FontSize;
	}

	/**
	 * set the hot spot
	 * \param fonSize the font size
	 */
	void setHotSpot(NL3D::CComputedString::THotSpot hotSpot)
	{
		_HotSpot = hotSpot;
	}

	/**
	 * set the X scale
	 * \param scaleX the X scale
	 */
	void setScaleX(float scaleX)
	{
		_ScaleX = scaleX;
	}

	/**
	 * set the Z scale
	 * \param scaleZ the Z scale
	 */
	void setScaleZ(float scaleZ)
	{
		_ScaleZ = scaleZ;
	}

	/**
	 * \return the X scale
	 */
	float getScaleX() const
	{
		return _ScaleX;
	}

	/**
	 * \return the Z scale
	 */
	float getScaleZ() const
	{
		 return _ScaleZ;
	}

	/**
	 * set the shade states
	 * \param the shade state
	 */
	void setShaded(bool b)
	{
		_Shaded = b;
	}

	/**
	 * \return the shade state
	 */
	bool getShaded() const 
	{
		 return _Shaded;
	}

	/**
	 * set the shadow's size
	 * \param the shade extent
	 */
	void setShadeExtent(float shext)
	{
		_ShadeExtent = shext;
	}

	/**
	 * get the hot spot
	 * \return the hot spot
	 */
	NL3D::CComputedString::THotSpot getHotSpot() const
	{
		return _HotSpot;
	}

	/**
	 * compute and add a string to the stack
	 * \param a string
	 * \return the index where string has been inserted
	 */
	uint32 textPush(const char *format, ...) 
	{ 
		nlassert(_FontGen);

		char *str;
		NLMISC_CONVERT_VARGS (str, format, NLMISC::MaxCStringSize);

		NL3D::CComputedString cptdstr;
		_FontManager->computeString(str,_FontGen,_Color,_FontSize,_Driver,cptdstr);
		_MaxIndex++;
		_StringList.insert(std::make_pair(_MaxIndex,cptdstr));
		return _MaxIndex;
	}

	/**
	 * computes an ucstring and adds the result to the stack
	 * \param an ucstring
	 * \return the index where computed string has been inserted
	 */
	uint32 textPush(ucstring str) 
	{ 
		nlassert(_FontGen);

		NL3D::CComputedString cptdstr;
		_FontManager->computeString(str,_FontGen,_Color,_FontSize,_Driver,cptdstr);
		_MaxIndex++;
		_StringList.insert(std::make_pair(_MaxIndex,cptdstr));
		return _MaxIndex;
	}
	
	/**
	 * remove a string from the list
	 */
	void erase(uint32 i) 
	{ 
		_StringList.erase(i);
	}

	/**
	 * empty the map
	 */
	void clear() 
	{ 
		_StringList.clear();
		_MaxIndex = -1;
	}

	/**
	 * print a string of the list
	 * (rq : it leaves the string in the stack)
	 */
	void printAt(float x, float z, uint32 i)
	{
		std::map<uint32,CComputedString>::iterator itstr = _StringList.find(i);
		nlassert(itstr!= _StringList.end());
		
		(*itstr).second.render2D(*_Driver,
								x,z,
								_HotSpot,
								_ScaleX,_ScaleZ);

		_XBound = x + (*itstr).second.StringWidth;

	}

	/**
	 * compute and print a ucstring at the location
	 */
	void printAt(float x, float z, ucstring ucstr)
	{
		NL3D::CComputedString cptdstr;

		if(_Shaded)
		{
			NL3D::CComputedString cptdstr1;
			_FontManager->computeString(ucstr,_FontGen,NLMISC::CRGBA(0,0,0),_FontSize,_Driver,cptdstr1);
			cptdstr1.render2D(*_Driver,x+_ShadeExtent,z-_ShadeExtent,_HotSpot,_ScaleX,_ScaleZ);
		}

		_FontManager->computeString(ucstr,_FontGen,_Color,_FontSize,_Driver,cptdstr);
		cptdstr.render2D(*_Driver,
							x,z,
							_HotSpot,
							_ScaleX,_ScaleZ);

		_XBound = x + cptdstr.StringWidth;
	}
	
	/**
	 * compute and print a string at the location
	 */
	void printfAt(float x, float z, const char * format, ...)
	{
		nlassert(_FontGen);

		char *str;
		NLMISC_CONVERT_VARGS (str, format, NLMISC::MaxCStringSize);

		if(_Shaded)
		{
			NL3D::CComputedString cptdstr1;
			_FontManager->computeString(str,_FontGen,NLMISC::CRGBA(0,0,0),_FontSize,_Driver,cptdstr1);
			cptdstr1.render2D(*_Driver,x+_ShadeExtent,z-_ShadeExtent,_HotSpot,_ScaleX,_ScaleZ);
		}

		NL3D::CComputedString cptdstr2;
		_FontManager->computeString(str,_FontGen,_Color,_FontSize,_Driver,cptdstr2);
		cptdstr2.render2D(*_Driver,x,z,_HotSpot,_ScaleX,_ScaleZ);

		_XBound = x + cptdstr2.StringWidth;
	}
	
	
	/**
	 *	operator[]
	 * \return the computed string
	 */
	CComputedString& operator[](uint32 i)
	{
		std::map<uint32,CComputedString>::iterator itstr = _StringList.find(i);
		
		return (*itstr).second;
	}


	/**
	 *	operator[]
	 * \return the computed string, NULL if not found.
	 */
	CComputedString		*getComputedString(uint32 i)
	{
		std::map<uint32,CComputedString>::iterator itstr = _StringList.find(i);

		if(itstr==_StringList.end())
			return NULL;
		else
			return &itstr->second;
	}


	/**
	 * Compute a string as primitive blocks using the
	 * font manager's method computeString
	 * \param a string
	 * \param the computed string
	 */
	void computeString(const std::string& s, CComputedString& output)
	{
		_FontManager->computeString(s,_FontGen,_Color,_FontSize,_Driver,output);
	}

	/**
	 * Compute a ucstring as primitive blocks using the
	 * font manager's method computeString
	 * \param an ucstring
	 * \param the computed string
	 */
	void computeString(const ucstring& s, CComputedString& output)
	{
		_FontManager->computeString(s,_FontGen,_Color,_FontSize,_Driver,output);
	}

	/**
	 * Return max x coordinate of last string printed. Useful to know if a string
	 * goes out of the screen (screen limit is supposed at x==4/3, should actually 
	 * depend on driver's frustum).
	 *	\return x coordinate
	 */
	float getLastXBound() const
	{
		return _XBound;
	}


	/// destructor
	~CTextContext()
	{
		if(_FontGen) delete _FontGen;
	}
};


} // NL3D


#endif // NL_TEXT_CONTEXT_H

/* End of text_context.h */
