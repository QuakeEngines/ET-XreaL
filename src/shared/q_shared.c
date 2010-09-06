/*
===========================================================================

Wolfenstein: Enemy Territory GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company. 

This file is part of the Wolfenstein: Enemy Territory GPL Source Code (Wolf ET Source Code).  

Wolf ET Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Wolf ET Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Wolf ET Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Wolf: ET Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Wolf ET Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

// q_shared.c -- stateless support routines that are included in each code dll
#include "q_shared.h"

// os x game bundles have no standard library links, and the defines are not always defined!

#ifdef MACOS_X
int qmax(int x, int y)
{
	return (((x) > (y)) ? (x) : (y));
}

int qmin(int x, int y)
{
	return (((x) < (y)) ? (x) : (y));
}
#endif

/*
============================================================================

GROWLISTS

============================================================================
*/

// malloc / free all in one place for debugging
//extern          "C" void *Com_Allocate(int bytes);
//extern          "C" void Com_Dealloc(void *ptr);

void Com_InitGrowList(growList_t * list, int maxElements)
{
	list->maxElements = maxElements;
	list->currentElements = 0;
	list->elements = (void **)Com_Allocate(list->maxElements * sizeof(void *));
}

void Com_DestroyGrowList(growList_t * list)
{
	Com_Dealloc(list->elements);
	memset(list, 0, sizeof(*list));
}

int Com_AddToGrowList(growList_t * list, void *data)
{
	void          **old;

	if(list->currentElements != list->maxElements)
	{
		list->elements[list->currentElements] = data;
		return list->currentElements++;
	}

	// grow, reallocate and move
	old = list->elements;

	if(list->maxElements < 0)
	{
		Com_Error(ERR_FATAL, "Com_AddToGrowList: maxElements = %i", list->maxElements);
	}

	if(list->maxElements == 0)
	{
		// initialize the list to hold 100 elements
		Com_InitGrowList(list, 100);
		return Com_AddToGrowList(list, data);
	}

	list->maxElements *= 2;

//  Com_DPrintf("Resizing growlist to %i maxElements\n", list->maxElements);

	list->elements = (void **)Com_Allocate(list->maxElements * sizeof(void *));

	if(!list->elements)
	{
		Com_Error(ERR_DROP, "Growlist alloc failed");
	}

	Com_Memcpy(list->elements, old, list->currentElements * sizeof(void *));

	Com_Dealloc(old);

	return Com_AddToGrowList(list, data);
}

void           *Com_GrowListElement(const growList_t * list, int index)
{
	if(index < 0 || index >= list->currentElements)
	{
		Com_Error(ERR_DROP, "Com_GrowListElement: %i out of range of %i", index, list->currentElements);
	}
	return list->elements[index];
}

int Com_IndexForGrowListElement(const growList_t * list, const void *element)
{
	int             i;

	for(i = 0; i < list->currentElements; i++)
	{
		if(list->elements[i] == element)
		{
			return i;
		}
	}
	return -1;
}

//=============================================================================

memStream_t *AllocMemStream(byte *buffer, int bufSize)
{
	memStream_t		*s;

	if(buffer == NULL || bufSize <= 0)
		return NULL;

	s = Com_Allocate(sizeof(memStream_t));
	if(s == NULL)
		return NULL;

	Com_Memset(s, 0, sizeof(memStream_t));

	s->buffer 	= buffer;
	s->curPos 	= buffer;
	s->bufSize	= bufSize;
	s->flags	= 0;

	return s;
}

void FreeMemStream(memStream_t * s)
{
	Com_Dealloc(s);
}

int MemStreamRead(memStream_t *s, void *buffer, int len)
{
	int				ret = 1;

	if(s == NULL || buffer == NULL)
		return 0;

	if(s->curPos + len > s->buffer + s->bufSize)
	{
		s->flags |= MEMSTREAM_FLAGS_EOF;
		len = s->buffer + s->bufSize - s->curPos;
		ret = 0;

		Com_Error(ERR_FATAL, "MemStreamRead: EOF reached");
	}

	Com_Memcpy(buffer, s->curPos, len);
	s->curPos += len;

	return ret;
}

int MemStreamGetC(memStream_t *s)
{
	int				c = 0;

	if(s == NULL)
		return -1;

	if(MemStreamRead(s, &c, 1) == 0)
		return -1;

	return c;
}

int MemStreamGetLong(memStream_t * s)
{
	int				c = 0;

	if(s == NULL)
		return -1;

	if(MemStreamRead(s, &c, 4) == 0)
		return -1;

	return LittleLong(c);
}

int MemStreamGetShort(memStream_t * s)
{
	int				c = 0;

	if(s == NULL)
		return -1;

	if(MemStreamRead(s, &c, 2) == 0)
		return -1;

	return LittleShort(c);
}

float MemStreamGetFloat(memStream_t * s)
{
	floatint_t		c;

	if(s == NULL)
		return -1;

	if(MemStreamRead(s, &c.i, 4) == 0)
		return -1;

	return LittleFloat(c.f);
}

//============================================================================

float Com_Clamp(float min, float max, float value)
{
	if(value < min)
	{
		return min;
	}
	if(value > max)
	{
		return max;
	}
	return value;
}


/*
COM_FixPath()
unixifies a pathname
*/

void COM_FixPath(char *pathname)
{
	while(*pathname)
	{
		if(*pathname == '\\')
		{
			*pathname = '/';
		}
		pathname++;
	}
}



/*
============
COM_SkipPath
============
*/
char           *COM_SkipPath(char *pathname)
{
	char           *last;

	last = pathname;
	while(*pathname)
	{
		if(*pathname == '/')
		{
			last = pathname + 1;
		}
		pathname++;
	}
	return last;
}

/*
============
COM_GetExtension
============
*/
const char     *COM_GetExtension(const char *name)
{
	int             length, i;

	length = strlen(name) - 1;
	i = length;

	while(name[i] != '.')
	{
		i--;
		if(name[i] == '/' || i == 0)
			return "";			// no extension
	}

	return &name[i + 1];
}

/*
============
COM_StripExtension
============
*/
void COM_StripExtension(const char *in, char *out)
{
	while(*in && *in != '.')
	{
		*out++ = *in++;
	}
	*out = 0;
}

/*
============
COM_StripExtension2
a safer version
============
*/
void COM_StripExtension2(const char *in, char *out, int destsize)
{
	int             len = 0;

	while(len < destsize - 1 && *in && *in != '.')
	{
		*out++ = *in++;
		len++;
	}
	*out = 0;
}

void COM_StripFilename(char *in, char *out)
{
	char           *end;

	Q_strncpyz(out, in, strlen(in) + 1);
	end = COM_SkipPath(out);
	*end = 0;
}

