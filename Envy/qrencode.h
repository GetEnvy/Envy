/*
* qrencode.h
* Amalgated subset of LibQREncode
*
* This file is part of Envy (getenvy.com) © 2016
* Portions copyright PeerProject 2016 and Kentaro Fukuchi 2006-2012 <kentaro@fukuchi.org>
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation (fsf.org);
* either version 2.1 of the License, or any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU Lesser General Public License for more details.
* (http://www.gnu.org/licenses/lgpl.html)
*/

#pragma once

//#ifdef __cplusplus
//extern "C" {
//#endif

	/* Maximum version (size) of QR-code symbol. (Targetting Version 3 29x29, Default 40) */
#define QRSPEC_VERSION_MAX 10
#define QRSPEC_WIDTH_MAX 177

#define QRSPEC_MODEID_NUM        1
#define QRSPEC_MODEID_AN         2
#define QRSPEC_MODEID_8          4
#define QRSPEC_MODEID_ECI        7
#define QRSPEC_MODEID_KANJI      8
#define QRSPEC_MODEID_FNC1FIRST  5
#define QRSPEC_MODEID_FNC1SECOND 9
#define QRSPEC_MODEID_STRUCTURE  3
#define QRSPEC_MODEID_TERMINATOR 0

	/* Encoding mode. (Targetting Byte-mode QR_MODE_8) */
	typedef enum {
		QR_MODE_NUL = -1,  ///< Terminator (NUL character). Internal use only
		QR_MODE_NUM = 0,   ///< Numeric mode
		QR_MODE_AN,        ///< Alphabet-numeric mode
		QR_MODE_8,         ///< 8-bit data mode
		QR_MODE_KANJI,     ///< Kanji (shift-jis) mode
		QR_MODE_STRUCTURE, ///< Internal use only
		QR_MODE_ECI,       ///< ECI mode
		QR_MODE_FNC1FIRST,  ///< FNC1, first position
		QR_MODE_FNC1SECOND, ///< FNC1, second position
	} QRencodeMode;

	/* Level of error correction. (Targetting 7% QR_ECLEVEL_L) */
	typedef enum {
		QR_ECLEVEL_L = 0, ///< lowest
		QR_ECLEVEL_M,
		QR_ECLEVEL_Q,
		QR_ECLEVEL_H      ///< highest
	} QRecLevel;

	typedef struct {
		int version;
		int width;
		unsigned char *data;
	} QRcode;

	typedef struct _QRcode_List {
		QRcode *code;
		struct _QRcode_List *next;
	} QRcode_List;

	typedef struct {
		int length;
		unsigned char *data;
	} BitStream;

	typedef struct {
		int dataLength;
		unsigned char *data;
		int eccLength;
		unsigned char *ecc;
	} RSblock;

	typedef struct {
		int version;
		int dataLength;
		int eccLength;
		unsigned char *datacode;
		unsigned char *ecccode;
		int b1;
		int blocks;
		RSblock *rsblock;
		int count;
	} QRRawCode;

	typedef unsigned char data_t;

	/* Reed-Solomon codec control block */
	struct _RS {
		int mm;            /* Bits per symbol */
		int nn;            /* Symbols per block (= (1<<mm)-1) */
		data_t *alpha_to;  /* log lookup table */
		data_t *index_of;  /* Antilog lookup table */
		data_t *genpoly;   /* Generator polynomial */
		int nroots;        /* Number of generator roots = number of parity symbols */
		int fcr;           /* First consecutive root, index form */
		int prim;          /* Primitive element, index form */
		int iprim;         /* prim-th root of 1, index form */
		int pad;           /* Padding bytes in shortened block */
		int gfpoly;
		struct _RS *next;
	};

	typedef struct _RS RS;
	typedef struct _QRinput QRinput;
	typedef struct _QRinput_List QRinput_List;

	struct _QRinput {
		int version;
		QRecLevel level;
		QRinput_List *head;
		QRinput_List *tail;
		int fnc1;
		unsigned char appid;
	};

	struct _QRinput_List {
		QRencodeMode mode;
		int size;				///< Size of data chunk (byte).
		unsigned char *data;	///< Data chunk.
		BitStream *bstream;
		QRinput_List *next;
	};

	typedef struct {
		int width; //< Edge length of the symbol
		int words;  //< Data capacity (bytes)
		int remainder; //< Remainder bit (bits)
		int ec[4];  //< Number of ECC code (bytes)
	} QRspec_Capacity;

	/**
	* Table of the capacity of symbols
	* See Table 1 (pp.13) and Table 12-16 (pp.30-36), JIS X0510:2004.
	*/
	static const QRspec_Capacity qrspecCapacity[QRSPEC_VERSION_MAX + 1] = {
		{  0,    0, 0, {   0,    0,    0,    0}},
		{ 21,   26, 0, {   7,   10,   13,   17}}, // 1
		{ 25,   44, 7, {  10,   16,   22,   28}},
		{ 29,   70, 7, {  15,   26,   36,   44}},
		{ 33,  100, 7, {  20,   36,   52,   64}},
		{ 37,  134, 7, {  26,   48,   72,   88}}, // 5
		{ 41,  172, 7, {  36,   64,   96,  112}},
		{ 45,  196, 0, {  40,   72,  108,  130}},
		{ 49,  242, 0, {  48,   88,  132,  156}},
		{ 53,  292, 0, {  60,  110,  160,  192}},
		{ 57,  346, 0, {  72,  130,  192,  224}}  //10
	};

	typedef struct {
		int width;
		unsigned char *frame;
		int x, y;
		int dir;
		int bit;
	} FrameFiller;

	static unsigned char *frames[QRSPEC_VERSION_MAX + 1];

	static int modnn(RS *rs, int x){
		while (x >= rs->nn) {
			x -= rs->nn;
			x = (x >> rs->mm) + (x & rs->nn);
		}
		return x;
	}

