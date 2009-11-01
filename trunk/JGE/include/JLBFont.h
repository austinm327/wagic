//-------------------------------------------------------------------------------------
//
// JGE++ is a hardware accelerated 2D game SDK for PSP/Windows.
//
// Licensed under the BSD license, see LICENSE in JGE root for details.
//
// Copyright (c) 2007 James Hui (a.k.a. Dr.Watson) <jhkhui@gmail.com>
//
//-------------------------------------------------------------------------------------


#ifndef JLBF_H
#define JLBF_H

#define PRINTF_BUFFER_SIZE		256
#define MAX_CHAR				256

#include "JRenderer.h"
#include <string>

//////////////////////////////////////////////////////////////////////////
///	Bitmap font class for LMNOpc's Bitmap Font Builder:
///		http://www.lmnopc.com/bitmapfontbuilder/
///
/// Two files are used for each font:
///		1: xxx.png, font bitmap.
///		2: xxx.dat, widths for each character
///	Each font contains 2 sets of characters ASCII code (32-159).
///
//////////////////////////////////////////////////////////////////////////
class JLBFont
{
public:

	//////////////////////////////////////////////////////////////////////////
	/// Constructor.
	///
	/// @param fontname - Name of the font WITHOUT extensions.
	/// @param lineheight - Font height.
	/// @param useVideoRAM - Indicate to use video RAM to store the font image or not (PSP only).
	///
	//////////////////////////////////////////////////////////////////////////
	JLBFont(const char *fontname, int lineheight, bool useVideoRAM=false);

	~JLBFont();

	//////////////////////////////////////////////////////////////////////////
	/// Rendering text to screen.
	///
	/// @param string - text for rendering.
	/// @param x - X position of text.
	/// @param y - Y position of text.
	/// @align - Text aligment.
	///
	//////////////////////////////////////////////////////////////////////////
	void		DrawString(const char *string, float x, float y, int align=JGETEXT_LEFT, float leftOffset = 0, float width = 0);
  void		DrawString(std::string s, float x, float y, int align=JGETEXT_LEFT, float leftOffset = 0, float width = 0);

	//////////////////////////////////////////////////////////////////////////
	/// Rendering text to screen with syntax similar to printf of C/C++.
	///
	/// @param x - X position of text.
	/// @param y - Y position of text.
	/// @param format - String formatting.
	///
	//////////////////////////////////////////////////////////////////////////
	void		printf(float x, float y, const char *format, ...);

	//////////////////////////////////////////////////////////////////////////
	/// Set font color.
	///
	/// @param color - color of font.
	///
	//////////////////////////////////////////////////////////////////////////
	void		SetColor(PIXEL_TYPE color);

	//////////////////////////////////////////////////////////////////////////
	/// Set scale for rendering.
	///
	/// @param scale - Scale for rendering characters.
	///
	//////////////////////////////////////////////////////////////////////////
	void		SetScale(float scale);

	//////////////////////////////////////////////////////////////////////////
	/// Set angle for rendering.
	///
	/// @param rot - Rotation angle in radian.
	///
	//////////////////////////////////////////////////////////////////////////
	void		SetRotation(float rot);

	//////////////////////////////////////////////////////////////////////////
	/// Set font tracking.
	///
	/// @param tracking - Font tracking.
	///
	//////////////////////////////////////////////////////////////////////////
	void		SetTracking(float tracking);

	//////////////////////////////////////////////////////////////////////////
	/// Get font color.
	///
	/// @return Font color.
	///
	//////////////////////////////////////////////////////////////////////////
	PIXEL_TYPE	GetColor() const;

	//////////////////////////////////////////////////////////////////////////
	/// Get rendering scale.
	///
	/// @return Rendering scale.
	///
	//////////////////////////////////////////////////////////////////////////
	float		GetScale() const;

	//////////////////////////////////////////////////////////////////////////
	/// Get rendering angle.
	///
	/// @return Rendering angle.
	///
	//////////////////////////////////////////////////////////////////////////
	float		GetRotation() const;

	//////////////////////////////////////////////////////////////////////////
	/// Get font tracking.
	///
	/// @return Font tracking.
	///
	//////////////////////////////////////////////////////////////////////////
	float		GetTracking() const;

	//////////////////////////////////////////////////////////////////////////
	/// Get height of font.
	///
	/// @return Height of font.
	///
	//////////////////////////////////////////////////////////////////////////
	float		GetHeight() const;

	//////////////////////////////////////////////////////////////////////////
	/// Get width of rendering string on screen.
	///
	/// @param string - NULL terminated string.
	///
	/// @return - Width in pixels
	///
	//////////////////////////////////////////////////////////////////////////
	float		GetStringWidth(const char *string) const;

	//////////////////////////////////////////////////////////////////////////
	/// There are usually 2 sets of characters in the font image. The first set
	/// is from index 0-127 and the second from 128-255. You should use this
	/// function to select which set of characters you want to use. The index
	/// base should be either 0 or 128.
	///
	/// @param base - Base for the character set to use.
	///
	//////////////////////////////////////////////////////////////////////////
	void		SetBase(int base);


private:
	static JRenderer*	mRenderer;

	JTexture*	mTexture;
	JQuad*		mQuad;

	float		mXPos[MAX_CHAR];
	float		mYPos[MAX_CHAR];
	float		mCharWidth[MAX_CHAR];

	float		mHeight;
	float		mScale;
	float		mRotation;
	float		mTracking;
	float       mSpacing;

	PIXEL_TYPE		mColor;
	int			mBlend;

	int			mBase;

};


#endif