/*
============
COM_StripExtension3

RB: ioquake3 version
============
*/
void COM_StripExtension3(const char *src, char *dest, int destsize)
{
	int             length;

	Q_strncpyz(dest, src, destsize);

	length = strlen(dest) - 1;

	while(length > 0 && dest[length] != '.')
	{
		length--;

		if(dest[length] == '/')
			return;				// no extension
	}

	if(length)
	{
		dest[length] = 0;
	}
}


/*
==================
COM_DefaultExtension
==================
*/
void COM_DefaultExtension(char *path, int maxSize, const char *extension)
{
	char            oldPath[MAX_QPATH];
	char           *src;

//
// if path doesn't have a .EXT, append extension
// (extension should include the .)
//
	src = path + strlen(path) - 1;

	while(*src != '/' && src != path)
	{
		if(*src == '.')
		{
			return;				// it has an extension
		}
		src--;
	}

	Q_strncpyz(oldPath, path, sizeof(oldPath));
	Com_sprintf(path, maxSize, "%s%s", oldPath, extension);
}

//============================================================================
/*
==================
COM_BitCheck

  Allows bit-wise checks on arrays with more than one item (> 32 bits)
==================
*/
qboolean COM_BitCheck(const int array[], int bitNum)
{
	int             i;

	i = 0;
	while(bitNum > 31)
	{
		i++;
		bitNum -= 32;
	}

	return ((array[i] & (1 << bitNum)) != 0);	// (SA) heh, whoops. :)
}

/*
==================
COM_BitSet

  Allows bit-wise SETS on arrays with more than one item (> 32 bits)
==================
*/
void COM_BitSet(int array[], int bitNum)
{
	int             i;

	i = 0;
	while(bitNum > 31)
	{
		i++;
		bitNum -= 32;
	}

	array[i] |= (1 << bitNum);
}

/*
==================
COM_BitClear

  Allows bit-wise CLEAR on arrays with more than one item (> 32 bits)
==================
*/
void COM_BitClear(int array[], int bitNum)
{
	int             i;

	i = 0;
	while(bitNum > 31)
	{
		i++;
		bitNum -= 32;
	}

	array[i] &= ~(1 << bitNum);
}

//============================================================================

/*
============
Com_HashKey
============
*/
int Com_HashKey(char *string, int maxlen)
{
	int register    hash, i;

	hash = 0;
	for(i = 0; i < maxlen && string[i] != '\0'; i++)
	{
		hash += string[i] * (119 + i);
	}
	hash = (hash ^ (hash >> 10) ^ (hash >> 20));
	return hash;
}

//============================================================================

/*
============================================================================

					BYTE ORDER FUNCTIONS

============================================================================
*/

// can't just use function pointers, or dll linkage can
// mess up when qcommon is included in multiple places
static short    (*_BigShort) (short l) = NULL;
static short    (*_LittleShort) (short l) = NULL;
static int      (*_BigLong) (int l) = NULL;
static int      (*_LittleLong) (int l) = NULL;
static          qint64(*_BigLong64) (qint64 l) = NULL;
static          qint64(*_LittleLong64) (qint64 l) = NULL;
static float    (*_BigFloat) (float l) = NULL;
static float    (*_LittleFloat) (float l) = NULL;

short LittleShort(short l)
{
	return _LittleShort(l);
}
int LittleLong(int l)
{
	return _LittleLong(l);
}

qint64 LittleLong64(qint64 l)
{
	return _LittleLong64(l);
}
float LittleFloat(float l)
{
	return _LittleFloat(l);
}

short BigShort(short l)
{
	return _BigShort(l);
}
int BigLong(int l)
{
	return _BigLong(l);
}

qint64 BigLong64(qint64 l)
{
	return _BigLong64(l);
}
float BigFloat(float l)
{
	return _BigFloat(l);
}

short ShortSwap(short l)
{
	byte            b1, b2;

	b1 = l & 255;
	b2 = (l >> 8) & 255;

	return (b1 << 8) + b2;
}

short ShortNoSwap(short l)
{
	return l;
}

int LongSwap(int l)
{
	byte            b1, b2, b3, b4;

	b1 = l & 255;
	b2 = (l >> 8) & 255;
	b3 = (l >> 16) & 255;
	b4 = (l >> 24) & 255;

	return ((int)b1 << 24) + ((int)b2 << 16) + ((int)b3 << 8) + b4;
}

int LongNoSwap(int l)
{
	return l;
}

qint64 Long64Swap(qint64 ll)
{
	qint64          result;

	result.b0 = ll.b7;
	result.b1 = ll.b6;
	result.b2 = ll.b5;
	result.b3 = ll.b4;
	result.b4 = ll.b3;
	result.b5 = ll.b2;
	result.b6 = ll.b1;
	result.b7 = ll.b0;

	return result;
}

qint64 Long64NoSwap(qint64 ll)
{
	return ll;
}

float FloatSwap(float f)
{
	union
	{
		float           f;
		byte            b[4];
	} dat1         , dat2;


	dat1.f = f;
	dat2.b[0] = dat1.b[3];
	dat2.b[1] = dat1.b[2];
	dat2.b[2] = dat1.b[1];
	dat2.b[3] = dat1.b[0];
	return dat2.f;
}

float FloatNoSwap(float f)
{
	return f;
}

/*
================
Swap_Init
================
*/
void Swap_Init(void)
{
	byte            swaptest[2] = { 1, 0 };

// set the byte swapping variables in a portable manner
	if(*(short *)swaptest == 1)
	{
		_BigShort = ShortSwap;
		_LittleShort = ShortNoSwap;
		_BigLong = LongSwap;
		_LittleLong = LongNoSwap;
		_BigLong64 = Long64Swap;
		_LittleLong64 = Long64NoSwap;
		_BigFloat = FloatSwap;
		_LittleFloat = FloatNoSwap;
	}
	else
	{
		_BigShort = ShortNoSwap;
		_LittleShort = ShortSwap;
		_BigLong = LongNoSwap;
		_LittleLong = LongSwap;
		_BigLong64 = Long64NoSwap;
		_LittleLong64 = Long64Swap;
		_BigFloat = FloatNoSwap;
		_LittleFloat = FloatSwap;
	}

}


/*
============================================================================

PARSING

============================================================================
*/

// multiple character punctuation tokens
const char     *punctuation[] = {
	"+=", "-=", "*=", "/=", "&=", "|=", "++", "--",
	"&&", "||", "<=", ">=", "==", "!=",
	NULL
};

static char     com_token[MAX_TOKEN_CHARS];
static char     com_parsename[MAX_TOKEN_CHARS];
static int      com_lines;

static int      backup_lines;
static char    *backup_text;

void COM_BeginParseSession(const char *name)
{
	com_lines = 0;
	Com_sprintf(com_parsename, sizeof(com_parsename), "%s", name);
}

void COM_BackupParseSession(char **data_p)
{
	backup_lines = com_lines;
	backup_text = *data_p;
}

