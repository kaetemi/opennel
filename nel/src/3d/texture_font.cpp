/** \file texture_font.cpp
 * <File description>
 *
 * $Id: texture_font.cpp,v 1.10 2001/09/07 12:51:29 besson Exp $
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

#include "3d/texture_font.h"
#include "nel/misc/common.h"
#include "nel/misc/rect.h"
#include <map>

#include "nel/misc/file.h"

using namespace std;
using namespace NLMISC;

namespace NL3D
{

// Config 1
const int TextureSizeX = 1024;
const int TextureSizeY = 1024; // If change this value -> change NbLine too
const int Categories[TEXTUREFONT_NBCATEGORY] = { 8, 16, 24, 32, 64 };
const int NbLine[TEXTUREFONT_NBCATEGORY] = { 8, 24, 16, 4, 1 }; // Based on textsize

/*	
const int TextureSizeX = 256;
const int TextureSizeY = 256;
const int Categories[TEXTUREFONT_NBCATEGORY] = { 8, 16, 24, 32 };
const int NbLine[TEXTUREFONT_NBCATEGORY] = { 4, 6, 4, 1 }; // Based on textsize
*/
// ---------------------------------------------------------------------------
uint32 CTextureFont::SLetterKey::getVal()
{
	return Char + ((Size&255)<<16) + ((((uint32)FontGenerator)&255)<<24);
}

/*bool CTextureFont::SLetterKey::operator < (const CTextureFont::SLetterKey&k) const
{
	if (Char < k.Char)
		return true;
	if (Char > k.Char)
		return false;
	if (Size < k.Size)
		return true;
	if (Size > k.Size)
		return false;
	if (FontGenerator < k.FontGenerator)
		return true;
	if (FontGenerator > k.FontGenerator)
		return false;
	return false;
}

// ---------------------------------------------------------------------------
bool CTextureFont::SLetterKey::operator == (const CTextureFont::SLetterKey&k) const
{
	if ((Char == k.Char) && (Size == k.Size) && (FontGenerator == k.FontGenerator))
		return true;
	else
		return false;
}
*/
// ---------------------------------------------------------------------------
CTextureFont::CTextureFont()
{
	uint i;

	setFilterMode (ITexture::Linear, ITexture::LinearMipMapOff);

	setWrapS (ITexture::Repeat);
	setWrapT (ITexture::Repeat);

	setUploadFormat (Alpha);

	setReleasable (false);

	resize (TextureSizeX, TextureSizeY, CBitmap::Alpha);
	for(i = 0; i < TextureSizeX*TextureSizeY; ++i)
		getPixels()[i] = 0;
	// convertToType (CBitmap::Alpha);

	sint posY = 0;

	for(i = 0; i < TEXTUREFONT_NBCATEGORY; ++i)
	{
		// Number of chars per cache
		Letters[i].resize ((TextureSizeX/Categories[i])*NbLine[i]);

		for(uint32 j = 0; j < Letters[i].size(); ++j)
		{
			SLetterInfo &rLetter = Letters[i][j];

			rLetter.Char = -1;
			rLetter.FontGenerator = NULL;

			// The less recently used infos
			if (j < Letters[i].size()-1)
				rLetter.Next = &Letters[i][j+1];
			else
				rLetter.Next = NULL;

			if (j > 0)
				rLetter.Prev = &Letters[i][j-1];
			else
				rLetter.Prev = NULL;

			rLetter.Cat = i;

			sint sizeX = TextureSizeX/Categories[i];
			rLetter.U = (Categories[i]*(j%sizeX)) / ((float)TextureSizeX);
			rLetter.V = (posY + Categories[i]*((sint)(j/sizeX))) / ((float)TextureSizeY);

			/////////////////////////////////////////////////
			
			rLetter.CharWidth = rLetter.CharHeight = 0;
			rLetter.GlyphIndex = rLetter.Top = rLetter.Left = rLetter.AdvX = 0;
		}
		Front[i] = &Letters[i][0];
		Back[i] = &Letters[i][Letters[i].size()-1];
		posY += NbLine[i] * Categories[i];
	}
}


CTextureFont::~CTextureFont()
{
	sint posY = 0;
}

/*
void dumpTextureFont(CTextureFont*ctf)
{
	CBitmap b;
	COFile f( "c:\\temp\\zzz.tga" );
	b.resize (TextureSizeX, TextureSizeY, CBitmap::RGBA);
	vector<uint8>&bits = b.getPixels();
	vector<uint8>&src = ctf->getPixels();

	for (uint i = 0; i < (TextureSizeX*TextureSizeY); ++i)
	{
		bits[i*4+0] = bits[i*4+1] = bits[i*4+2] = bits[i*4+3] = src[i];
	}

	b.writeTGA (f, 32);
}
*/

// ---------------------------------------------------------------------------
// cat : categories where the letter is
// x : pos x of the letter
// y : pos y of the letter
void CTextureFont::rebuildLetter (sint cat, sint x, sint y)
{
	sint sizex = TextureSizeX / Categories[cat];
	sint index = x + y*sizex;
	SLetterInfo &rLetter = Letters[cat][index];

	if (rLetter.FontGenerator == NULL)
		return;

	sint catTopY = 0;
	sint c = 0;
	while (c < cat)
	{
		catTopY += NbLine[c] * Categories[c];
		++c;
	}
	// Destination position in pixel of the letter
	sint posx = x * Categories[cat];
	sint posy = catTopY + y * Categories[cat];

	uint32 pitch = 0;
	uint8 *bitmap = rLetter.FontGenerator->getBitmap (	rLetter.Char, rLetter.Size,
														rLetter.CharWidth, rLetter.CharHeight,
														pitch, rLetter.Left, rLetter.Top, 
														rLetter.AdvX, rLetter.GlyphIndex );

	// Copy FreeType buffer
	for (uint i = 0; i < rLetter.CharHeight; ++i)
	{
		uint8 *pDst = &_Data[0][posx + (posy+i)*TextureSizeY];
		uint8 *pSrc = &bitmap[i*pitch];
		for (uint j = 0; j < rLetter.CharWidth; ++j)
		{
			*pDst = *pSrc;
			++pDst;
			++pSrc;
		}
	}
	/*
	dumpTextureFont (this);
	int a = 5;
	a++;
	*/
}