/* Demerit coefficients */
#define N1 (3)
#define N2 (3)
#define N3 (40)
#define N4 (10)

#define MODNN(x) modnn(rs,x)
#define MM (rs->mm)
#define NN (rs->nn)
#define ALPHA_TO (rs->alpha_to)
#define INDEX_OF (rs->index_of)
#define GENPOLY (rs->genpoly)
#define NROOTS (rs->nroots)
#define FCR (rs->fcr)
#define PRIM (rs->prim)
#define IPRIM (rs->iprim)
#define PAD (rs->pad)
#define A0 (NN)

#define STRUCTURE_HEADER_SIZE 20
#define MODE_INDICATOR_SIZE 4

#define QRspec_rsBlockNum(__spec__) (__spec__[0] + __spec__[3])
#define QRspec_rsBlockNum1(__spec__) (__spec__[0])
#define QRspec_rsDataCodes1(__spec__) (__spec__[1])
#define QRspec_rsEccCodes1(__spec__) (__spec__[2])
#define QRspec_rsBlockNum2(__spec__) (__spec__[3])
#define QRspec_rsDataCodes2(__spec__) (__spec__[4])
#define QRspec_rsEccCodes2(__spec__) (__spec__[2])

#define QRspec_rsDataLength(__spec__) \
	((QRspec_rsBlockNum1(__spec__) * QRspec_rsDataCodes1(__spec__)) + \
	 (QRspec_rsBlockNum2(__spec__) * QRspec_rsDataCodes2(__spec__)))