void COM_RestoreParseSession(char **data_p)
{
	com_lines = backup_lines;
	*data_p = backup_text;
}

void COM_SetCurrentParseLine(int line)
{
	com_lines = line;
}

int COM_GetCurrentParseLine(void)
{
	return com_lines;
}

char           *COM_Parse(char **data_p)
{
	return COM_ParseExt(data_p, qtrue);
}

void COM_ParseError(char *format, ...)
{
	va_list         argptr;
	static char     string[4096];

	va_start(argptr, format);
	Q_vsnprintf(string, sizeof(string), format, argptr);
	va_end(argptr);

	Com_Printf(S_COLOR_RED "ERROR: %s, line %d: %s\n", com_parsename, com_lines, string);
}

void COM_ParseWarning(char *format, ...)
{
	va_list         argptr;
	static char     string[4096];

	va_start(argptr, format);
	Q_vsnprintf(string, sizeof(string), format, argptr);
	va_end(argptr);

	Com_Printf(S_COLOR_YELLOW "WARNING: %s, line %d: %s\n", com_parsename, com_lines, string);
}

/*
==============
COM_Parse

Parse a token out of a string
Will never return NULL, just empty strings

If "allowLineBreaks" is qtrue then an empty
string will be returned if the next token is
a newline.
==============
*/
static char    *SkipWhitespace(char *data, qboolean * hasNewLines)
{
	int             c;

	while((c = *data) <= ' ')
	{
		if(!c)
		{
			return NULL;
		}
		if(c == '\n')
		{
			com_lines++;
			*hasNewLines = qtrue;
		}
		data++;
	}

	return data;
}

int COM_Compress(char *data_p)
{
	char           *datai, *datao;
	int             c, pc, size;
	qboolean        ws = qfalse;

	size = 0;
	pc = 0;
	datai = datao = data_p;
	if(datai)
	{
		while((c = *datai) != 0)
		{
			if(c == 13 || c == 10)
			{
				*datao = c;
				datao++;
				ws = qfalse;
				pc = c;
				datai++;
				size++;
				// skip double slash comments
			}
			else if(c == '/' && datai[1] == '/')
			{
				while(*datai && *datai != '\n')
				{
					datai++;
				}
				ws = qfalse;
				// skip /* */ comments
			}
			else if(c == '/' && datai[1] == '*')
			{
				datai += 2;		// Arnout: skip over '/*'
				while(*datai && (*datai != '*' || datai[1] != '/'))
				{
					datai++;
				}
				if(*datai)
				{
					datai += 2;
				}
				ws = qfalse;
			}
			else
			{
				if(ws)
				{
					*datao = ' ';
					datao++;
				}
				*datao = c;
				datao++;
				datai++;
				ws = qfalse;
				pc = c;
				size++;
			}
		}
	}
	*datao = 0;
	return size;
}

char           *COM_ParseExt(char **data_p, qboolean allowLineBreaks)
{
	int             c = 0, len;
	qboolean        hasNewLines = qfalse;
	char           *data;

	data = *data_p;
	len = 0;
	com_token[0] = 0;

	// make sure incoming data is valid
	if(!data)
	{
		*data_p = NULL;
		return com_token;
	}

	// RF, backup the session data so we can unget easily
	COM_BackupParseSession(data_p);

	while(1)
	{
		// skip whitespace
		data = SkipWhitespace(data, &hasNewLines);
		if(!data)
		{
			*data_p = NULL;
			return com_token;
		}
		if(hasNewLines && !allowLineBreaks)
		{
			*data_p = data;
			return com_token;
		}

		c = *data;

		// skip double slash comments
		if(c == '/' && data[1] == '/')
		{
			data += 2;
			while(*data && *data != '\n')
			{
				data++;
			}
//          com_lines++;
		}
		// skip /* */ comments
		else if(c == '/' && data[1] == '*')
		{
			data += 2;
			while(*data && (*data != '*' || data[1] != '/'))
			{
				data++;
				if(*data == '\n')
				{
//                  com_lines++;
				}
			}
			if(*data)
			{
				data += 2;
			}
		}
		else
		{
			break;
		}
	}

	// handle quoted strings
	if(c == '\"')
	{
		data++;
		while(1)
		{
			c = *data++;
			if(c == '\\' && *(data) == '\"')
			{
				// Arnout: string-in-string
				if(len < MAX_TOKEN_CHARS)
				{
					com_token[len] = '\"';
					len++;
				}
				data++;

				while(1)
				{
					c = *data++;

					if(!c)
					{
						com_token[len] = 0;
						*data_p = (char *)data;
						break;
					}
					if((c == '\\' && *(data) == '\"'))
					{
						if(len < MAX_TOKEN_CHARS)
						{
							com_token[len] = '\"';
							len++;
						}
						data++;
						c = *data++;
						break;
					}
					if(len < MAX_TOKEN_CHARS)
					{
						com_token[len] = c;
						len++;
					}
				}
			}
			if(c == '\"' || !c)
			{
				com_token[len] = 0;
				*data_p = (char *)data;
				return com_token;
			}
			if(len < MAX_TOKEN_CHARS)
			{
				com_token[len] = c;
				len++;
			}
		}
	}

	// parse a regular word
	do
	{
		if(len < MAX_TOKEN_CHARS)
		{
			com_token[len] = c;
			len++;
		}
		data++;
		c = *data;
		if(c == '\n')
		{
			com_lines++;
		}
	} while(c > 32);

	if(len == MAX_TOKEN_CHARS)
	{
//      Com_Printf ("Token exceeded %i chars, discarded.\n", MAX_TOKEN_CHARS);
		len = 0;
	}
	com_token[len] = 0;

	*data_p = (char *)data;
	return com_token;
}



char           *COM_Parse2(char **data_p)
{
	return COM_ParseExt2(data_p, qtrue);
}


