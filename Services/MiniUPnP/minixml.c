/* $Id: minixml.c,v 1.11 2014/02/03 15:54:12 nanard Exp $ */
/* minixml.c : the minimum size a xml parser can be ! */
/* Project : miniupnp
 * webpage : http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * Author : Thomas Bernard
 * Copyright (c) 2005-2014, Thomas BERNARD
 * This software is subject to the conditions detailed in the
 * LICENCE file provided in this distribution.
 * */

#include <string.h>
#include "minixml.h"

/* parseatt : used to parse the argument list
 * return 0 (false) in case of success and -1 (true) if the end
 * of the xmlbuffer is reached. */
static int parseatt(struct xmlparser * p)
{
	const char * attname;
	int attnamelen;
	const char * attvalue;
	int attvaluelen;
	while(p->xml < p->xmlend)
	{
		if(*p->xml=='/' || *p->xml=='>')
			return 0;
		if( !IS_WHITE_SPACE(*p->xml) )
		{
			char sep;
			attname = p->xml;
			attnamelen = 0;
			while(*p->xml!='=' && !IS_WHITE_SPACE(*p->xml) )
			{
				attnamelen++; p->xml++;
				if(p->xml >= p->xmlend)
					return -1;
			}
			while(*(p->xml++) != '=')
			{
				if(p->xml >= p->xmlend)
					return -1;
			}
			while(IS_WHITE_SPACE(*p->xml))
			{
				p->xml++;
				if(p->xml >= p->xmlend)
					return -1;
			}
			sep = *p->xml;
			if(sep=='\'' || sep=='\"')
			{
				p->xml++;
				if(p->xml >= p->xmlend)
					return -1;
				attvalue = p->xml;
				attvaluelen = 0;
				while(*p->xml != sep)
				{
					attvaluelen++; p->xml++;
					if(p->xml >= p->xmlend)
						return -1;
				}
			}
			else
			{
				attvalue = p->xml;
				attvaluelen = 0;
				while(   !IS_WHITE_SPACE(*p->xml)
					  && *p->xml != '>' && *p->xml != '/')
				{
					attvaluelen++; p->xml++;
					if(p->xml >= p->xmlend)
						return -1;
				}
			}
			/*printf("%.*s='%.*s'\n",
					attnamelen, attname, attvaluelen, attvalue);*/
			if(p->attfunc)
				p->attfunc(p->data, attname, attnamelen, attvalue, attvaluelen);
		}
		p->xml++;
	}
	return -1;
}

/* parseelt parse the xml stream and
 * call the callback functions when needed... */
static void parseelt(struct xmlparser * p)
{
	int i;
	const char * elementname;
	while(p->xml < (p->xmlend - 1))
	{
		if((p->xml + 4) <= p->xmlend && (0 == memcmp(p->xml, "<!--", 4)))
		{
			p->xml += 3;
			/* ignore comments */
			do
			{
				p->xml++;
				if ((p->xml + 3) >= p->xmlend)
					return;
			}
			while(memcmp(p->xml, "-->", 3) != 0);
			p->xml += 3;
		}
		else if((p->xml)[0]=='<' && (p->xml)[1]!='?')
		{
			i = 0; elementname = ++p->xml;
			while( !IS_WHITE_SPACE(*p->xml)
				  && (*p->xml!='>') && (*p->xml!='/')
				 )
			{
				i++; p->xml++;
				if (p->xml >= p->xmlend)
					return;
				/* to ignore namespace : */
				if(*p->xml==':')
				{
					i = 0;
					elementname = ++p->xml;
				}
			}
			if(i>0)
			{
				if(p->starteltfunc)
					p->starteltfunc(p->data, elementname, i);
				if(parseatt(p))
					return;
				if(*p->xml!='/')
				{
					const char * data;
					i = 0; data = ++p->xml;
					if (p->xml >= p->xmlend)
						return;
					while( IS_WHITE_SPACE(*p->xml) )
					{
						i++; p->xml++;
						if (p->xml >= p->xmlend)
							return;
					}
					if(memcmp(p->xml, "<![CDATA[", 9) == 0)
					{
						/* CDATA handling */
						p->xml += 9;
						data = p->xml;
						i = 0;
						while(memcmp(p->xml, "]]>", 3) != 0)
						{
							i++; p->xml++;
							if ((p->xml + 3) >= p->xmlend)
								return;
						}
						if(i>0 && p->datafunc)
							p->datafunc(p->data, data, i);
						while(*p->xml!='<')
						{
							p->xml++;
							if (p->xml >= p->xmlend)
								return;
						}
					}
					else
					{
						while(*p->xml!='<')
						{
							i++; p->xml++;
							if ((p->xml + 1) >= p->xmlend)
								return;
						}
						if(i>0 && p->datafunc && *(p->xml + 1) == '/')
							p->datafunc(p->data, data, i);
					}
				}
			}
			else if(*p->xml == '/')
			{
				i = 0; elementname = ++p->xml;
				if (p->xml >= p->xmlend)
					return;
				while((*p->xml != '>'))
				{
					i++; p->xml++;
					if (p->xml >= p->xmlend)
						return;
				}
				if(p->endeltfunc)
					p->endeltfunc(p->data, elementname, i);
				p->xml++;
			}
		}
		else
		{
			p->xml++;
		}
	}
}

/* the parser must be initialized before calling this function */
void parsexml(struct xmlparser * parser)
{
	parser->xml = parser->xmlstart;
	parser->xmlend = parser->xmlstart + parser->xmlsize;
	parseelt(parser);
}