#define QRspec_rsEccLength(__spec__) \
	(QRspec_rsBlockNum(__spec__) * QRspec_rsEccCodes1(__spec__))

	// encodeString8bit() is same to encodeString(), but encodes whole data in 8-bit mode.
	//extern QRcode *QRcode_encodeString(const char *string, int version, QRecLevel level, QRencodeMode hint, int casesensitive);
	QRcode *QRcode_encodeString(const char *string);
	QRcode *QRcode_encodeString8bit(const char *string, int version, QRecLevel level);
	QRcode *QRcode_encodeData(const unsigned char *data, int length, int version, QRecLevel level);
	QRcode *QRcode_encodeInput(QRinput *input);
	QRcode *QRcode_encodeMask(QRinput *input, int mask);
	QRcode *QRcode_new(int version, int width, unsigned char *data);

	QRinput *QRinput_new(int version, QRecLevel level);
	QRinput_List *QRinput_List_newEntry(QRencodeMode mode, int size, const unsigned char *data);
	void QRinput_appendEntry(QRinput *input, QRinput_List *entry);
	int QRinput_append(QRinput *input, QRencodeMode mode, int size, const unsigned char *data);
	unsigned char *QRinput_getByteStream(QRinput *input);
	BitStream *QRinput_getBitStream(QRinput *input);
	BitStream *QRinput_mergeBitStream(QRinput *input);
	int QRinput_convertData(QRinput *input);
	int QRinput_encodeBitStream(QRinput_List *entry, int version);
	int QRinput_createBitStream(QRinput *input);
	int QRinput_estimateBitStreamSize(QRinput *input, int version);
	int QRinput_estimateBitStreamSizeOfEntry(QRinput_List *entry, int version);
	int QRinput_estimateBitsModeECI(unsigned char *data);
	int QRinput_estimateVersion(QRinput *input);
	int QRinput_getVersion(QRinput *input);
	int QRinput_setVersion(QRinput *input, int version);
	int QRinput_encodeMode8(QRinput_List *entry, int version);
	int QRinput_encodeModeStructure(QRinput_List *entry);
	int QRinput_encodeModeFNC1Second(QRinput_List *entry, int version);
	int QRinput_encodeModeECI(QRinput_List *entry, int version);
	unsigned int QRinput_decodeECIfromByteArray(unsigned char *data);
	int QRinput_appendPaddingBit(BitStream *bstream, QRinput *input);
	int QRinput_insertFNC1Header(QRinput *input);
	int QRinput_isSplittableMode(QRencodeMode mode);

	unsigned char *QRspec_newFrame(int version);
	unsigned char *QRspec_createFrame(int version);
	int QRspec_maximumWords(QRencodeMode mode, int version);
	unsigned int QRspec_getFormatInfo(int mask, QRecLevel level);
	int QRspec_getRemainder(int version);
	int QRspec_getWidth(int version);
	int QRspec_getMinimumVersion(int size, QRecLevel level);
	int QRspec_getDataLength(int version, QRecLevel level);
	int QRspec_getECCLength(int version, QRecLevel level);
	void QRspec_getEccSpec(int version, QRecLevel level, int spec[5]);
	int QRspec_lengthIndicator(QRencodeMode mode, int version);
	void QRspec_putAlignmentMarker(unsigned char *frame, int width, int ox, int oy);
	void QRspec_putAlignmentPattern(int version, unsigned char *frame, int width);
	void putFinderPattern(unsigned char *frame, int width, int ox, int oy);

	void QRraw_free(QRRawCode *raw);
	QRRawCode *QRraw_new(QRinput *input);
	unsigned char QRraw_getCode(QRRawCode *raw);
	void RSblock_initBlock(RSblock *block, int dl, unsigned char *data, int el, unsigned char *ecc, RS *rs);
	int RSblock_init(RSblock *blocks, int spec[5], unsigned char *data, unsigned char *ecc);
	RS *init_rs_char(int symsize, int gfpoly, int fcr, int prim, int nroots, int pad);
	RS *init_rs(int symsize, int gfpoly, int fcr, int prim, int nroots, int pad);
	void encode_rs_char(RS *rs, const data_t *data, data_t *parity);

	BitStream *BitStream_new(void);
	int BitStream_append(BitStream *bstream, BitStream *arg);
	int BitStream_appendNum(BitStream *bstream, int bits, unsigned int num);
	int BitStream_appendBytes(BitStream *bstream, int size, unsigned char *data);
	BitStream *BitStream_newFromNum(int bits, unsigned int num);
	BitStream *BitStream_newFromBytes(int size, unsigned char *data);
	unsigned char *BitStream_toByte(BitStream *bstream);
	#define BitStream_size(__bstream__) (__bstream__->length)
	FrameFiller *FrameFiller_new(int width, unsigned char *frame);
	unsigned char *FrameFiller_next(FrameFiller *filler);

	unsigned char *Mask_mask(int width, unsigned char *frame, QRecLevel level);
	unsigned char *Mask_makeMask(int width, unsigned char *frame, int mask, QRecLevel level);
	int Mask_calcN2(int width, unsigned char *frame);
	int Mask_calcN1N3(int length, int *runLength);
	int Mask_calcRunLength(int width, unsigned char *frame, int dir, int *runLength);
	int Mask_evaluateSymbol(int width, unsigned char *frame);

	void QRcode_free(QRcode *qrcode);
	void QRraw_free(QRRawCode *raw);
	void QRinput_free(QRinput *input);
	void QRinput_List_freeEntry(QRinput_List *entry);
	void BitStream_free(BitStream *bstream);

//#ifdef __cplusplus
//}
//#endif