// ---------------------------------------------------------------------------
void CTextureFont::doGenerate()
{
	// Rectangle invalidate ?
	if (_ListInvalidRect.begin()!=_ListInvalidRect.end())
	{
		// Yes, rebuild only those rectangles.

		// For each rectangle to compute
		std::list<NLMISC::CRect>::iterator ite=_ListInvalidRect.begin();
		while (ite!=_ListInvalidRect.end())
		{
			// Compute rectangle coordinates
			sint x = ite->left();
			sint y = ite->bottom();

			// Look in wich category is the rectangle
			sint cat = 0;
			sint catTopY = 0;
			sint catBotY = NbLine[cat] * Categories[cat];
			while (y >  catBotY)
			{
				if (y < catBotY)
					break;
				++cat;
				nlassert (cat < TEXTUREFONT_NBCATEGORY);
				catTopY = catBotY;
				catBotY += NbLine[cat] * Categories[cat];
			}

			x = x / Categories[cat];
			y = ite->top();
			y = y - catTopY;
			y = y / Categories[cat];
			
			rebuildLetter (cat, x, y);

			// Next rectangle
			ite++;
		}
	}
	else
	{
		for(int cat = 0; cat < TEXTUREFONT_NBCATEGORY; ++cat)
		{
			sint sizex = TextureSizeX / Categories[cat];
			sint sizey = NbLine[cat];
			for (sint y = 0; y < sizey; y++)
			for (sint x = 0; x < sizex; x++)
			{
				rebuildLetter (cat, x, y);
			}
		}
	}
/*
	dumpTextureFont (this);
	int a = 5;
*/
}

// ---------------------------------------------------------------------------
CTextureFont::SLetterInfo* CTextureFont::getLetterInfo (SLetterKey& k)
{
	sint cat;
	map<uint32, SLetterInfo*>::iterator itAccel = Accel.find (k.getVal());
	if (itAccel != Accel.end())
	{
		// Put it in the first place
		SLetterInfo *pLetterToMove = itAccel->second;
		cat = pLetterToMove->Cat;
		if (pLetterToMove != Front[cat])
		{
			pLetterToMove->Prev->Next = pLetterToMove->Next;

			if (pLetterToMove == Back[cat])
			{
				Back[cat] = pLetterToMove->Prev;
			}
			else
			{
				pLetterToMove->Next->Prev = pLetterToMove->Prev;
			}
			pLetterToMove->Prev = NULL;
			pLetterToMove->Next = Front[cat];
			Front[cat]->Prev = pLetterToMove;
			Front[cat] = pLetterToMove;
		}
		return pLetterToMove;
	}

	// The letter is not already present
	// Found the category of the new letter
	uint32 width, height;

	//k.FontGenerator->getSizes (k.Char, k.Size, width, height);
	// \todo mat : Temporaire !!! Essayer de faire intervenir le cache de freetype
	uint32 nPitch, nGlyphIndex;
	sint32 nLeft, nTop, nAdvX;
	k.FontGenerator->getBitmap (k.Char, k.Size, width, height, nPitch, nLeft, nTop, 
														nAdvX, nGlyphIndex );

	cat = 0;

	if (((sint)width > Categories[TEXTUREFONT_NBCATEGORY-1]) ||
		((sint)height > Categories[TEXTUREFONT_NBCATEGORY-1]))
		return NULL;

	while (((sint)width > Categories[cat]) || ((sint)height > Categories[cat]))
	{
		++cat;
		nlassert (cat != TEXTUREFONT_NBCATEGORY);
	}

	// And replace the less recently used letter
	SLetterKey k2;
	k2.Char = Back[cat]->Char;
	k2.FontGenerator = Back[cat]->FontGenerator;
	k2.Size = Back[cat]->Size;

	itAccel	= Accel.find (k2.getVal());
	if (itAccel != Accel.end())
	{
		Accel.erase (itAccel);
	}

	SLetterInfo *NewBack = Back[cat]->Prev;
	NewBack->Next = NULL;
	Back[cat]->Cat = cat;
	Back[cat]->Char = k.Char;
	Back[cat]->FontGenerator = k.FontGenerator;
	Back[cat]->Size = k.Size;
	Back[cat]->CharWidth = width;
	Back[cat]->CharHeight = height;
	Back[cat]->Top = nTop;
	Back[cat]->Left = nLeft;
	Back[cat]->AdvX = nAdvX;
	Back[cat]->Prev = NULL;
	Back[cat]->Next = Front[cat];
	Front[cat]->Prev = Back[cat];
	Front[cat] = Back[cat];
	Back[cat] = NewBack;

	Accel.insert (map<uint32, SLetterInfo*>::value_type(k.getVal(),Front[cat]));

	// Invalidate the zone
	sint index = (Front[cat] - &Letters[cat][0]);// / sizeof (SLetterInfo);
	sint sizex = TextureSizeX / Categories[cat];
	sint x = index % sizex;
	sint y = index / sizex;
	x = x * Categories[cat];
	y = y * Categories[cat];

	sint c = 0;
	while (c < cat)
	{
		y = y + NbLine[c] * Categories[c];
		++c;
	}

	CRect r (x, y, width, height);

	touchRect (r);

	return Front[cat];
}

} // NL3D