// *INDENT-OFF*
char           *COM_ParseExt2(char **data_p, qboolean allowLineBreaks)
{
	int             c = 0, len;
	qboolean        hasNewLines = qfalse;
	char           *data;
	const char    **punc;

	if(!data_p)
	{
		Com_Error(ERR_FATAL, "COM_ParseExt: NULL data_p");
	}

	data = *data_p;
	len = 0;
	com_token[0] = 0;

	// make sure incoming data is valid
	if(!data)
	{
		*data_p = NULL;
		return com_token;
	}

	// RF, backup the session data so we can unget easily
	COM_BackupParseSession(data_p);

	// skip whitespace
	while(1)
	{
		data = SkipWhitespace(data, &hasNewLines);
		if(!data)
		{
			*data_p = NULL;
			return com_token;
		}
		if(hasNewLines && !allowLineBreaks)
		{
			*data_p = data;
			return com_token;
		}

		c = *data;

		// skip double slash comments
		if(c == '/' && data[1] == '/')
		{
			data += 2;
			while(*data && *data != '\n')
			{
				data++;
			}
		}
		// skip /* */ comments
		else if(c == '/' && data[1] == '*')
		{
			data += 2;
			while(*data && (*data != '*' || data[1] != '/'))
			{
				data++;
			}
			if(*data)
			{
				data += 2;
			}
		}
		else
		{
			// a real token to parse
			break;
		}
	}

	// handle quoted strings
	if(c == '\"')
	{
		data++;
		while(1)
		{
			c = *data++;

			if((c == '\\') && (*data == '\"'))
			{
				// allow quoted strings to use \" to indicate the " character
				data++;
			}
			else if(c == '\"' || !c)
			{
				com_token[len] = 0;
				*data_p = (char *)data;
				return com_token;
			}
			else if(*data == '\n')
			{
				com_lines++;
			}

			if(len < MAX_TOKEN_CHARS - 1)
			{
				com_token[len] = c;
				len++;
			}
		}
	}

	// check for a number
	// is this parsing of negative numbers going to cause expression problems
	if(	(c >= '0' && c <= '9') ||
		(c == '-' && data[1] >= '0' && data[1] <= '9') ||
		(c == '.' && data[1] >= '0' && data[1] <= '9') ||
		(c == '-' && data[1] == '.' && data[2] >= '0' && data[2] <= '9'))
	{
		do
		{
			if(len < MAX_TOKEN_CHARS - 1)
			{
				com_token[len] = c;
				len++;
			}
			data++;

			c = *data;
		} while((c >= '0' && c <= '9') || c == '.');

		// parse the exponent
		if(c == 'e' || c == 'E')
		{
			if(len < MAX_TOKEN_CHARS - 1)
			{
				com_token[len] = c;
				len++;
			}
			data++;
			c = *data;

			if(c == '-' || c == '+')
			{
				if(len < MAX_TOKEN_CHARS - 1)
				{
					com_token[len] = c;
					len++;
				}
				data++;
				c = *data;
			}

			do
			{
				if(len < MAX_TOKEN_CHARS - 1)
				{
					com_token[len] = c;
					len++;
				}
				data++;

				c = *data;
			} while(c >= '0' && c <= '9');
		}

		if(len == MAX_TOKEN_CHARS)
		{
			len = 0;
		}
		com_token[len] = 0;

		*data_p = (char *)data;
		return com_token;
	}

	// check for a regular word
	// we still allow forward and back slashes in name tokens for pathnames
	// and also colons for drive letters
	if(	(c >= 'a' && c <= 'z') ||
		(c >= 'A' && c <= 'Z') ||
		(c == '_') ||
		(c == '/') ||
		(c == '\\') ||
		(c == '$') || (c == '*')) // Tr3B - for bad shader strings
	{
		do
		{
			if(len < MAX_TOKEN_CHARS - 1)
			{
				com_token[len] = c;
				len++;
			}
			data++;

			c = *data;
		}
		while
			((c >= 'a' && c <= 'z') ||
			 (c >= 'A' && c <= 'Z') ||
			 (c == '_') ||
			 (c == '-') ||
			 (c >= '0' && c <= '9') ||
			 (c == '/') ||
			 (c == '\\') ||
			 (c == ':') ||
			 (c == '.') ||
			 (c == '$') ||
			 (c == '*') ||
			 (c == '@'));

		if(len == MAX_TOKEN_CHARS)
		{
			len = 0;
		}
		com_token[len] = 0;

		*data_p = (char *)data;
		return com_token;
	}

	// check for multi-character punctuation token
	for(punc = punctuation; *punc; punc++)
	{
		int             l;
		int             j;

		l = strlen(*punc);
		for(j = 0; j < l; j++)
		{
			if(data[j] != (*punc)[j])
			{
				break;
			}
		}
		if(j == l)
		{
			// a valid multi-character punctuation
			Com_Memcpy(com_token, *punc, l);
			com_token[l] = 0;
			data += l;
			*data_p = (char *)data;
			return com_token;
		}
	}

	// single character punctuation
	com_token[0] = *data;
	com_token[1] = 0;
	data++;
	*data_p = (char *)data;

	return com_token;
}
// *INDENT-ON*



/*
==================
COM_MatchToken
==================
*/
void COM_MatchToken(char **buf_p, char *match)
{
	char           *token;

	token = COM_Parse(buf_p);
	if(strcmp(token, match))
	{
		Com_Error(ERR_DROP, "MatchToken: %s != %s", token, match);
	}
}

/*
=================
SkipBracedSection_Depth

=================
*/
void SkipBracedSection_Depth(char **program, int depth)
{
	char           *token;

	do
	{
		token = COM_ParseExt(program, qtrue);
		if(token[1] == 0)
		{
			if(token[0] == '{')
			{
				depth++;
			}
			else if(token[0] == '}')
			{
				depth--;
			}
		}
	} while(depth && *program);
}

/*
=================
SkipBracedSection

The next token should be an open brace.
Skips until a matching close brace is found.
Internal brace depths are properly skipped.
=================
*/
void SkipBracedSection(char **program)
{
	char           *token;
	int             depth;

	depth = 0;
	do
	{
		token = COM_ParseExt(program, qtrue);
		if(token[1] == 0)
		{
			if(token[0] == '{')
			{
				depth++;
			}
			else if(token[0] == '}')
			{
				depth--;
			}
		}
	} while(depth && *program);
}

/*
=================
SkipRestOfLine
=================
*/
void SkipRestOfLine(char **data)
{
	char           *p;
	int             c;

	p = *data;
	while((c = *p++) != 0)
	{
		if(c == '\n')
		{
			com_lines++;
			break;
		}
	}

	*data = p;
}


void Parse1DMatrix(char **buf_p, int x, float *m)
{
	char           *token;
	int             i;

	COM_MatchToken(buf_p, "(");

	for(i = 0; i < x; i++)
	{
		token = COM_Parse(buf_p);
		m[i] = atof(token);
	}

	COM_MatchToken(buf_p, ")");
}

void Parse2DMatrix(char **buf_p, int y, int x, float *m)
{
	int             i;

	COM_MatchToken(buf_p, "(");

	for(i = 0; i < y; i++)
	{
		Parse1DMatrix(buf_p, x, m + i * x);
	}

	COM_MatchToken(buf_p, ")");
}

void Parse3DMatrix(char **buf_p, int z, int y, int x, float *m)
{
	int             i;

	COM_MatchToken(buf_p, "(");

	for(i = 0; i < z; i++)
	{
		Parse2DMatrix(buf_p, y, x, m + i * x * y);
	}

	COM_MatchToken(buf_p, ")");
}


