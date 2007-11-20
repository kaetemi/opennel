/**********************************************************************
 *<
	FILE: ligoscape_utility.h

	DESCRIPTION:	Template Utility

	CREATED BY:

	HISTORY:

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/

#ifndef __PLUGIN_MAX__H
#define __PLUGIN_MAX__H

#pragma conform(forScope, push)
#pragma conform(forScope, off)

#define _CRT_SECURE_NO_DEPRECATE

#include <Max.h>
#include <istdplug.h>
#include <iparamb2.h>
#include <iparamm2.h>

#include <utilapi.h>

#undef _CRT_SECURE_NO_DEPRECATE

#pragma conform(forScope, pop)

#undef min
#undef max

#include "resource.h"

extern TCHAR *GetString(int id);

extern HINSTANCE hInstance;

#endif // __PLUGIN_MAX__H

/* MERGE: this is the result of merging branch_mtr_nostlport with trunk (NEL-16)
 */
