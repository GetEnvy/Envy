//
// QRCode.h
//
// This file is part of Envy (getenvy.com) © 2016-2018
// All work here is original and released as-is under Persistent Public Domain [PPD]
//
// LibQREncode DLL Interface

#pragma once

//#include <QREncode\qrencode.h>	// Services DLL
//#include "qrencode.h"
//#include "qrencode.c"


class CQRCode
{
public:
	CQRCode();
//	~CQRCode();

public:
	CBitmap m_bmRemote;

	HBITMAP GetBitmap(CString str, UINT nPixelsize = 4);
	void UpdateRemote(CString str);
};

extern CQRCode QREncode;