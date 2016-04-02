/*
* qrencode.c
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

// QRcode* pQRC = QRcode_encodeString(szString, 3, QR_ECLEVEL_L, QR_MODE_8)

#include "qrencode.h"

//#ifdef __cplusplus
//extern "C" {
//#endif

	// Unused, prefer encodeString8bit below
	//static QRcode *QRcode_encodeString(const char *string, int version, QRecLevel level, QRencodeMode hint, int casesensitive)
	//{
	//	QRinput *input;
	//	QRcode *code;
	//	int ret;
	//
	//	if(string == NULL) {
	//		errno = EINVAL;
	//		return NULL;
	//	}
	//	if(hint != QR_MODE_8) {
	//		errno = EINVAL;
	//		return NULL;
	//	}
	//
	//	input = QRinput_new(version, level);
	//	if(input == NULL) return NULL;
	//
	//	ret = Split_splitStringToQRinput(string, input, hint, casesensitive);
	//	if(ret < 0) {
	//		QRinput_free(input);
	//		return NULL;
	//	}
	//	code = QRcode_encodeInput(input);
	//	QRinput_free(input);
	//
	//	return code;
	//}

	static QRcode *QRcode_encodeString(const char *string)
	{
		if(string == NULL) {
			errno = EINVAL;
			return NULL;
		}
		return QRcode_encodeData((unsigned char *)string, strlen(string), 3, QR_ECLEVEL_L);
	}

	QRcode *QRcode_encodeString8bit(const char *string, int version, QRecLevel level)
	{
		if(string == NULL) {
			errno = EINVAL;
			return NULL;
		}
		return QRcode_encodeData((unsigned char *)string, strlen(string), version, level);
	}

	static QRcode *QRcode_encodeData(const unsigned char *data, int length, int version, QRecLevel level)
	{
		QRinput *input;
		QRcode *code;
		int ret;

		if(data == NULL || length == 0) {
			errno = EINVAL;
			return NULL;
		}

		input = QRinput_new(version, level);
		if(input == NULL) return NULL;

		ret = QRinput_append(input, QR_MODE_8, length, data);
		if(ret < 0) {
			QRinput_free(input);
			return NULL;
		}
		code = QRcode_encodeInput(input);
		QRinput_free(input);

		return code;
	}

	QRcode *QRcode_encodeInput(QRinput *input)
	{
		return QRcode_encodeMask(input, -1);
	}

	static QRcode *QRcode_encodeMask(QRinput *input, int mask)
	{
		QRRawCode *raw;
		QRcode *qrcode = NULL;
		FrameFiller *filler;
		unsigned char *frame, *masked = NULL, *p, code, bit;
		int width, version;

		if(input->version < 0 || input->version > QRSPEC_VERSION_MAX || input->level > QR_ECLEVEL_H) {
			errno = EINVAL;
			return NULL;
		}

		raw = QRraw_new(input);
		if(raw == NULL) return NULL;

		version = raw->version;
		width = QRspec_getWidth(version);
		frame = QRspec_newFrame(version);
		if(frame == NULL) {
			QRraw_free(raw);
			return NULL;
		}
		filler = FrameFiller_new(width, frame);
		if(filler == NULL) {
			QRraw_free(raw);
			free(frame);
			return NULL;
		}

		/* inteleaved data and ecc codes */
		for(int i=0; i<raw->dataLength + raw->eccLength; i++) {
			code = QRraw_getCode(raw);
			bit = 0x80;
			for(int j=0; j<8; j++) {
				p = FrameFiller_next(filler);
				if(p == NULL)  goto EXIT;
				*p = 0x02 | ((bit & code) != 0);
				bit = bit >> 1;
			}
		}
		QRraw_free(raw);
		raw = NULL;
		/* remainder bits */
		int j = QRspec_getRemainder(version);
		for(int i=0; i<j; i++) {
			p = FrameFiller_next(filler);
			if(p == NULL)  goto EXIT;
			*p = 0x02;
		}

		/* masking */
		if(mask == -2) { // just for debug purpose
		//	masked = (unsigned char *)malloc(width * width);
		//	memcpy(masked, frame, width * width);
		} else if(mask < 0) {
			masked = Mask_mask(width, frame, input->level);
		} else {
			masked = Mask_makeMask(width, frame, mask, input->level);
		}
		if(masked == NULL) {
			goto EXIT;
		}
		qrcode = QRcode_new(version, width, masked);
		if(qrcode == NULL) {
			free(masked);
		}

	EXIT:
		QRraw_free(raw);
		free(filler);
		free(frame);
		return qrcode;
	}

	static QRcode *QRcode_new(int version, int width, unsigned char *data)
	{
		QRcode *qrcode;

		qrcode = (QRcode *)malloc(sizeof(QRcode));
		if(qrcode == NULL) return NULL;

		qrcode->version = version;
		qrcode->width = width;
		qrcode->data = data;

		return qrcode;
	}


	/*****************************************************************************
	* Input Data
	*****************************************************************************/

	QRinput *QRinput_new(int version, QRecLevel level)
	{
		QRinput *input;

		if(version < 0 || version > QRSPEC_VERSION_MAX || level > QR_ECLEVEL_H) {
			errno = EINVAL;
			return NULL;
		}

		input = (QRinput *)malloc(sizeof(QRinput));
		if(input == NULL) return NULL;

		input->head = NULL;
		input->tail = NULL;
		input->version = version;
		input->level = level;
		input->fnc1 = 0;

		return input;
	}

	static QRinput_List *QRinput_List_newEntry(QRencodeMode mode, int size, const unsigned char *data)
	{
		QRinput_List *entry;

		// Assume QR_MODE_8

		entry = (QRinput_List *)malloc(sizeof(QRinput_List));
		if(entry == NULL) return NULL;

		entry->mode = mode;
		entry->size = size;
		if(size > 0) {
			entry->data = (unsigned char *)malloc(size);
			if(entry->data == NULL) {
				free(entry);
				return NULL;
			}
			memcpy(entry->data, data, size);
		}
		entry->bstream = NULL;
		entry->next = NULL;

		return entry;
	}

	static void QRinput_appendEntry(QRinput *input, QRinput_List *entry)
	{
		if(input->tail == NULL) {
			input->head = entry;
			input->tail = entry;
		} else {
			input->tail->next = entry;
			input->tail = entry;
		}
		entry->next = NULL;
	}

	int QRinput_append(QRinput *input, QRencodeMode mode, int size, const unsigned char *data)
	{
		QRinput_List *entry;

		entry = QRinput_List_newEntry(mode, size, data);
		if(entry == NULL) {
			return -1;
		}

		QRinput_appendEntry(input, entry);

		return 0;
	}

	unsigned char *QRinput_getByteStream(QRinput *input)
	{
		BitStream *bstream;
		unsigned char *array;

		bstream = QRinput_getBitStream(input);
		if(bstream == NULL) {
			return NULL;
		}
		array = BitStream_toByte(bstream);
		BitStream_free(bstream);

		return array;
	}

	static BitStream *QRinput_getBitStream(QRinput *input)
	{
		BitStream *bstream;
		int ret;

		bstream = QRinput_mergeBitStream(input);
		if(bstream == NULL) {
			return NULL;
		}

		ret = QRinput_appendPaddingBit(bstream, input);
		if(ret < 0) {
			BitStream_free(bstream);
			return NULL;
		}

		return bstream;
	}

	static BitStream *QRinput_mergeBitStream(QRinput *input)
	{
		BitStream *bstream;
		QRinput_List *list;
		int ret;

		if(input->fnc1) {
			if(QRinput_insertFNC1Header(input) < 0) {
				return NULL;
			}
		}
		if(QRinput_convertData(input) < 0) {
			return NULL;
		}

		bstream = BitStream_new();
		if(bstream == NULL) return NULL;

		list = input->head;
		while(list != NULL) {
			ret = BitStream_append(bstream, list->bstream);
			if(ret < 0) {
				BitStream_free(bstream);
				return NULL;
			}
			list = list->next;
		}

		return bstream;
	}

	static int QRinput_insertFNC1Header(QRinput *input)
	{
		QRinput_List *entry = NULL;

		if(input->fnc1 == 1) {
			entry = QRinput_List_newEntry(QR_MODE_FNC1FIRST, 0, NULL);
		} else if(input->fnc1 == 2) {
			entry = QRinput_List_newEntry(QR_MODE_FNC1SECOND, 1, &(input->appid));
		}
		if(entry == NULL) {
			return -1;
		}

		if(input->head->mode != QR_MODE_STRUCTURE || input->head->mode != QR_MODE_ECI) {
			entry->next = input->head;
			input->head = entry;
		} else {
			entry->next = input->head->next;
			input->head->next = entry;
		}

		return 0;
	}

	static int QRinput_convertData(QRinput *input)
	{
		int bits;
		int ver;

		ver = QRinput_estimateVersion(input);
		if(ver > QRinput_getVersion(input)) {
			QRinput_setVersion(input, ver);
		}

		for(;;) {
			bits = QRinput_createBitStream(input);
			if(bits < 0) return -1;
			ver = QRspec_getMinimumVersion((bits + 7) / 8, input->level);
			if(ver < 0) {
				errno = ERANGE;
				return -1;
			} else if(ver > QRinput_getVersion(input)) {
				QRinput_setVersion(input, ver);
			} else {
				break;
			}
		}

		return 0;
	}

	static int QRinput_encodeBitStream(QRinput_List *entry, int version)
	{
		int words, ret = 0;
		QRinput_List *st1 = NULL, *st2 = NULL;

		if(entry->bstream != NULL) {
			BitStream_free(entry->bstream);
			entry->bstream = NULL;
		}

		words = QRspec_maximumWords(entry->mode, version);
		if(words != 0 && entry->size > words) {
				st1 = QRinput_List_newEntry(entry->mode, words, entry->data);
			if(st1 == NULL)
				ret = -1;
			if(ret >= 0)
				st2 = QRinput_List_newEntry(entry->mode, entry->size - words, &entry->data[words]);
			if(st2 == NULL)
				ret = -1;

			if(ret >= 0)
				ret = QRinput_encodeBitStream(st1, version);
			if(ret >= 0)
				ret = QRinput_encodeBitStream(st2, version);
			if(ret >= 0)
				entry->bstream = BitStream_new();
			if(entry->bstream == NULL)
				ret = -1;
			if(ret >= 0)
				ret = BitStream_append(entry->bstream, st1->bstream);
			if(ret >= 0)
				ret = BitStream_append(entry->bstream, st2->bstream);

			QRinput_List_freeEntry(st1);
			QRinput_List_freeEntry(st2);
		} else {
			switch(entry->mode) {
		//	case QR_MODE_NUM:
		//		ret = QRinput_encodeModeNum(entry, version);
		//		break;
		//	case QR_MODE_AN:
		//		ret = QRinput_encodeModeAn(entry, version);
		//		break;
			case QR_MODE_8:
				ret = QRinput_encodeMode8(entry, version);
				break;
			case QR_MODE_STRUCTURE:
				ret = QRinput_encodeModeStructure(entry);
				break;
			case QR_MODE_ECI:
				ret = QRinput_encodeModeECI(entry, version);
				break;
			case QR_MODE_FNC1SECOND:
				ret = QRinput_encodeModeFNC1Second(entry, version);
				break;
			//default:
			//	break;
			}
		}

		if(ret < 0) return -1;
		return BitStream_size(entry->bstream);
	}

	static int QRinput_createBitStream(QRinput *input)
	{
		QRinput_List *list;
		int bits, total = 0;

		list = input->head;
		while(list != NULL) {
			bits = QRinput_encodeBitStream(list, input->version);
			if(bits < 0) return -1;
			total += bits;
			list = list->next;
		}

		return total;
	}

	static int QRinput_estimateBitStreamSize(QRinput *input, int version)
	{
		QRinput_List *list;
		int bits = 0;

		list = input->head;
		while(list != NULL) {
			bits += QRinput_estimateBitStreamSizeOfEntry(list, version);
			list = list->next;
		}

		return bits;
	}

	static int QRinput_estimateVersion(QRinput *input)
	{
		int bits;
		int version, prev;

		version = 0;
		do {
			prev = version;
			bits = QRinput_estimateBitStreamSize(input, prev);
			version = QRspec_getMinimumVersion((bits + 7) / 8, input->level);
			if (version < 0) return -1;
		} while (version > prev);

		return version;
	}

	int QRinput_getVersion(QRinput *input)
	{
		return input->version;
	}

	int QRinput_setVersion(QRinput *input, int version)
	{
		if(version < 0 || version > QRSPEC_VERSION_MAX) {
			errno = EINVAL;
			return -1;
		}

		input->version = version;

		return 0;
	}

	static int QRinput_estimateBitStreamSizeOfEntry(QRinput_List *entry, int version)
	{
		int bits = 0;
		int l, m;
		int num;

		if(version == 0) version = 1;

		switch(entry->mode) {
	//	case QR_MODE_NUM:
	//		bits = QRinput_estimateBitsModeNum(entry->size);
	//		break;
	//	case QR_MODE_AN:
	//		bits = QRinput_estimateBitsModeAn(entry->size);
	//		break;
		case QR_MODE_8:
			bits = entry->size * 8;		// QRinput_estimateBitsMode8()
			break;
		case QR_MODE_STRUCTURE:
			return STRUCTURE_HEADER_SIZE;
		case QR_MODE_ECI:
			bits = QRinput_estimateBitsModeECI(entry->data);
			break;
		case QR_MODE_FNC1FIRST:
			return MODE_INDICATOR_SIZE;
		case QR_MODE_FNC1SECOND:
			return MODE_INDICATOR_SIZE + 8;
		//default:
		//	return 0;
		}

		l = QRspec_lengthIndicator(entry->mode, version);
		m = 1 << l;
		num = (entry->size + m - 1) / m;

		bits += num * (MODE_INDICATOR_SIZE + l);

		return bits;
	}

	int QRinput_estimateBitsModeECI(unsigned char *data)
	{
		unsigned int ecinum = QRinput_decodeECIfromByteArray(data);

		/* See Table 4 of JISX 0510:2004 pp.17. */
		if(ecinum < 128)
			return MODE_INDICATOR_SIZE + 8;
		if(ecinum < 16384)
			return MODE_INDICATOR_SIZE + 16;

			return MODE_INDICATOR_SIZE + 24;
	}

	static int QRinput_encodeMode8(QRinput_List *entry, int version)
	{
		entry->bstream = BitStream_new();
		if(entry->bstream == NULL) return -1;

		int ret = BitStream_appendNum(entry->bstream, 4, QRSPEC_MODEID_8);
		if(ret >= 0)
			ret = BitStream_appendNum(entry->bstream, QRspec_lengthIndicator(QR_MODE_8, version), entry->size);
		if(ret >= 0)
			ret = BitStream_appendBytes(entry->bstream, entry->size, entry->data);
		if(ret >= 0)
			return 0;	// Success

		// Fail:
		BitStream_free(entry->bstream);
		entry->bstream = NULL;
		return -1;
	}

	static int QRinput_encodeModeStructure(QRinput_List *entry)
	{
		entry->bstream = BitStream_new();
		if(entry->bstream == NULL) return -1;

		int ret = BitStream_appendNum(entry->bstream, 4, QRSPEC_MODEID_STRUCTURE);
		if(ret >= 0)
			ret = BitStream_appendNum(entry->bstream, 4, entry->data[1] - 1);
		if(ret >= 0)
			ret = BitStream_appendNum(entry->bstream, 4, entry->data[0] - 1);
		if(ret >= 0)
			ret = BitStream_appendNum(entry->bstream, 8, entry->data[2]);
		if(ret >= 0)
			return 0;	// Success

		// Fail:
		BitStream_free(entry->bstream);
		entry->bstream = NULL;
		return -1;
	}

	static int QRinput_encodeModeFNC1Second(QRinput_List *entry, int version)
	{
		entry->bstream = BitStream_new();
		if(entry->bstream == NULL) return -1;

		int ret = BitStream_appendNum(entry->bstream, 4, QRSPEC_MODEID_FNC1SECOND);
		if(ret >= 0)
			ret = BitStream_appendBytes(entry->bstream, 1, entry->data);
		if(ret >= 0)
			return 0;	// Success

		// Fail:
		BitStream_free(entry->bstream);
		entry->bstream = NULL;
		return -1;
	}

	static int QRinput_encodeModeECI(QRinput_List *entry, int version)
	{
		int words;
		unsigned int ecinum, code;

		entry->bstream = BitStream_new();
		if(entry->bstream == NULL) return -1;

		ecinum = QRinput_decodeECIfromByteArray(entry->data);;

		/* See Table 4 of JISX 0510:2004 pp.17. */
		if(ecinum < 128) {
			words = 1;
			code = ecinum;
		} else if(ecinum < 16384) {
			words = 2;
			code = 0x8000 + ecinum;
		} else {
			words = 3;
			code = 0xc0000 + ecinum;
		}

		int ret = BitStream_appendNum(entry->bstream, 4, QRSPEC_MODEID_ECI);
		if(ret >= 0)
			ret = BitStream_appendNum(entry->bstream, words * 8, code);
		if(ret >= 0)
			return 0;	// Success

		// Fail:
		BitStream_free(entry->bstream);
		entry->bstream = NULL;
		return -1;
	}

	static unsigned int QRinput_decodeECIfromByteArray(unsigned char *data)
	{
		unsigned int ecinum = 0;

		for(int i=0; i<4; i++) {
			ecinum = ecinum << 8;
			ecinum |= data[3-i];
		}

		return ecinum;
	}

	static int QRinput_appendPaddingBit(BitStream *bstream, QRinput *input)
	{
		int bits, maxbits, words, maxwords, ret;
		BitStream *padding = NULL;
		unsigned char *padbuf;
		int padlen;

		bits = BitStream_size(bstream);
		maxwords = QRspec_getDataLength(input->version, input->level);
		maxbits = maxwords * 8;

		if(maxbits < bits) {
			errno = ERANGE;
			return -1;
		}
		if(maxbits == bits) {
			return 0;
		}

		if(maxbits - bits <= 4) {
			ret = BitStream_appendNum(bstream, maxbits - bits, 0);
			goto DONE;
		}

		words = (bits + 4 + 7) / 8;

		padding = BitStream_new();
		if(padding == NULL) return -1;
		ret = BitStream_appendNum(padding, words * 8 - bits, 0);
		if(ret < 0) goto DONE;

		padlen = maxwords - words;
		if(padlen > 0) {
			padbuf = (unsigned char *)malloc(padlen);
			if(padbuf == NULL) {
				ret = -1;
				goto DONE;
			}
			for(int i=0; i<padlen; i++) {
				padbuf[i] = (i&1)?0x11:0xec;
			}
			ret = BitStream_appendBytes(padding, padlen, padbuf);
			free(padbuf);
			if(ret < 0) {
				goto DONE;
			}
		}

		ret = BitStream_append(bstream, padding);

	DONE:
		BitStream_free(padding);
		return ret;
	}

	int QRinput_isSplittableMode(QRencodeMode mode)
	{
		return (mode >= QR_MODE_NUM && mode < QR_MODE_KANJI);
	}

	/******************************************************************************
	* Frame filling
	*****************************************************************************/

	static FrameFiller *FrameFiller_new(int width, unsigned char *frame)
	{
		FrameFiller *filler;

		filler = (FrameFiller *)malloc(sizeof(FrameFiller));
		if(filler == NULL) return NULL;
		filler->width = width;
		filler->frame = frame;
		filler->x = width - 1;
		filler->y = width - 1;
		filler->dir = -1;
		filler->bit = -1;

		return filler;
	}

	static unsigned char *FrameFiller_next(FrameFiller *filler)
	{
		unsigned char *p;
		int x, y, w;

		if(filler->bit == -1) {
			filler->bit = 0;
			return filler->frame + filler->y * filler->width + filler->x;
		}

		x = filler->x;
		y = filler->y;
		p = filler->frame;
		w = filler->width;

		if(filler->bit == 0) {
			x--;
			filler->bit++;
		} else {
			x++;
			y += filler->dir;
			filler->bit--;
		}

		if(filler->dir < 0) {
			if(y < 0) {
				y = 0;
				x -= 2;
				filler->dir = 1;
				if(x == 6) {
					x--;
					y = 9;
				}
			}
		} else {
			if(y == w) {
				y = w - 1;
				x -= 2;
				filler->dir = -1;
				if(x == 6) {
					x--;
					y -= 8;
				}
			}
		}
		if(x < 0 || y < 0) return NULL;

		filler->x = x;
		filler->y = y;

		if(p[y * w + x] & 0x80) {
			// This tail recursion could be optimized.
			return FrameFiller_next(filler);
		}
		return &p[y * w + x];
	}


	/*****************************************************************************
	*  BitStream
	*****************************************************************************/

	BitStream *BitStream_new(void)
	{
		BitStream *bstream;

		bstream = (BitStream *)malloc(sizeof(BitStream));
		if(bstream == NULL) return NULL;

		bstream->length = 0;
		bstream->data = NULL;

		return bstream;
	}

	unsigned char *BitStream_toByte(BitStream *bstream)
	{
		int size, bytes;
		unsigned char *data, v;
		unsigned char *p;

		size = BitStream_size(bstream);
		if(size == 0) {
			return NULL;
		}
		data = (unsigned char *)malloc((size + 7) / 8);
		if(data == NULL) {
			return NULL;
		}

		bytes = size / 8;

		p = bstream->data;
		for(int i=0; i<bytes; i++) {
			v = 0;
			for(int j=0; j<8; j++) {
				v = v << 1;
				v |= *p;
				p++;
			}
			data[i] = v;
		}
		if(size & 7) {
			v = 0;
			for(int j=0; j<(size & 7); j++) {
				v = v << 1;
				v |= *p;
				p++;
			}
			data[bytes] = v;
		}

		return data;
	}

	static int BitStream_allocate(BitStream *bstream, int length)
	{
		unsigned char *data;

		if(bstream == NULL) {
			return -1;
		}

		data = (unsigned char *)malloc(length);
		if(data == NULL) {
			return -1;
		}

		if(bstream->data) {
			free(bstream->data);
		}
		bstream->length = length;
		bstream->data = data;

		return 0;
	}

	int BitStream_append(BitStream *bstream, BitStream *arg)
	{
		unsigned char *data;

		if(arg == NULL) {
			return -1;
		}
		if(arg->length == 0) {
			return 0;
		}
		if(bstream->length == 0) {
			if(BitStream_allocate(bstream, arg->length)) {
				return -1;
			}
			memcpy(bstream->data, arg->data, arg->length);
			return 0;
		}

		data = (unsigned char *)malloc(bstream->length + arg->length);
		if(data == NULL) {
			return -1;
		}
		memcpy(data, bstream->data, bstream->length);
		memcpy(data + bstream->length, arg->data, arg->length);

		free(bstream->data);
		bstream->length += arg->length;
		bstream->data = data;

		return 0;
	}

	int BitStream_appendNum(BitStream *bstream, int bits, unsigned int num)
	{
		BitStream *b;
		int ret;

		if(bits == 0) return 0;

		b = BitStream_newFromNum(bits, num);
		if(b == NULL) return -1;

		ret = BitStream_append(bstream, b);
		BitStream_free(b);

		return ret;
	}

	int BitStream_appendBytes(BitStream *bstream, int size, unsigned char *data)
	{
		BitStream *b;
		int ret;

		if(size == 0) return 0;

		b = BitStream_newFromBytes(size, data);
		if(b == NULL) return -1;

		ret = BitStream_append(bstream, b);
		BitStream_free(b);

		return ret;
	}

	static BitStream *BitStream_newFromNum(int bits, unsigned int num)
	{
		BitStream *bstream;
		unsigned char *p;
		unsigned int mask;

		bstream = BitStream_new();
		if(bstream == NULL) return NULL;

		if(BitStream_allocate(bstream, bits)) {
			BitStream_free(bstream);
			return NULL;
		}

		p = bstream->data;
		mask = 1 << (bits - 1);
		for(int i=0; i<bits; i++) {
			if(num & mask) {
				*p = 1;
			} else {
				*p = 0;
			}
			p++;
			mask = mask >> 1;
		}

		return bstream;
	}

	static BitStream *BitStream_newFromBytes(int size, unsigned char *data)
	{
		BitStream *bstream;
		unsigned char mask;
		unsigned char *p;

		bstream = BitStream_new();
		if(bstream == NULL) return NULL;

		if(BitStream_allocate(bstream, size * 8)) {
			BitStream_free(bstream);
			return NULL;
		}

		p = bstream->data;
		for(int i=0; i<size; i++) {
			mask = 0x80;
			for(int j=0; j<8; j++) {
				if(data[i] & mask) {
					*p = 1;
				} else {
					*p = 0;
				}
				p++;
				mask = mask >> 1;
			}
		}

		return bstream;
	}

	/*****************************************************************************
	* Raw
	*****************************************************************************/

	//static void QRraw_free(QRRawCode *raw);
	static QRRawCode *QRraw_new(QRinput *input)
	{
		QRRawCode *raw;
		int spec[5], ret;

		raw = (QRRawCode *)malloc(sizeof(QRRawCode));
		if(raw == NULL) return NULL;

		raw->datacode = QRinput_getByteStream(input);
		if(raw->datacode == NULL) {
			free(raw);
			return NULL;
		}

		QRspec_getEccSpec(input->version, input->level, spec);

		raw->version = input->version;
		raw->b1 = QRspec_rsBlockNum1(spec);
		raw->dataLength = QRspec_rsDataLength(spec);
		raw->eccLength = QRspec_rsEccLength(spec);
		raw->ecccode = (unsigned char *)malloc(raw->eccLength);
		if(raw->ecccode == NULL) {
			free(raw->datacode);
			free(raw);
			return NULL;
		}

		raw->blocks = QRspec_rsBlockNum(spec);
		raw->rsblock = (RSblock *)calloc(raw->blocks, sizeof(RSblock));
		if(raw->rsblock == NULL) {
			QRraw_free(raw);
			return NULL;
		}
		ret = RSblock_init(raw->rsblock, spec, raw->datacode, raw->ecccode);
		if(ret < 0) {
			QRraw_free(raw);
			return NULL;
		}

		raw->count = 0;

		return raw;
	}

	static unsigned char QRraw_getCode(QRRawCode *raw)
	{
		int col, row;
		unsigned char ret;

		if(raw->count < raw->dataLength) {
			row = raw->count % raw->blocks;
			col = raw->count / raw->blocks;
			if(col >= raw->rsblock[0].dataLength) {
				row += raw->b1;
			}
			ret = raw->rsblock[row].data[col];
		} else if(raw->count < raw->dataLength + raw->eccLength) {
			row = (raw->count - raw->dataLength) % raw->blocks;
			col = (raw->count - raw->dataLength) / raw->blocks;
			ret = raw->rsblock[row].ecc[col];
		} else {
			return 0;
		}
		raw->count++;
		return ret;
	}

	static void RSblock_initBlock(RSblock *block, int dl, unsigned char *data, int el, unsigned char *ecc, RS *rs)
	{
		block->dataLength = dl;
		block->data = data;
		block->eccLength = el;
		block->ecc = ecc;

		encode_rs_char(rs, data, ecc);
	}

	static int RSblock_init(RSblock *blocks, int spec[5], unsigned char *data, unsigned char *ecc)
	{
		RS *rs;
		RSblock *block;
		unsigned char *dp, *ep;
		int el, dl;

		dl = QRspec_rsDataCodes1(spec);
		el = QRspec_rsEccCodes1(spec);
		rs = init_rs(8, 0x11d, 0, 1, el, 255 - dl - el);
		if(rs == NULL) return -1;

		block = blocks;
		dp = data;
		ep = ecc;
		for(int i=0; i<QRspec_rsBlockNum1(spec); i++) {
			RSblock_initBlock(block, dl, dp, el, ep, rs);
			dp += dl;
			ep += el;
			block++;
		}

		if(QRspec_rsBlockNum2(spec) == 0) return 0;

		dl = QRspec_rsDataCodes2(spec);
		el = QRspec_rsEccCodes2(spec);
		rs = init_rs(8, 0x11d, 0, 1, el, 255 - dl - el);
		if(rs == NULL) return -1;
		for(int i=0; i<QRspec_rsBlockNum2(spec); i++) {
			RSblock_initBlock(block, dl, dp, el, ep, rs);
			dp += dl;
			ep += el;
			block++;
		}

		return 0;
	}

	static RS *rslist = NULL;
	static RS *init_rs_char(int symsize, int gfpoly, int fcr, int prim, int nroots, int pad)
	{
		// Common code for intializing a Reed-Solomon control block (char or int symbols) Copyright 2004 Phil Karn, KA9Q
		// May be used under the terms of the GNU Lesser General Public License (LGPL)
		//#undef NULL
		//#define NULL ((void *)0)

		RS *rs = NULL;

		int sr,root,iprim;

		/* Check parameter ranges */
		if(symsize < 0 || symsize > (int)(8*sizeof(data_t))) {
			return NULL;
		}

		if(fcr < 0 || fcr >= (1<<symsize))
			return NULL;
		if(prim <= 0 || prim >= (1<<symsize))
			return NULL;
		if(nroots < 0 || nroots >= (1<<symsize))
			return NULL; /* Can't have more roots than symbol values! */
		if(pad < 0 || pad >= ((1<<symsize) -1 - nroots))
			return NULL; /* Too much padding */

		rs = (RS *)calloc(1,sizeof(RS));
		if(rs == NULL)
			return NULL;

		rs->mm = symsize;
		rs->nn = (1<<symsize)-1;
		rs->pad = pad;

		rs->alpha_to = (data_t *)malloc(sizeof(data_t)*(rs->nn+1));
		if(rs->alpha_to == NULL){
			free(rs);
			return NULL;
		}
		rs->index_of = (data_t *)malloc(sizeof(data_t)*(rs->nn+1));
		if(rs->index_of == NULL){
			free(rs->alpha_to);
			free(rs);
			return NULL;
		}

		/* Generate Galois field lookup tables */
		rs->index_of[0] = A0; /* log(zero) = -inf */
		rs->alpha_to[A0] = 0; /* alpha**-inf = 0 */
		sr = 1;
		for(int i=0;i<rs->nn;i++){
			rs->index_of[sr] = i;
			rs->alpha_to[i] = sr;
			sr <<= 1;
			if(sr & (1<<symsize))
				sr ^= gfpoly;
			sr &= rs->nn;
		}
		if(sr != 1){
			/* field generator polynomial is not primitive! */
			free(rs->alpha_to);
			free(rs->index_of);
			free(rs);
			rs = NULL;
			return NULL;
		}

		/* Form RS code generator polynomial from its roots */
		rs->genpoly = (data_t *)malloc(sizeof(data_t)*(nroots+1));
		if(rs->genpoly == NULL){
			free(rs->alpha_to);
			free(rs->index_of);
			free(rs);
			rs = NULL;
			return NULL;
		}
		rs->fcr = fcr;
		rs->prim = prim;
		rs->nroots = nroots;
		rs->gfpoly = gfpoly;

		/* Find prim-th root of 1, used in decoding */
		for(iprim=1;(iprim % prim) != 0;iprim += rs->nn)
			;
		rs->iprim = iprim / prim;

		rs->genpoly[0] = 1;
		for (int i = 0,root=fcr*prim; i < nroots; i++,root += prim) {
			rs->genpoly[i+1] = 1;

			/* Multiply rs->genpoly[] by  @**(root + x) */
			for (int j = i; j > 0; j--){
				if (rs->genpoly[j] != 0)
					rs->genpoly[j] = rs->genpoly[j-1] ^ rs->alpha_to[modnn(rs,rs->index_of[rs->genpoly[j]] + root)];
				else
					rs->genpoly[j] = rs->genpoly[j-1];
			}
			/* rs->genpoly[0] can never be zero */
			rs->genpoly[0] = rs->alpha_to[modnn(rs,rs->index_of[rs->genpoly[0]] + root)];
		}
		/* convert rs->genpoly[] to index form for quicker encoding */
		for (int i = 0; i <= nroots; i++)
			rs->genpoly[i] = rs->index_of[rs->genpoly[i]];

		return rs;
	}

	RS *init_rs(int symsize, int gfpoly, int fcr, int prim, int nroots, int pad)
	{
		RS *rs;

		//#ifdef HAVE_LIBPTHREAD
		//	pthread_mutex_lock(&rslist_mutex);
		//#endif
		for(rs = rslist; rs != NULL; rs = rs->next) {
			if(rs->pad != pad) continue;
			if(rs->nroots != nroots) continue;
			if(rs->mm != symsize) continue;
			if(rs->gfpoly != gfpoly) continue;
			if(rs->fcr != fcr) continue;
			if(rs->prim != prim) continue;

			//#ifdef HAVE_LIBPTHREAD
			//	pthread_mutex_unlock(&rslist_mutex);
			//#endif
			return rs;
		}

		rs = init_rs_char(symsize, gfpoly, fcr, prim, nroots, pad);
		if(rs != NULL) {
			rs->next = rslist;
			rslist = rs;
		}

		//#ifdef HAVE_LIBPTHREAD
		//	pthread_mutex_unlock(&rslist_mutex);
		//#endif
		return rs;
	}

	void encode_rs_char(RS *rs, const data_t *data, data_t *parity)
	{
		data_t feedback;

		memset(parity,0,NROOTS*sizeof(data_t));

		for(int i=0;i<NN-NROOTS-PAD;i++){
			feedback = INDEX_OF[data[i] ^ parity[0]];
			if(feedback != A0){      /* feedback term is non-zero */
#ifdef UNNORMALIZED
				/* This line is unnecessary when GENPOLY[NROOTS] is unity, as must always be for the polynomials constructed by init_rs() */
				feedback = MODNN(NN - GENPOLY[NROOTS] + feedback);
#endif
				for(int j=1;j<NROOTS;j++)
					parity[j] ^= ALPHA_TO[MODNN(feedback + GENPOLY[NROOTS-j])];
			}
			/* Shift */
			memmove(&parity[0],&parity[1],sizeof(data_t)*(NROOTS-1));
			if(feedback != A0)
				parity[NROOTS-1] = ALPHA_TO[MODNN(feedback + GENPOLY[0])];
			else
				parity[NROOTS-1] = 0;
		}
	}


	/******************************************************************************
	* Mask
	*****************************************************************************/