/*
===============
Com_ParseInfos
===============
*/
int Com_ParseInfos(char *buf, int max, char infos[][MAX_INFO_STRING])
{
	const char     *token;
	int             count;
	char            key[MAX_TOKEN_CHARS];

	count = 0;

	while(1)
	{
		token = COM_Parse(&buf);
		if(!token[0])
		{
			break;
		}
		if(strcmp(token, "{"))
		{
			Com_Printf("Missing { in info file\n");
			break;
		}

		if(count == max)
		{
			Com_Printf("Max infos exceeded\n");
			break;
		}

		infos[count][0] = 0;
		while(1)
		{
			token = COM_Parse(&buf);
			if(!token[0])
			{
				Com_Printf("Unexpected end of info file\n");
				break;
			}
			if(!strcmp(token, "}"))
			{
				break;
			}
			Q_strncpyz(key, token, sizeof(key));

			token = COM_ParseExt(&buf, qfalse);
			if(!token[0])
			{
				token = "<NULL>";
			}
			Info_SetValueForKey(infos[count], key, token);
		}
		count++;
	}

	return count;
}

/*
============================================================================

					LIBRARY REPLACEMENT FUNCTIONS

============================================================================
*/

int Q_isprint(int c)
{
	if(c >= 0x20 && c <= 0x7E)
	{
		return (1);
	}
	return (0);
}

int Q_islower(int c)
{
	if(c >= 'a' && c <= 'z')
	{
		return (1);
	}
	return (0);
}

int Q_isupper(int c)
{
	if(c >= 'A' && c <= 'Z')
	{
		return (1);
	}
	return (0);
}

int Q_isalpha(int c)
{
	if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
	{
		return (1);
	}
	return (0);
}

int Q_isnumeric(int c)
{
	if(c >= '0' && c <= '9')
	{
		return (1);
	}
	return (0);
}

int Q_isalphanumeric(int c)
{
	if(Q_isalpha(c) || Q_isnumeric(c))
	{
		return (1);
	}
	return (0);
}

int Q_isforfilename(int c)
{
	if((Q_isalphanumeric(c) || c == '_') && c != ' ')
	{							// space not allowed in filename
		return (1);
	}
	return (0);
}

char           *Q_strrchr(const char *string, int c)
{
	char            cc = c;
	char           *s;
	char           *sp = (char *)0;

	s = (char *)string;

	while(*s)
	{
		if(*s == cc)
		{
			sp = s;
		}
		s++;
	}
	if(cc == 0)
	{
		sp = s;
	}

	return sp;
}

/*
=============
Q_stristr

Find the first occurrence of find in s.
=============
*/
char           *Q_stristr(const char *s, const char *find)
{
	char            c, sc;
	size_t          len;

	if((c = *find++) != 0)
	{
		if(c >= 'a' && c <= 'z')
		{
			c -= ('a' - 'A');
		}
		len = strlen(find);
		do
		{
			do
			{
				if((sc = *s++) == 0)
				{
					return NULL;
				}

				if(sc >= 'a' && sc <= 'z')
				{
					sc -= ('a' - 'A');
				}
			} while(sc != c);
		} while(Q_stricmpn(s, find, len) != 0);
		s--;
	}
	return (char *)s;
}

/*
=============
Q_strncpyz

Safe strncpy that ensures a trailing zero
=============
*/
void Q_strncpyz(char *dest, const char *src, int destsize)
{
	if(!src)
	{
		Com_Error(ERR_FATAL, "Q_strncpyz: NULL src");
	}
	if(destsize < 1)
	{
		Com_Error(ERR_FATAL, "Q_strncpyz: destsize < 1");
	}

	strncpy(dest, src, destsize - 1);
	dest[destsize - 1] = 0;
}

int Q_stricmpn(const char *s1, const char *s2, int n)
{
	int             c1, c2;

	do
	{
		c1 = *s1++;
		c2 = *s2++;

		if(!n--)
		{
			return 0;			// strings are equal until end point
		}

		if(c1 != c2)
		{
			if(c1 >= 'a' && c1 <= 'z')
			{
				c1 -= ('a' - 'A');
			}
			if(c2 >= 'a' && c2 <= 'z')
			{
				c2 -= ('a' - 'A');
			}
			if(c1 != c2)
			{
				return c1 < c2 ? -1 : 1;
			}
		}
	} while(c1);

	return 0;					// strings are equal
}

int Q_strncmp(const char *s1, const char *s2, int n)
{
	int             c1, c2;

	do
	{
		c1 = *s1++;
		c2 = *s2++;

		if(!n--)
		{
			return 0;			// strings are equal until end point
		}

		if(c1 != c2)
		{
			return c1 < c2 ? -1 : 1;
		}
	} while(c1);

	return 0;					// strings are equal
}

int Q_stricmp(const char *s1, const char *s2)
{
	return (s1 && s2) ? Q_stricmpn(s1, s2, 99999) : -1;
}

char           *Q_strlwr(char *s1)
{
	char           *s;

	for(s = s1; *s; ++s)
	{
		if(('A' <= *s) && (*s <= 'Z'))
		{
			*s -= 'A' - 'a';
		}
	}

	return s1;
}

char           *Q_strupr(char *s1)
{
	char           *cp;

	for(cp = s1; *cp; ++cp)
	{
		if(('a' <= *cp) && (*cp <= 'z'))
		{
			*cp += 'A' - 'a';
		}
	}

	return s1;
}


// never goes past bounds or leaves without a terminating 0
void Q_strcat(char *dest, int size, const char *src)
{
	int             l1;

	l1 = strlen(dest);
	if(l1 >= size)
	{
		Com_Error(ERR_FATAL, "Q_strcat: already overflowed");
	}
	Q_strncpyz(dest + l1, src, size - l1);
}

/*
=============
Q_strreplace

replaces content of find by replace in dest
=============
*/
qboolean Q_strreplace(char *dest, int destsize, const char *find, const char *replace)
{
	int             lstart, lfind, lreplace, lend;
	char           *s;
	char            backup[32000];	// big, but small enough to fit in PPC stack

	lend = strlen(dest);
	if(lend >= destsize)
	{
		Com_Error(ERR_FATAL, "Q_strreplace: already overflowed");
	}

	s = strstr(dest, find);
	if(!s)
	{
		return qfalse;
	}
	else
	{
		Q_strncpyz(backup, dest, lend + 1);
		lstart = s - dest;
		lfind = strlen(find);
		lreplace = strlen(replace);

		strncpy(s, replace, destsize - lstart - 1);
		strncpy(s + lreplace, backup + lstart + lfind, destsize - lstart - lreplace - 1);

		return qtrue;
	}
}

int Q_PrintStrlen(const char *string)
{
	int             len;
	const char     *p;

	if(!string)
	{
		return 0;
	}

	len = 0;
	p = string;
	while(*p)
	{
		if(Q_IsColorString(p))
		{
			p += 2;
			continue;
		}
		p++;
		len++;
	}

	return len;
}


char           *Q_CleanStr(char *string)
{
	char           *d;
	char           *s;
	int             c;

	s = string;
	d = string;
	while((c = *s) != 0)
	{
		if(Q_IsColorString(s))
		{
			s++;
		}
		else if(c >= 0x20 && c <= 0x7E)
		{
			*d++ = c;
		}
		s++;
	}
	*d = '\0';

	return string;
}