#define MASKMAKER(__exp__) \
	int x, y;\
	int b = 0;\
\
	for(y=0; y<width; y++) {\
		for(x=0; x<width; x++) {\
			if(*s & 0x80) {\
				*d = *s;\
			} else {\
				*d = *s ^ ((__exp__) == 0);\
			}\
			b += (int)(*d & 1);\
			s++; d++;\
		}\
	}\
	return b;

	static int Mask_mask0(int width, const unsigned char *s, unsigned char *d)
	{
		MASKMAKER((x+y)&1)
	}

	static int Mask_mask1(int width, const unsigned char *s, unsigned char *d)
	{
		MASKMAKER(y&1)
	}

	static int Mask_mask2(int width, const unsigned char *s, unsigned char *d)
	{
		MASKMAKER(x%3)
	}

	static int Mask_mask3(int width, const unsigned char *s, unsigned char *d)
	{
		MASKMAKER((x+y)%3)
	}

	static int Mask_mask4(int width, const unsigned char *s, unsigned char *d)
	{
		MASKMAKER(((y/2)+(x/3))&1)
	}

	static int Mask_mask5(int width, const unsigned char *s, unsigned char *d)
	{
		MASKMAKER(((x*y)&1)+(x*y)%3)
	}

	static int Mask_mask6(int width, const unsigned char *s, unsigned char *d)
	{
		MASKMAKER((((x*y)&1)+(x*y)%3)&1)
	}

	static int Mask_mask7(int width, const unsigned char *s, unsigned char *d)
	{
		MASKMAKER((((x*y)%3)+((x+y)&1))&1)
	}

	#define maskNum (8)
	typedef int MaskMaker(int, const unsigned char *, unsigned char *);
	static MaskMaker *maskMakers[maskNum] = {
		Mask_mask0, Mask_mask1, Mask_mask2, Mask_mask3,
		Mask_mask4, Mask_mask5, Mask_mask6, Mask_mask7
	};

	static int Mask_writeFormatInformation(int width, unsigned char *frame, int mask, QRecLevel level)
	{
		unsigned int format;
		unsigned char v;
		int blacks = 0;

		format = QRspec_getFormatInfo(mask, level);

		for(int i=0; i<8; i++) {
			if(format & 1) {
				blacks += 2;
				v = 0x85;
			} else {
				v = 0x84;
			}
			frame[width * 8 + width - 1 - i] = v;
			if(i < 6) {
				frame[width * i + 8] = v;
			} else {
				frame[width * (i + 1) + 8] = v;
			}
			format= format >> 1;
		}
		for(int i=0; i<7; i++) {
			if(format & 1) {
				blacks += 2;
				v = 0x85;
			} else {
				v = 0x84;
			}
			frame[width * (width - 7 + i) + 8] = v;
			if(i == 0) {
				frame[width * 8 + 7] = v;
			} else {
				frame[width * 8 + 6 - i] = v;
			}
			format= format >> 1;
		}

		return blacks;
	}

	unsigned char *Mask_mask(int width, unsigned char *frame, QRecLevel level)
	{
		unsigned char *mask, *bestMask;
		int minDemerit = INT_MAX;
		int blacks;
		int bratio;
		int demerit;
		int w2 = width * width;

		mask = (unsigned char *)malloc(w2);
		if(mask == NULL) return NULL;
		bestMask = NULL;

		for(int i=0; i<maskNum; i++) {
			//		n1 = n2 = n3 = n4 = 0;
			demerit = 0;
			blacks = maskMakers[i](width, frame, mask);
			blacks += Mask_writeFormatInformation(width, mask, i, level);
			bratio = (200 * blacks + w2) / w2 / 2; /* (int)(100*blacks/w2+0.5) */
			demerit = (abs(bratio - 50) / 5) * N4;
			//		n4 = demerit;
			demerit += Mask_evaluateSymbol(width, mask);
			//		printf("(%d,%d,%d,%d)=%d\n", n1, n2, n3 ,n4, demerit);
			if(demerit < minDemerit) {
				minDemerit = demerit;
				free(bestMask);
				bestMask = mask;
				mask = (unsigned char *)malloc(w2);
				if(mask == NULL) break;
			}
		}
		free(mask);
		return bestMask;
	}

	unsigned char *Mask_makeMask(int width, unsigned char *frame, int mask, QRecLevel level)
	{
		unsigned char *masked;

		if(mask < 0 || mask >= maskNum) {
			errno = EINVAL;
			return NULL;
		}

		masked = (unsigned char *)malloc(width * width);
		if(masked == NULL) return NULL;

		maskMakers[mask](width, frame, masked);
		Mask_writeFormatInformation(width, masked, mask, level);

		return masked;
	}

	static int Mask_evaluateSymbol(int width, unsigned char *frame)
	{
		int runLength[QRSPEC_WIDTH_MAX + 1];
		int length;

		int demerit = Mask_calcN2(width, frame);

		for(int y=0; y<width; y++) {
			length = Mask_calcRunLength(width, frame + y * width, 0, runLength);
			demerit += Mask_calcN1N3(length, runLength);
		}

		for(int x=0; x<width; x++) {
			length = Mask_calcRunLength(width, frame + x, 1, runLength);
			demerit += Mask_calcN1N3(length, runLength);
		}

		return demerit;
	}

	static int Mask_calcN1N3(int length, int *runLength)
	{
		int demerit = 0;
		int fact;

		for(int i=0; i<length; i++) {
			if(runLength[i] >= 5) {
				demerit += N1 + (runLength[i] - 5);
				//n1 += N1 + (runLength[i] - 5);
			}
			if((i & 1)) {
				if(i >= 3 && i < length-2 && (runLength[i] % 3) == 0) {
					fact = runLength[i] / 3;
					if(runLength[i-2] == fact &&
						runLength[i-1] == fact &&
						runLength[i+1] == fact &&
						runLength[i+2] == fact) {
						if(i == 3 || runLength[i-3] >= 4 * fact) {
							demerit += N3;
							//n3 += N3;
						} else if(i+4 >= length || runLength[i+3] >= 4 * fact) {
							demerit += N3;
							//n3 += N3;
						}
					}
				}
			}
		}

		return demerit;
	}

	static int Mask_calcN2(int width, unsigned char *frame)
	{
		unsigned char *p;
		unsigned char b22, w22;
		int demerit = 0;

		p = frame + width + 1;
		for(int y=1; y<width; y++) {
			for(int x=1; x<width; x++) {
				b22 = p[0] & p[-1] & p[-width] & p [-width-1];
				w22 = p[0] | p[-1] | p[-width] | p [-width-1];
				if((b22 | (w22 ^ 1))&1) {
					demerit += N2;
				}
				p++;
			}
			p++;
		}

		return demerit;
	}

	static int Mask_calcRunLength(int width, unsigned char *frame, int dir, int *runLength)
	{
		int head;
		unsigned char *p;

		int pitch = (dir==0)?1:width;
		if(frame[0] & 1) {
			runLength[0] = -1;
			head = 1;
		} else {
			head = 0;
		}
		runLength[head] = 1;
		p = frame + pitch;

		for(int i=1; i<width; i++) {
			if((p[0] ^ p[-pitch]) & 1) {
				head++;
				runLength[head] = 1;
			} else {
				runLength[head]++;
			}
			p += pitch;
		}

		return head + 1;
	}


	/******************************************************************************
	* QRSpec
	*****************************************************************************/
	/**
	* Table of error correction code (Reed-Solomon block)
	*/
	static const int eccTable[QRSPEC_VERSION_MAX+1][4][2] = {
		{{ 0,  0}, { 0,  0}, { 0,  0}, { 0,  0}},
		{{ 1,  0}, { 1,  0}, { 1,  0}, { 1,  0}}, // 1
		{{ 1,  0}, { 1,  0}, { 1,  0}, { 1,  0}},
		{{ 1,  0}, { 1,  0}, { 2,  0}, { 2,  0}},
		{{ 1,  0}, { 2,  0}, { 2,  0}, { 4,  0}},
		{{ 1,  0}, { 2,  0}, { 2,  2}, { 2,  2}}, // 5
		{{ 2,  0}, { 4,  0}, { 4,  0}, { 4,  0}},
		{{ 2,  0}, { 4,  0}, { 2,  4}, { 4,  1}},
		{{ 2,  0}, { 2,  2}, { 4,  2}, { 4,  2}},
		{{ 2,  0}, { 3,  2}, { 4,  4}, { 4,  4}},
		{{ 2,  2}, { 4,  1}, { 6,  2}, { 6,  2}}  //10
	};

	static const int lengthTableBits[4][3] = {
		{10, 12, 14},
		{ 9, 11, 13},
		{ 8, 16, 16},
		{ 8, 10, 12}
	};

	static const int alignmentPattern[QRSPEC_VERSION_MAX+1][2] = {
		{ 0,  0},
		{ 0,  0}, {18,  0}, {22,  0}, {26,  0}, {30,  0}, // 1- 5
		{34,  0}, {22, 38}, {24, 42}, {26, 46}, {28, 50}  // 6-10
	};

	static const unsigned int formatInfo[4][8] = {
		{0x77c4, 0x72f3, 0x7daa, 0x789d, 0x662f, 0x6318, 0x6c41, 0x6976},
		{0x5412, 0x5125, 0x5e7c, 0x5b4b, 0x45f9, 0x40ce, 0x4f97, 0x4aa0},
		{0x355f, 0x3068, 0x3f31, 0x3a06, 0x24b4, 0x2183, 0x2eda, 0x2bed},
		{0x1689, 0x13be, 0x1ce7, 0x19d0, 0x0762, 0x0255, 0x0d0c, 0x083b}
	};

	unsigned int QRspec_getFormatInfo(int mask, QRecLevel level)
	{
		if(mask < 0 || mask > 7) return 0;

		return formatInfo[level][mask];
	}

	void QRspec_getEccSpec(int version, QRecLevel level, int spec[5])
	{
		int b1 = eccTable[version][level][0];
		int b2 = eccTable[version][level][1];
		int data = QRspec_getDataLength(version, level);
		int ecc  = QRspec_getECCLength(version, level);

		if(b2 == 0) {
			spec[0] = b1;
			spec[1] = data / b1;
			spec[2] = ecc / b1;
			spec[3] = spec[4] = 0;
		} else {
			spec[0] = b1;
			spec[1] = data / (b1 + b2);
			spec[2] = ecc  / (b1 + b2);
			spec[3] = b2;
			spec[4] = spec[1] + 1;
		}
	}

	int QRspec_getWidth(int version)
	{
		return qrspecCapacity[version].width;
	}

	int QRspec_getRemainder(int version)
	{
		return qrspecCapacity[version].remainder;
	}

	int QRspec_getDataLength(int version, QRecLevel level)
	{
		return qrspecCapacity[version].words - qrspecCapacity[version].ec[level];
	}

	int QRspec_getECCLength(int version, QRecLevel level)
	{
		return qrspecCapacity[version].ec[level];
	}

	int QRspec_getMinimumVersion(int size, QRecLevel level)
	{
		for(int i=1; i<= QRSPEC_VERSION_MAX; i++) {
			int words = qrspecCapacity[i].words - qrspecCapacity[i].ec[level];
			if(words >= size) return i;
		}

		return -1;
	}

	unsigned char *QRspec_newFrame(int version)
	{
		unsigned char *frame;

		if(version < 1 || version > QRSPEC_VERSION_MAX) return NULL;

		//#ifdef HAVE_LIBPTHREAD
		//	pthread_mutex_lock(&frames_mutex);
		//#endif
		if(frames[version] == NULL) {
			frames[version] = QRspec_createFrame(version);
		}
		//#ifdef HAVE_LIBPTHREAD
		//	pthread_mutex_unlock(&frames_mutex);
		//#endif
		if(frames[version] == NULL) return NULL;

		int width = qrspecCapacity[version].width;
		frame = (unsigned char *)malloc(width * width);
		if(frame == NULL) return NULL;
		memcpy(frame, frames[version], width * width);

		return frame;
	}

	static unsigned char *QRspec_createFrame(int version)
	{
		unsigned char *frame, *p, *q;
		unsigned int verinfo, v;

		int width = qrspecCapacity[version].width;
		frame = (unsigned char *)malloc(width * width);
		if(frame == NULL) return NULL;

		memset(frame, 0, width * width);
		/* Finder pattern */
		putFinderPattern(frame, width, 0, 0);
		putFinderPattern(frame, width, width - 7, 0);
		putFinderPattern(frame, width, 0, width - 7);
		/* Separator */
		p = frame;
		q = frame + width * (width - 7);
		for(int y=0; y<7; y++) {
			p[7] = 0xc0;
			p[width - 8] = 0xc0;
			q[7] = 0xc0;
			p += width;
			q += width;
		}
		memset(frame + width * 7, 0xc0, 8);
		memset(frame + width * 8 - 8, 0xc0, 8);
		memset(frame + width * (width - 8), 0xc0, 8);
		/* Mask format information area */
		memset(frame + width * 8, 0x84, 9);
		memset(frame + width * 9 - 8, 0x84, 8);
		p = frame + 8;
		for(int y=0; y<8; y++) {
			*p = 0x84;
			p += width;
		}
		p = frame + width * (width - 7) + 8;
		for(int y=0; y<7; y++) {
			*p = 0x84;
			p += width;
		}
		/* Timing pattern */
		p = frame + width * 6 + 8;
		q = frame + width * 8 + 6;
		for(int x=1; x<width-15; x++) {
			*p =  0x90 | (x & 1);
			*q =  0x90 | (x & 1);
			p++;
			q += width;
		}
		/* Alignment pattern */
		QRspec_putAlignmentPattern(version, frame, width);

		/* Version information */
		/* if(version >= 7) Unsupported */
		/* and a little bit... */
		frame[width * (width - 8) + 8] = 0x81;

		return frame;
	}

	static void putFinderPattern(unsigned char *frame, int width, int ox, int oy)
	{
		static const unsigned char finder[] = {
			0xc1, 0xc1, 0xc1, 0xc1, 0xc1, 0xc1, 0xc1,
			0xc1, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc1,
			0xc1, 0xc0, 0xc1, 0xc1, 0xc1, 0xc0, 0xc1,
			0xc1, 0xc0, 0xc1, 0xc1, 0xc1, 0xc0, 0xc1,
			0xc1, 0xc0, 0xc1, 0xc1, 0xc1, 0xc0, 0xc1,
			0xc1, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc1,
			0xc1, 0xc1, 0xc1, 0xc1, 0xc1, 0xc1, 0xc1,
		};
		int x, y;
		const unsigned char *s;

		frame += oy * width + ox;
		s = finder;
		for(y=0; y<7; y++) {
			for(x=0; x<7; x++) {
				frame[x] = s[x];
			}
			frame += width;
			s += 7;
		}
	}

	static void QRspec_putAlignmentPattern(int version, unsigned char *frame, int width)
	{
		int d, w, x, y, cx, cy;

		if(version < 2) return;

		d = alignmentPattern[version][1] - alignmentPattern[version][0];
		if(d < 0) {
			w = 2;
		} else {
			w = (width - alignmentPattern[version][0]) / d + 2;
		}

		if(w * w - 3 == 1) {
			x = alignmentPattern[version][0];
			y = alignmentPattern[version][0];
			QRspec_putAlignmentMarker(frame, width, x, y);
			return;
		}

		cx = alignmentPattern[version][0];
		for(x=1; x<w - 1; x++) {
			QRspec_putAlignmentMarker(frame, width,  6, cx);
			QRspec_putAlignmentMarker(frame, width, cx,  6);
			cx += d;
		}

		cy = alignmentPattern[version][0];
		for(y=0; y<w-1; y++) {
			cx = alignmentPattern[version][0];
			for(x=0; x<w-1; x++) {
				QRspec_putAlignmentMarker(frame, width, cx, cy);
				cx += d;
			}
			cy += d;
		}
	}

	static void QRspec_putAlignmentMarker(unsigned char *frame, int width, int ox, int oy)
	{
		static const unsigned char finder[] = {
			0xa1, 0xa1, 0xa1, 0xa1, 0xa1,
			0xa1, 0xa0, 0xa0, 0xa0, 0xa1,
			0xa1, 0xa0, 0xa1, 0xa0, 0xa1,
			0xa1, 0xa0, 0xa0, 0xa0, 0xa1,
			0xa1, 0xa1, 0xa1, 0xa1, 0xa1,
		};
		int x, y;
		const unsigned char *s;

		frame += (oy - 2) * width + ox - 2;
		s = finder;
		for(y=0; y<5; y++) {
			for(x=0; x<5; x++) {
				frame[x] = s[x];
			}
			frame += width;
			s += 5;
		}
	}

	int QRspec_lengthIndicator(QRencodeMode mode, int version)
	{
		int l;

		//if(!QRinput_isSplittableMode(mode)) return 0;
		if(version <= 9) {
			l = 0;
		} else if(version <= 26) {
			l = 1;
		} else {
			l = 2;
		}

		return lengthTableBits[mode][l];
	}

	int QRspec_maximumWords(QRencodeMode mode, int version)
	{
		int l;

		if(!QRinput_isSplittableMode(mode)) return 0;
		if(version <= 9) {
			l = 0;
		} else if(version <= 26) {
			l = 1;
		} else {
			l = 2;
		}

		int bits = lengthTableBits[mode][l];
		int words = (1 << bits) - 1;

		return words;
	}


	/*****************************************************************************
	* Free
	*****************************************************************************/

	void QRcode_free(QRcode *qrcode)
	{
		if(qrcode != NULL) {
			free(qrcode->data);
			free(qrcode);
		}
	}

	static void QRraw_free(QRRawCode *raw)
	{
		if(raw != NULL) {
			free(raw->datacode);
			free(raw->ecccode);
			free(raw->rsblock);
			free(raw);
		}
	}

	void QRinput_free(QRinput *input)
	{
		QRinput_List *list, *next;

		if(input != NULL) {
			list = input->head;
			while(list != NULL) {
				next = list->next;
				QRinput_List_freeEntry(list);
				list = next;
			}
			free(input);
		}
	}

	static void QRinput_List_freeEntry(QRinput_List *entry)
	{
		if(entry != NULL) {
			free(entry->data);
			BitStream_free(entry->bstream);
			free(entry);
		}
	}

	void BitStream_free(BitStream *bstream)
	{
		if(bstream != NULL) {
			free(bstream->data);
			free(bstream);
		}
	}

//#ifdef __cplusplus
//}
//#endif