// strips whitespaces and bad characters
qboolean Q_isBadDirChar(char c)
{
	char            badchars[] =
		{ ';', '&', '(', ')', '|', '<', '>', '*', '?', '[', ']', '~', '+', '@', '!', '\\', '/', ' ', '\'', '\"', '\0' };
	int             i;

	for(i = 0; badchars[i] != '\0'; i++)
	{
		if(c == badchars[i])
		{
			return qtrue;
		}
	}

	return qfalse;
}

char           *Q_CleanDirName(char *dirname)
{
	char           *d;
	char           *s;

	s = dirname;
	d = dirname;

	// clear trailing .'s
	while(*s == '.')
	{
		s++;
	}

	while(*s != '\0')
	{
		if(!Q_isBadDirChar(*s))
		{
			*d++ = *s;
		}
		s++;
	}
	*d = '\0';

	return dirname;
}

/*
============
Q_vsnprintf

vsnprintf portability:

C99 standard: vsnprintf returns the number of characters (excluding the trailing
'\0') which would have been written to the final string if enough space had been available
snprintf and vsnprintf do not write more than size bytes (including the trailing '\0')

win32: _vsnprintf returns the number of characters written, not including the terminating null character,
or a negative value if an output error occurs. If the number of characters to write exceeds count,
then count characters are written and -1 is returned and no trailing '\0' is added.

Q_vsnPrintf: always append a trailing '\0', returns number of characters written or
returns -1 on failure or if the buffer would be overflowed.

copied over from common.c implementation
============
*/
int Q_vsnprintf(char *dest, int size, const char *fmt, va_list argptr)
{
	int             ret;

#ifdef _WIN32
#undef _vsnprintf
	ret = _vsnprintf(dest, size - 1, fmt, argptr);
#define _vsnprintf  use_idStr_vsnPrintf
#else
#undef vsnprintf
	ret = vsnprintf(dest, size, fmt, argptr);
#define vsnprintf   use_idStr_vsnPrintf
#endif
	dest[size - 1] = '\0';
	if(ret < 0 || ret >= size)
	{
		return -1;
	}
	return ret;
}

void QDECL Com_sprintf(char *dest, int size, const char *fmt, ...)
{
	int             ret;
	va_list         argptr;

	va_start(argptr, fmt);
	ret = Q_vsnprintf(dest, size, fmt, argptr);
	va_end(argptr);
	if(ret == -1)
	{
		Com_Printf("Com_sprintf: overflow of %i bytes buffer\n", size);
	}
}

/*
============
va

does a varargs printf into a temp buffer, so I don't need to have
varargs versions of all text functions.
FIXME: make this buffer size safe someday

Ridah, modified this into a circular list, to further prevent stepping on
previous strings
============
*/
char           *QDECL va(char *format, ...)
{
	va_list         argptr;

#define MAX_VA_STRING   32000
	static char     temp_buffer[MAX_VA_STRING];
	static char     string[MAX_VA_STRING];	// in case va is called by nested functions
	static int      index = 0;
	char           *buf;
	int             len;


	va_start(argptr, format);
	vsprintf(temp_buffer, format, argptr);
	va_end(argptr);

	if((len = strlen(temp_buffer)) >= MAX_VA_STRING)
	{
		Com_Error(ERR_DROP, "Attempted to overrun string in call to va()\n");
	}

	if(len + index >= MAX_VA_STRING - 1)
	{
		index = 0;
	}

	buf = &string[index];
	memcpy(buf, temp_buffer, len + 1);

	index += len + 1;

	return buf;
}

/*
=============
TempVector

(SA) this is straight out of g_utils.c around line 210

This is just a convenience function
for making temporary vectors for function calls
=============
*/
float          *tv(float x, float y, float z)
{
	static int      index;
	static vec3_t   vecs[8];
	float          *v;

	// use an array so that multiple tempvectors won't collide
	// for a while
	v = vecs[index];
	index = (index + 1) & 7;

	v[0] = x;
	v[1] = y;
	v[2] = z;

	return v;
}

/*
=====================================================================

  INFO STRINGS

=====================================================================
*/

/*
===============
Info_ValueForKey

Searches the string for the given
key and returns the associated value, or an empty string.
FIXME: overflow check?
===============
*/
char           *Info_ValueForKey(const char *s, const char *key)
{
	char            pkey[BIG_INFO_KEY];
	static char     value[2][BIG_INFO_VALUE];	// use two buffers so compares

	// work without stomping on each other
	static int      valueindex = 0;
	char           *o;

	if(!s || !key)
	{
		return "";
	}

	if(strlen(s) >= BIG_INFO_STRING)
	{
		Com_Error(ERR_DROP, "Info_ValueForKey: oversize infostring [%s] [%s]", s, key);
	}

	valueindex ^= 1;
	if(*s == '\\')
	{
		s++;
	}
	while(1)
	{
		o = pkey;
		while(*s != '\\')
		{
			if(!*s)
			{
				return "";
			}
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value[valueindex];

		while(*s != '\\' && *s)
		{
			*o++ = *s++;
		}
		*o = 0;

		if(!Q_stricmp(key, pkey))
		{
			return value[valueindex];
		}

		if(!*s)
		{
			break;
		}
		s++;
	}

	return "";
}


/*
===================
Info_NextPair

Used to itterate through all the key/value pairs in an info string
===================
*/
void Info_NextPair(const char **head, char *key, char *value)
{
	char           *o;
	const char     *s;

	s = *head;

	if(*s == '\\')
	{
		s++;
	}
	key[0] = 0;
	value[0] = 0;

	o = key;
	while(*s != '\\')
	{
		if(!*s)
		{
			*o = 0;
			*head = s;
			return;
		}
		*o++ = *s++;
	}
	*o = 0;
	s++;

	o = value;
	while(*s != '\\' && *s)
	{
		*o++ = *s++;
	}
	*o = 0;

	*head = s;
}


/*
===================
Info_RemoveKey
===================
*/
void Info_RemoveKey(char *s, const char *key)
{
	char           *start;
	char            pkey[MAX_INFO_KEY];
	char            value[MAX_INFO_VALUE];
	char           *o;

	if(strlen(s) >= MAX_INFO_STRING)
	{
		Com_Error(ERR_DROP, "Info_RemoveKey: oversize infostring [%s] [%s]", s, key);
	}

	if(strchr(key, '\\'))
	{
		return;
	}

	while(1)
	{
		start = s;
		if(*s == '\\')
		{
			s++;
		}
		o = pkey;
		while(*s != '\\')
		{
			if(!*s)
			{
				return;
			}
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value;
		while(*s != '\\' && *s)
		{
			if(!*s)
			{
				return;
			}
			*o++ = *s++;
		}
		*o = 0;

		if(!Q_stricmp(key, pkey))
		{
			// rain - arguments to strcpy must not overlap
			//strcpy (start, s);    // remove this part
			memmove(start, s, strlen(s) + 1);	// remove this part
			return;
		}

		if(!*s)
		{
			return;
		}
	}

}

/*
===================
Info_RemoveKey_Big
===================
*/
void Info_RemoveKey_Big(char *s, const char *key)
{
	char           *start;
	char            pkey[BIG_INFO_KEY];
	char            value[BIG_INFO_VALUE];
	char           *o;

	if(strlen(s) >= BIG_INFO_STRING)
	{
		Com_Error(ERR_DROP, "Info_RemoveKey_Big: oversize infostring [%s] [%s]", s, key);
	}

	if(strchr(key, '\\'))
	{
		return;
	}

	while(1)
	{
		start = s;
		if(*s == '\\')
		{
			s++;
		}
		o = pkey;
		while(*s != '\\')
		{
			if(!*s)
			{
				return;
			}
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value;
		while(*s != '\\' && *s)
		{
			if(!*s)
			{
				return;
			}
			*o++ = *s++;
		}
		*o = 0;

		if(!Q_stricmp(key, pkey))
		{
			strcpy(start, s);	// remove this part
			return;
		}

		if(!*s)
		{
			return;
		}
	}

}




/*
==================
Info_Validate

Some characters are illegal in info strings because they
can mess up the server's parsing
==================
*/
qboolean Info_Validate(const char *s)
{
	if(strchr(s, '\"'))
	{
		return qfalse;
	}
	if(strchr(s, ';'))
	{
		return qfalse;
	}
	return qtrue;
}

/*
==================
Info_SetValueForKey

Changes or adds a key/value pair
==================
*/
void Info_SetValueForKey(char *s, const char *key, const char *value)
{
	char            newi[MAX_INFO_STRING];

	if(strlen(s) >= MAX_INFO_STRING)
	{
		Com_Error(ERR_DROP, "Info_SetValueForKey: oversize infostring [%s] [%s] [%s]", s, key, value);
	}

	if(strchr(key, '\\') || strchr(value, '\\'))
	{
		Com_Printf("Can't use keys or values with a \\\n");
		return;
	}

	if(strchr(key, ';') || strchr(value, ';'))
	{
		Com_Printf("Can't use keys or values with a semicolon\n");
		return;
	}

	if(strchr(key, '\"') || strchr(value, '\"'))
	{
		Com_Printf("Can't use keys or values with a \"\n");
		return;
	}

	Info_RemoveKey(s, key);
	if(!value || !strlen(value))
	{
		return;
	}

	Com_sprintf(newi, sizeof(newi), "\\%s\\%s", key, value);

	if(strlen(newi) + strlen(s) > MAX_INFO_STRING)
	{
		Com_Printf("Info string length exceeded\n");
		return;
	}

	strcat(s, newi);
}

/*
==================
Info_SetValueForKey_Big

Changes or adds a key/value pair
==================
*/
void Info_SetValueForKey_Big(char *s, const char *key, const char *value)
{
	char            newi[BIG_INFO_STRING];

	if(strlen(s) >= BIG_INFO_STRING)
	{
		Com_Error(ERR_DROP, "Info_SetValueForKey: oversize infostring [%s] [%s] [%s]", s, key, value);
	}

	if(strchr(key, '\\') || strchr(value, '\\'))
	{
		Com_Printf("Can't use keys or values with a \\\n");
		return;
	}

	if(strchr(key, ';') || strchr(value, ';'))
	{
		Com_Printf("Can't use keys or values with a semicolon\n");
		return;
	}

	if(strchr(key, '\"') || strchr(value, '\"'))
	{
		Com_Printf("Can't use keys or values with a \"\n");
		return;
	}

	Info_RemoveKey_Big(s, key);
	if(!value || !strlen(value))
	{
		return;
	}

	Com_sprintf(newi, sizeof(newi), "\\%s\\%s", key, value);

	if(strlen(newi) + strlen(s) > BIG_INFO_STRING)
	{
		Com_Printf("BIG Info string length exceeded\n");
		return;
	}

	strcat(s, newi);
}




//====================================================================


/*
============================================================================

OPTIMIZED memory functions moved from common.c

============================================================================
*/


//#if !( defined __linux__ || defined __FreeBSD__ )	// r010123 - include FreeBSD
//#if defined(__x86_64__) || ( ( !id386 ) && ( !defined __i386__ ) || __GNUC__)	// rcg010212 - for PPC

void Com_Memcpy(void *dest, const void *src, const size_t count)
{
	memcpy(dest, src, count);
}

void Com_Memset(void *dest, const int val, const size_t count)
{
	memset(dest, val, count);
}

/*
#else

typedef enum
{
	PRE_READ,					// prefetch assuming that buffer is used for reading only
	PRE_WRITE,					// prefetch assuming that buffer is used for writing only
	PRE_READ_WRITE				// prefetch assuming that buffer is used for both reading and writing
} e_prefetch;

void            Com_Prefetch(const void *s, const unsigned int bytes, e_prefetch type);

// *INDENT-OFF*
#define EMMS_INSTRUCTION    __asm emms

void _copyDWord( unsigned int* dest, const unsigned int constant, const unsigned int count ) {
	__asm
	{
		mov edx,dest
		mov eax,constant
		mov ecx,count
		and     ecx,~7
		jz padding
		sub ecx,8
		jmp loopu
		align   16
loopu:
		test    [edx + ecx * 4 + 28],ebx        // fetch next block destination to L1 cache
		mov     [edx + ecx * 4 + 0],eax
		mov     [edx + ecx * 4 + 4],eax
		mov     [edx + ecx * 4 + 8],eax
		mov     [edx + ecx * 4 + 12],eax
		mov     [edx + ecx * 4 + 16],eax
		mov     [edx + ecx * 4 + 20],eax
		mov     [edx + ecx * 4 + 24],eax
		mov     [edx + ecx * 4 + 28],eax
		sub ecx,8
		jge loopu
padding:    mov ecx,count
		mov ebx,ecx
		and     ecx,7
		jz outta
		and     ebx,~7
		lea edx,[edx + ebx * 4]                 // advance dest pointer
		test    [edx + 0],eax                   // fetch destination to L1 cache
		cmp ecx,4
		jl skip4
		mov     [edx + 0],eax
		mov     [edx + 4],eax
		mov     [edx + 8],eax
		mov     [edx + 12],eax
		add edx,16
		sub ecx,4
skip4:      cmp ecx,2
		jl skip2
		mov     [edx + 0],eax
		mov     [edx + 4],eax
		add edx,8
		sub ecx,2
skip2:      cmp ecx,1
		jl outta
		mov     [edx + 0],eax
outta:
	}
}

// optimized memory copy routine that handles all alignment
// cases and block sizes efficiently
void Com_Memcpy( void* dest, const void* src, const size_t count ) {
	Com_Prefetch( src, count, PRE_READ );
	__asm
	{
		push edi
		push esi
		mov ecx,count
		cmp ecx,0                           // count = 0 check (just to be on the safe side)
		je outta
		mov edx,dest
		mov ebx,src
		cmp ecx,32                          // padding only?
		jl padding

		mov edi,ecx
		and     edi,~31                 // edi = count&~31
		sub edi,32

		align 16
loopMisAligned:
		mov eax,[ebx + edi + 0 + 0 * 8]
		mov esi,[ebx + edi + 4 + 0 * 8]
		mov     [edx + edi + 0 + 0 * 8],eax
		mov     [edx + edi + 4 + 0 * 8],esi
		mov eax,[ebx + edi + 0 + 1 * 8]
		mov esi,[ebx + edi + 4 + 1 * 8]
		mov     [edx + edi + 0 + 1 * 8],eax
		mov     [edx + edi + 4 + 1 * 8],esi
		mov eax,[ebx + edi + 0 + 2 * 8]
		mov esi,[ebx + edi + 4 + 2 * 8]
		mov     [edx + edi + 0 + 2 * 8],eax
		mov     [edx + edi + 4 + 2 * 8],esi
		mov eax,[ebx + edi + 0 + 3 * 8]
		mov esi,[ebx + edi + 4 + 3 * 8]
		mov     [edx + edi + 0 + 3 * 8],eax
		mov     [edx + edi + 4 + 3 * 8],esi
		sub edi,32
		jge loopMisAligned

		mov edi,ecx
		and     edi,~31
		add ebx,edi                     // increase src pointer
		add edx,edi                     // increase dst pointer
		and     ecx,31                  // new count
		jz outta                        // if count = 0, get outta here

padding:
		cmp ecx,16
		jl skip16
		mov eax,dword ptr [ebx]
		mov dword ptr [edx],eax
		mov eax,dword ptr [ebx + 4]
		mov dword ptr [edx + 4],eax
		mov eax,dword ptr [ebx + 8]
		mov dword ptr [edx + 8],eax
		mov eax,dword ptr [ebx + 12]
		mov dword ptr [edx + 12],eax
		sub ecx,16
		add ebx,16
		add edx,16
skip16:
		cmp ecx,8
		jl skip8
		mov eax,dword ptr [ebx]
		mov dword ptr [edx],eax
		mov eax,dword ptr [ebx + 4]
		sub ecx,8
		mov dword ptr [edx + 4],eax
		add ebx,8
		add edx,8
skip8:
		cmp ecx,4
		jl skip4
		mov eax,dword ptr [ebx]     // here 4-7 bytes
		add ebx,4
		sub ecx,4
		mov dword ptr [edx],eax
		add edx,4
skip4:                          // 0-3 remaining bytes
		cmp ecx,2
		jl skip2
		mov ax,word ptr [ebx]       // two bytes
		cmp ecx,3                   // less than 3?
		mov word ptr [edx],ax
		jl outta
		mov al,byte ptr [ebx + 2]   // last byte
		mov byte ptr [edx + 2],al
		jmp outta
skip2:
		cmp ecx,1
		jl outta
		mov al,byte ptr [ebx]
		mov byte ptr [edx],al
outta:
		pop esi
		pop edi
	}
}

void Com_Memset( void* dest, const int val, const size_t count ) {
	unsigned int fillval;

	if ( count < 8 ) {
		__asm
		{
			mov edx,dest
			mov eax, val
			mov ah,al
			mov ebx,eax
			and     ebx, 0xffff
			shl eax,16
			add eax,ebx                 // eax now contains pattern
			mov ecx,count
			cmp ecx,4
			jl skip4
			mov     [edx],eax           // copy first dword
			add edx,4
			sub ecx,4
skip4:  cmp ecx,2
			jl skip2
			mov word ptr [edx],ax       // copy 2 bytes
			add edx,2
			sub ecx,2
skip2:  cmp ecx,0
			je skip1
			mov byte ptr [edx],al       // copy single byte
skip1:
		}
		return;
	}

	fillval = val;

	fillval = fillval | ( fillval << 8 );
	fillval = fillval | ( fillval << 16 );        // fill dword with 8-bit pattern

	_copyDWord( (unsigned int*)( dest ),fillval, count / 4 );

	__asm                                   // padding of 0-3 bytes
	{
		mov ecx,count
		mov eax,ecx
		and     ecx,3
		jz skipA
		and     eax,~3
		mov ebx,dest
		add ebx,eax
		mov eax,fillval
		cmp ecx,2
		jl skipB
		mov word ptr [ebx],ax
		cmp ecx,2
		je skipA
		mov byte ptr [ebx + 2],al
		jmp skipA
skipB:
		cmp ecx,0
		je skipA
		mov byte ptr [ebx],al
skipA:
	}
}
// *INDENT-ON*

qboolean Com_Memcmp(const void *src0, const void *src1, const unsigned int count)
{
	unsigned int    i;

	// MMX version anyone?

	if(count >= 16)
	{
		unsigned int   *dw = (unsigned int *)(src0);
		unsigned int   *sw = (unsigned int *)(src1);

		unsigned int    nm2 = count / 16;

		for(i = 0; i < nm2; i += 4)
		{
			unsigned int    tmp = (dw[i + 0] - sw[i + 0]) | (dw[i + 1] - sw[i + 1]) |
				(dw[i + 2] - sw[i + 2]) | (dw[i + 3] - sw[i + 3]);
			if(tmp)
			{
				return qfalse;
			}
		}
	}
	if(count & 15)
	{
		byte           *d = (byte *) src0;
		byte           *s = (byte *) src1;

		for(i = count & 0xfffffff0; i < count; i++)
			if(d[i] != s[i])
			{
				return qfalse;
			}
	}

	return qtrue;
}


// *INDENT-OFF*
void Com_Prefetch( const void *s, const unsigned int bytes, e_prefetch type ) {
	// write buffer prefetching is performed only if
	// the processor benefits from it. Read and read/write
	// prefetching is always performed.

	switch ( type )
	{
	case PRE_WRITE: break;
	case PRE_READ:
	case PRE_READ_WRITE:

		__asm
		{
			mov ebx,s
			mov ecx,bytes
			cmp ecx,4096                    // clamp to 4kB
			jle skipClamp
			mov ecx,4096
skipClamp:
			add ecx,0x1f
			shr ecx,5                       // number of cache lines
			jz skip
			jmp loopie

			align 16
loopie: test byte ptr [ebx],al
			add ebx,32
			dec ecx
			jnz loopie
skip:
		}

		break;
	}
}
// *INDENT-ON*

#endif
#endif							// bk001208 - memset/memcpy assembly, Q_acos needed (RC4)
*/
