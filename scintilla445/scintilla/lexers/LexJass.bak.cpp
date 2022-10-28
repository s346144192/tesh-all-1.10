
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <ctype.h>

#include <string>


#include "ILexer.h"
#include "Scintilla.h"
#include "SciLexer.h"

#include "StringCopy.h"
#include "WordList.h"
#include "LexAccessor.h"
#include "Accessor.h"
#include "StyleContext.h"
#include "CharacterSet.h"
#include "LexerModule.h"



using namespace Scintilla;

void set_or_byte_data(int* src, int b, int st, int length) {
	*src = *src | ((b % (1 << length)) << st);
}
int get_byte_data(int src, int st, int length) {
	return (src << (32 - st - length)) >> (32 - length);
}
static void strtolower(char str[]) {
	for (; *str;++str) {
		if (*str >= 65 && *str <= 90) {
			*str += 32;
		}
	}
}

// Test for [=[ ... ]=] delimiters, returns 0 if it's only a [ or ],
// return 1 for [[ or ]], returns >=2 for [=[ or ]=] and so on.
// The maximum number of '=' characters allowed is 254.
static int LongDelimCheck(StyleContext &sc) {
	int sep = 1;
	while (sc.GetRelative(sep) == '=' && sep < 0xFF)
		sep++;
	if (sc.GetRelative(sep) == sc.ch)
		return sep;
	return 0;
}

static bool check_lua_import(Accessor &styler,  Sci_Position i) {
	bool skip = false;
	char brace = 0;
	for(char ch; ch = styler.SafeGetCharAt(i, 0);--i)
	{
		if (!skip) {
			if (!brace) {
				if (styler.SafeGetCharAt(i - 1, 0) != '\\' && (ch == '\"' || ch == '\'')) {
					--i; brace = ch;
				}
				else {	
					if (!strchr("-[\n\r\t\v ",ch)) {
						return false;
					}
				}
			}
			else {
				if (styler.SafeGetCharAt(i - 1, 0) != '\\' && ch == brace) {
					--i; skip = true;
				}
				else {
					if (ch == '\n' || ch == '\r' ) {
						return false;
					}
				}
			}
		}
		else {
			if (ch=='t') {
				if (!styler.Match(i - 5, "import"))
				{
					return false;
				}
				ch = styler.SafeGetCharAt(i - 6, 0);
				if (ch =='_' 
					|| (ch >= 'a' && ch<='z')
					|| (ch >= 'A' && ch <= 'Z')
					|| (ch >= '0' && ch <= '9')) {
					return false;
				}
				return true;
			}
			else if (ch !='\t' && ch != ' ') 
			 {
				return false;
			}
		}
		
	}
	return false;
}
static bool check_s_hex(Accessor &styler, Sci_Position i, CharacterSet& setNumber) {
	if (styler.SafeGetCharAt(i, 0)!='$') {
		return false;
	}
	++i;
	for (char ch; ch = styler.SafeGetCharAt(i, 0); ++i)
	{
		if (strchr("\n\r\t ", ch)) {
			return true;
		}
		else if (!setNumber.Contains(ch)) {
			ch = toupper(ch);
			return (ch != '$' && ch!='_' && !(ch>='G' && ch<='Z'));
		}
	}
	return false;
}
static int GetNextVisibleChar(StyleContext &sc, Sci_Position* pos) {
	if (!pos)
		return 0;

	int c = 0;
	c = sc.GetRelative(*pos);
	while (c != 0 && isspace(c)) {
		*pos = *pos + 1;
		c = sc.GetRelative(*pos);
	}
	return c;
}

static void ColouriseJassLuaDoc(
	StyleContext &sc,
	int initStyle,
	WordList *keywordlists[],
	Accessor &styler) {

	const WordList &keywords = *keywordlists[1];
	//const WordList &keywords2 = *keywordlists[2];
	//const WordList &keywords3 = *keywordlists[3];
	//const WordList &keywords4 = *keywordlists[4];
	const WordList &keywords5 = *keywordlists[5];
	const WordList &keywords6 = *keywordlists[6];
	const WordList &keywords7 = *keywordlists[7];

	// Accepts accented characters
	CharacterSet setWordStart(CharacterSet::setAlpha, "_", 0x80, true);
	CharacterSet setWord(CharacterSet::setAlphaNum, "_", 0x80, true);
	// Not exactly following number definition (several dots are seen as OK, etc.)
	// but probably enough in most cases. [pP] is for hex floats.
	CharacterSet setNumber(CharacterSet::setDigits, ".-+abcdefpABCDEFP");
	CharacterSet setExponent(CharacterSet::setNone, "eEpP");
	CharacterSet setLuaOperator(CharacterSet::setNone, "*/-+()={}~[];<>,.^%:#&|?");
	CharacterSet setLuaNameRight(CharacterSet::setNone, "*/-+=~[]<>.^%:&|;,)");
	CharacterSet setEscapeSkip(CharacterSet::setNone, "\"'\\");

	Sci_Position currentLine = styler.GetLine(sc.currentPos);;
	// Initialize long string [[ ... ]] or block comment --[[ ... ]] nesting level,
	// if we are inside such a string. Block comment was introduced in Lua 5.0,
	// blocks with separators [=[ ... ]=] in Lua 5.1.
	// Continuation of a string (\z whitespace escaping) is controlled by stringWs.
	int nestLevel = 0;
	int sepCount = 0;
	int stringWs = 0;
	int C1sepCount = 0;
	{
		const int lineState = styler.GetLineState(currentLine - 1);
		C1sepCount = (lineState >> 9) & 0xFF;
		if (initStyle == SCE_LUA_LITERALSTRING || initStyle == SCE_LUA_COMMENT ||
			initStyle == SCE_LUA_STRING || initStyle == SCE_LUA_CHARACTER) {
		
			nestLevel = lineState >> 17;
			C1sepCount = (lineState >> 9) & 0xFF;
			sepCount = lineState & 0xFF;
			stringWs = lineState & 0x100;
		}
	}
	// results of identifier/keyword matching
	Sci_Position idenPos = 0;
	Sci_Position idenWordPos = 0;
	int idenStyle = SCE_LUA_IDENTIFIER;
	Sci_Position idenDimPos = 0;
	bool foundGoto = false;
	
	// Do not leak onto next line
	if (initStyle == SCE_LUA_STRINGEOL || initStyle == SCE_LUA_COMMENTLINE || initStyle == SCE_LUA_PREPROCESSOR) {
		initStyle = SCE_LUA_DEFAULT;
	}

	for (; sc.More(); sc.Forward()) {
		if (sc.atLineEnd) {
			// Update the line state, so it can be seen by next line
			currentLine = styler.GetLine(sc.currentPos);
			switch (sc.state) {
			case SCE_LUA_LITERALSTRING:
			case SCE_LUA_COMMENT:
			case SCE_LUA_STRING:
			case SCE_LUA_CHARACTER:
				// Inside a literal string, block comment or string, we set the line state
				styler.SetLineState(currentLine, (nestLevel << 17) | (C1sepCount << 9) | stringWs | sepCount);
				break;
			default:
				// Reset the line state

				//jass lua
				if (C1sepCount >0) {
					styler.SetLineState(currentLine, (C1sepCount << 9));
				}
				else {
					styler.SetLineState(currentLine, 0);
				}
				
				break;
			}
		}
		if (sc.atLineStart && (sc.state == SCE_LUA_STRING)) {
			// Prevent SCE_LUA_STRINGEOL from leaking back to previous line
			sc.SetState(SCE_LUA_STRING);
		}
		if (sc.atLineStart && (sc.state == SCE_LUA_COMMENTLINE)) {
			sc.SetState(SCE_LUA_DEFAULT);
		}
		// Handle string line continuation
		if ((sc.state == SCE_LUA_STRING || sc.state == SCE_LUA_CHARACTER) &&
			sc.ch == '\\') {
			if (sc.chNext == '\n' || sc.chNext == '\r') {
				sc.Forward();
				if (sc.ch == '\r' && sc.chNext == '\n') {
					sc.Forward();
				}
				continue;
			}
		}
		if (sc.state == 0) {
			sc.SetState(SCE_LUA_DEFAULT);
		}

		if (C1sepCount>0 && nestLevel==0 
			&& (sc.state != SCE_LUA_STRING && sc.state != SCE_LUA_CHARACTER 
			&& sc.state != SCE_LUA_COMMENTLINE && sc.state != SCE_LUA_PREPROCESSOR)) {
			//不能判断注释或字符串类型。
			if (sc.ch == ']') {
				//jass_lua 自动离开范围,无多层级要求。
				int sep = LongDelimCheck(sc);
				if (sep == 1 /* && C1sepCount == 1 */) {    // un-nest with ]]-only
					sc.SetState(SCE_LUA_IMPORT_FALG);
					sc.Forward();
					sc.ForwardSetState(SCE_LUA_DEFAULT);
					C1sepCount = 0;
				}
				else if (sep > 1 /* && sep == C1sepCount */) {   
					sc.SetState(SCE_LUA_IMPORT_FALG);
					sc.Forward(sep);
					sc.ForwardSetState(SCE_LUA_DEFAULT);
					C1sepCount = 0;
				}
			}
		}

		// Determine if the current state should terminate.
		if (sc.state == SCE_LUA_OPERATOR) {
			if (sc.ch == ':' && sc.chPrev == ':') {	// :: <label> :: forward scan
				sc.Forward();
				Sci_Position ln = 0;
				while (IsASpaceOrTab(sc.GetRelative(ln)))	// skip over spaces/tabs
					ln++;
				Sci_Position ws1 = ln;
				if (setWordStart.Contains(sc.GetRelative(ln))) {
					int c, i = 0;
					char s[100];
					while (setWord.Contains(c = sc.GetRelative(ln))) {	// get potential label
						if (i < 90)
							s[i++] = static_cast<char>(c);
						ln++;
					}
					s[i] = '\0'; Sci_Position lbl = ln;
					if (!keywords.InList(s)) {
						while (IsASpaceOrTab(sc.GetRelative(ln)))	// skip over spaces/tabs
							ln++;
						Sci_Position ws2 = ln - lbl;
						if (sc.GetRelative(ln) == ':' && sc.GetRelative(ln + 1) == ':') {
							// final :: found, complete valid label construct
							sc.ChangeState(SCE_LUA_LABEL);
							if (ws1) {
								sc.SetState(SCE_LUA_DEFAULT);
								sc.ForwardBytes(ws1);
							}
							sc.SetState(SCE_LUA_LABEL);
							sc.ForwardBytes(lbl - ws1);
							if (ws2) {
								sc.SetState(SCE_LUA_DEFAULT);
								sc.ForwardBytes(ws2);
							}
							sc.SetState(SCE_LUA_LABEL);
							sc.ForwardBytes(2);
						}
					}
				}
			}
			sc.SetState(SCE_LUA_DEFAULT);
		}
		else if (sc.state == SCE_LUA_NUMBER) {
			// We stop the number definition on non-numerical non-dot non-eEpP non-sign non-hexdigit char
			if (!setNumber.Contains(sc.ch)) {
				sc.SetState(SCE_LUA_DEFAULT);
			}
			else if (sc.ch == '-' || sc.ch == '+') {
				if (!setExponent.Contains(sc.chPrev))
					sc.SetState(SCE_LUA_DEFAULT);
			}
		}
		else if (sc.state == SCE_LUA_IDENTIFIER) {
			idenPos--;			// commit already-scanned identitier/word parts
			if (idenWordPos > 0) {
				idenWordPos--;
				Sci_Position idenWordPosCur = idenWordPos;
				if (idenDimPos > 1) {
					idenDimPos--;
					idenWordPosCur -= idenDimPos;
					sc.ForwardBytes(idenDimPos);
					sc.SetState(idenStyle);
				}
				else {
					sc.ChangeState(idenStyle);
				}
				if (idenWordPosCur > 0) {
					sc.ForwardBytes(idenWordPosCur);
				}
				idenPos -= idenWordPos;
				if (idenPos > 0) {
					sc.SetState(SCE_LUA_IDENTIFIER);
					sc.ForwardBytes(idenPos);
				}
				idenDimPos = 0;
				idenWordPos = 0;
			}
			else {
				sc.ForwardBytes(idenPos);
			}
			sc.SetState(SCE_LUA_DEFAULT);
			if (foundGoto) {					// goto <label> forward scan
				while (IsASpaceOrTab(sc.ch) && !sc.atLineEnd)
					sc.Forward();
				if (setWordStart.Contains(sc.ch)) {
					sc.SetState(SCE_LUA_LABEL);
					sc.Forward();
					while (setWord.Contains(sc.ch))
						sc.Forward();
					char s[100];
					sc.GetCurrent(s, sizeof(s));
					if (keywords.InList(s))		// labels cannot be keywords
						sc.ChangeState(SCE_LUA_WORD);
				}
				sc.SetState(SCE_LUA_DEFAULT);
			}
		}
		else if (sc.state == SCE_LUA_COMMENTLINE || sc.state == SCE_LUA_PREPROCESSOR) {
			if (sc.atLineEnd) {
				sc.ForwardSetState(SCE_LUA_DEFAULT);
			}
			//lua end
			else if (sc.state == SCE_LUA_COMMENTLINE && sc.ch == '?' && sc.chNext == '>')
			{
				sc.SetState(SCE_LUA_OPERATOR);
				sc.ForwardSetState(SCE_LUA_OPERATOR);
				//跳回jass
				sc.SetState(SCE_JASS_DEFAULT);
				return;
			}
		}
		else if (sc.state == SCE_LUA_STRING) {
			if (stringWs) {
				if (!IsASpace(sc.ch))
					stringWs = 0;
			}
			if (sc.ch == '\\') {
				if (setEscapeSkip.Contains(sc.chNext)) {
					sc.Forward();
				}
				else if (sc.chNext == 'z') {
					sc.Forward();
					stringWs = 0x100;
				}
			}
			else if (sc.ch == '\"') {
				sc.ForwardSetState(SCE_LUA_DEFAULT);
			}
			else if (stringWs == 0 && sc.atLineEnd) {
				sc.ChangeState(SCE_LUA_STRINGEOL);
				sc.ForwardSetState(SCE_LUA_DEFAULT);
			}
		}
		else if (sc.state == SCE_LUA_CHARACTER) {
			if (stringWs) {
				if (!IsASpace(sc.ch))
					stringWs = 0;
			}
			if (sc.ch == '\\') {
				if (setEscapeSkip.Contains(sc.chNext)) {
					sc.Forward();
				}
				else if (sc.chNext == 'z') {
					sc.Forward();
					stringWs = 0x100;
				}
			}
			else if (sc.ch == '\'') {
				sc.ForwardSetState(SCE_LUA_DEFAULT);
			}
			else if (stringWs == 0 && sc.atLineEnd) {
				sc.ChangeState(SCE_LUA_STRINGEOL);
				sc.ForwardSetState(SCE_LUA_DEFAULT);
			}
		}

		else if (sc.state == SCE_LUA_LITERALSTRING || sc.state == SCE_LUA_COMMENT) {
			if (sc.ch == '[') {
				const int sep = LongDelimCheck(sc);
				if (sep == 1 && sepCount == 1) {    // [[-only allowed to nest
					nestLevel++;
					sc.Forward();
				}
			}
			else if (sc.ch == ']') {
				int sep = LongDelimCheck(sc);
				if (sep == 1 && sepCount == 1) {    // un-nest with ]]-only
					nestLevel--;
					sc.Forward();
					if (nestLevel ==0) {
						sc.ForwardSetState(SCE_LUA_DEFAULT);
					}
				}
				else if (sep > 1 && sep == sepCount) {   // ]=]-style delim
					sc.Forward(sep);
					sc.ForwardSetState(SCE_LUA_DEFAULT);
				}
			}
		}
		
		// Determine if a new state should be entered.
		if (sc.state == SCE_LUA_DEFAULT) {
			if (IsADigit(sc.ch) || (sc.ch == '.' && IsADigit(sc.chNext))) {
				sc.SetState(SCE_LUA_NUMBER);
				if (sc.ch == '0' && toupper(sc.chNext) == 'X') {
					sc.Forward();
				}
			}
			//lua end
			else if (sc.ch == '?' && sc.chNext == '>')
			{
				sc.SetState(SCE_LUA_OPERATOR);
				//sc.ForwardSetState(SCE_LUA_OPERATOR);
				//sc.Forward();
				//跳回jass
				//sc.SetState(SCE_JASS_DEFAULT);
				return;
			}
			else if (setWordStart.Contains(sc.ch)) {
				// For matching various identifiers with dots and colons, multiple
				// matches are done as identifier segments are added. Longest match is
				// set to a word style. The non-matched part is in identifier style.
				std::string ident;
				idenPos = 0;
				idenWordPos = 0;
				idenDimPos = 0;
				idenStyle = SCE_LUA_IDENTIFIER;
				foundGoto = false;
				int cNext;
				int locDimPos = 0;
				do {
					int c;
					const Sci_Position idenPosOld = idenPos;
					std::string identSeg;
					identSeg += static_cast<char>(sc.GetRelative(idenPos++));
					while (setWord.Contains(c = sc.GetRelative(idenPos))) {
						identSeg += static_cast<char>(c);
						idenPos++;
					}
					if (keywords.InList(identSeg.c_str()) && (idenPosOld > 0)) {
						idenPos = idenPosOld - 1;	// keywords cannot mix
						ident.pop_back();
						break;
					}
					ident += identSeg;
					const char* s = ident.c_str();
					
					int newStyle = SCE_LUA_IDENTIFIER;
					if (keywords.InList(s)) {
						newStyle = SCE_LUA_WORD;
					}
					//else if (keywords2.InList(s)) {
					//	newStyle = SCE_LUA_WORD2;
					//}
					//else if (keywords3.InList(s)) {
					//	newStyle = SCE_LUA_WORD3;
					//}
					//else if (keywords4.InList(s)) {
					//	newStyle = SCE_LUA_WORD4;
					//}
					//跳过分界符以支持cj.Atan2、...
					if (locDimPos>0 && sc.GetRelative(locDimPos-1) == '.') {
						s += locDimPos;
					}
					cNext = sc.GetRelative(idenPos + 1);
					if(newStyle!= SCE_LUA_IDENTIFIER){}
					else if (keywords5.InList(s)) {
						newStyle = SCE_LUA_WORD5;
					}
					else if (keywords6.InList(s)) {
						newStyle = SCE_LUA_WORD6;
					}
					else if (keywords7.InList(s)) {
						newStyle = SCE_LUA_WORD7;
					}
					if (newStyle != SCE_LUA_IDENTIFIER) {
						if (newStyle == SCE_LUA_WORD5 || newStyle == SCE_LUA_WORD6 || newStyle == SCE_LUA_WORD7) {
							idenDimPos = locDimPos;
						}
						idenStyle = newStyle;
						idenWordPos = idenPos;
					}
					if (idenStyle == SCE_LUA_WORD)	// keywords cannot mix
						break;
					if ((c == '.' || c == ':') && setWordStart.Contains(cNext)) {
						ident += static_cast<char>(c);
						idenPos++;
						locDimPos = idenPos;
					}
					else {
						cNext = 0;
					}
				} while (cNext);

				if ((idenStyle == SCE_LUA_WORD) && (ident.compare("goto") == 0)) {
					foundGoto = true;
				}

				if (idenStyle != SCE_LUA_WORD && idenPos > 0) {
					int startPos = idenPos;
					cNext = GetNextVisibleChar(sc, &startPos);
					if (setLuaNameRight.Contains(cNext)) {
						if (locDimPos > 0) {
							idenStyle = SCE_LUA_KEY;
							idenWordPos = locDimPos;
						}
						else {
							idenStyle = SCE_LUA_NAME;
							idenWordPos = idenPos;
						}
					}else if (locDimPos > 0) {
						sc.SetState(SCE_LUA_NAME);
						sc.Forward(locDimPos-1);
					}
				}
				sc.SetState(SCE_LUA_IDENTIFIER);
			}
			else if (sc.ch == '\"') {
				sc.SetState(SCE_LUA_STRING);
				stringWs = 0;
			}
			else if (sc.ch == '\'') {
				sc.SetState(SCE_LUA_CHARACTER);
				stringWs = 0;
			}
			else if (sc.ch == '[') {
				sepCount = LongDelimCheck(sc);
				if (sepCount == 0) {
					sc.SetState(SCE_LUA_OPERATOR);
				}
				else {
					bool is_lua_import = false;
					if (!C1sepCount) {
						is_lua_import = check_lua_import(styler, sc.currentPos - 1);
						if (is_lua_import) {
							nestLevel = 0;
							sc.SetState(SCE_LUA_IMPORT_FALG);
							sc.Forward(sepCount);
							C1sepCount = sepCount;
							sc.ForwardSetState(SCE_LUA_DEFAULT);
						}
					}
					if (!is_lua_import) {
						nestLevel = 1;
						sc.SetState(SCE_LUA_LITERALSTRING);
						sc.Forward(sepCount);
					}
				}
			}
			else if (sc.Match('-', '-')) {
				sc.SetState(SCE_LUA_COMMENTLINE);
				if (sc.Match("--[")) {
					sc.Forward(2);
					sepCount = LongDelimCheck(sc);
					if (sepCount > 0) {
						bool is_lua_import = false;
						if (!C1sepCount) {
							is_lua_import = check_lua_import(styler, sc.currentPos - 1);
							if (is_lua_import) {
								nestLevel = 0;
								sc.ChangeState(SCE_LUA_IMPORT_FALG);
								sc.Forward(sepCount);
								C1sepCount = sepCount;
								//sc.ForwardSetState(SCE_LUA_DEFAULT);
							}
						}
						if(!is_lua_import) {
							nestLevel = 1;
							sc.ChangeState(SCE_LUA_COMMENT);
							sc.Forward(sepCount);
						}
					}
				}
				else {
					sc.Forward();
				}
			}
			else if (sc.atLineStart && sc.Match('$')) {
				sc.SetState(SCE_LUA_PREPROCESSOR);	// Obsolete since Lua 4.0, but still in old code
			}
			else if (setLuaOperator.Contains(sc.ch)) {
				sc.SetState(SCE_LUA_OPERATOR);
			}
		}
	}

	//sc.Complete();
}

static void ColouriseJassDoc(
	Sci_PositionU startPos,
	Sci_Position length,
	int initStyle,
	WordList *keywordlists[],
	Accessor &styler) {

	const WordList &keywords = *keywordlists[0];
	const WordList &keywords2 = *keywordlists[2];
	const WordList &keywords3 = *keywordlists[3];
	const WordList &keywords4 = *keywordlists[4];
	const WordList &keywords5 = *keywordlists[5];
	const WordList &keywords6 = *keywordlists[6];
	const WordList &keywords7 = *keywordlists[7];

	// Accepts accented characters
	CharacterSet setWordStart(CharacterSet::setAlpha, "_", 0x80, true);
	CharacterSet setWord(CharacterSet::setAlphaNum, "_", 0x80, true);
	// Not exactly following number definition (several dots are seen as OK, etc.)
	// but probably enough in most cases. [pP] is for hex floats.
	CharacterSet setNumber(CharacterSet::setDigits, ".-+abcdefpABCDEFP");
	CharacterSet setExponent(CharacterSet::setNone, "eEpP");
	CharacterSet setOperator(CharacterSet::setNone, "*/-+()={}[]<>&|!\\");
	CharacterSet setEscapeSkip(CharacterSet::setNone, "\"'\\");

	Sci_Position currentLine = styler.GetLine(startPos);
	// Initialize long string [[ ... ]] or block comment --[[ ... ]] nesting level,
	// if we are inside such a string. Block comment was introduced in Lua 5.0,
	// blocks with separators [=[ ... ]=] in Lua 5.1.
	// Continuation of a string (\z whitespace escaping) is controlled by stringWs.
	int nestLevel = 0;
	int sepCount = 0;
	int stringWs = 0;
	int strlineCount = 0;
	if (initStyle == SCE_JASS_COMMENT ||
		initStyle == SCE_JASS_STRING || initStyle == SCE_JASS_CHARACTER) {
		const int lineState = styler.GetLineState(currentLine - 1);
		nestLevel = lineState >> 17;
		strlineCount = (lineState >> 9) & 0xFF;
		sepCount = lineState & 0xFF;
		stringWs = lineState & 0x100;
	}
	
	// results of identifier/keyword matching
	Sci_Position idenPos = 0;
	Sci_Position idenWordPos = 0;
	int idenStyle = SCE_JASS_IDENTIFIER;
	//SCE_JASS_INCLUDE_LUA
	// Do not leak onto next line
	if (initStyle == SCE_JASS_STRINGEOL 
		|| initStyle == SCE_JASS_COMMENTLINE 
		|| initStyle == SCE_JASS_PREPROCESSOR 
		|| initStyle == SCE_JASS_RUNTEXTMACRO) {
		initStyle = SCE_JASS_DEFAULT;
	}

	StyleContext sc(startPos, length, initStyle, styler);
	if (initStyle >= SCE_LUA_DEFAULT) {
		ColouriseJassLuaDoc(sc, initStyle, keywordlists, styler);
		initStyle = SCE_JASS_DEFAULT;
	}
	for (; sc.More(); sc.Forward()) {
		//if (sc.state >= SCE_LUA_DEFAULT) {
		//	sc.ForwardSetState(SCE_JASS_DEFAULT);
		//	//continue;
		//}
		if (sc.atLineEnd) {
			// Update the line state, so it can be seen by next line
			currentLine = styler.GetLine(sc.currentPos);
			
			switch (sc.state) {
			case SCE_JASS_COMMENTFLAG:
			case SCE_JASS_COMMENT:
			case SCE_JASS_STRING:
			case SCE_JASS_CHARACTER:
				// Inside a literal string, block comment or string, we set the line state
				styler.SetLineState(currentLine, (nestLevel << 17) | (strlineCount << 9) | stringWs | sepCount);
				break;
			default:
				// Reset the line state
				styler.SetLineState(currentLine, 0);
				break;
			}
		}
		
		if (sc.atLineStart && (sc.state == SCE_JASS_STRING)) {
			// Prevent SCE_LUA_STRINGEOL from leaking back to previous line
			if (strlineCount<0xFF) {
				strlineCount++;
			}
			sc.SetState(SCE_JASS_STRING);
		}

		// Handle string line continuation
		if ((sc.state == SCE_JASS_STRING || sc.state == SCE_JASS_CHARACTER) &&
			sc.ch == '\\') {
			if (sc.chNext == '\n' || sc.chNext == '\r') {
				sc.Forward();
				if (sc.ch == '\r' && sc.chNext == '\n') {
					sc.Forward();
				}
				continue;
			}
		}

		// 跳回jass
		if (sc.state >= SCE_LUA_DEFAULT) {
			sc.SetState(SCE_JASS_DEFAULT);
		}
		switch (sc.state)
		{
		case SCE_JASS_OPERATOR:
			sc.SetState(SCE_JASS_DEFAULT);
			break;
		case SCE_JASS_NUMBER:
			// We stop the number definition on non-numerical non-dot non-eEpP non-sign non-hexdigit char
			if (!setNumber.Contains(sc.ch)) {
				sc.SetState(SCE_JASS_DEFAULT);
			}
			else if (sc.ch == '-' || sc.ch == '+') {
				if (!setExponent.Contains(sc.chPrev))
					sc.SetState(SCE_JASS_DEFAULT);
			}
			break;
		case SCE_JASS_IDENTIFIER:
			idenPos--;			// commit already-scanned identitier/word parts
			if (idenWordPos > 0) {
				idenWordPos--;
				sc.ChangeState(idenStyle);
				sc.ForwardBytes(idenWordPos);
				idenPos -= idenWordPos;
				if (idenPos > 0) {
					sc.SetState(SCE_JASS_IDENTIFIER);
					sc.ForwardBytes(idenPos);
				}
			}
			else {
				sc.ForwardBytes(idenPos);
			}
			sc.SetState(SCE_JASS_DEFAULT);
			break;
		case SCE_JASS_PREPROCESSOR:
			if (IsASpaceOrTab(sc.ch)) {
				sc.ForwardSetState(SCE_JASS_DEFAULT);
			}
			if (sc.atLineEnd) {
				sc.ForwardSetState(SCE_JASS_DEFAULT);
			}
			break;
		case SCE_JASS_COMMENTLINE:
		case SCE_JASS_RUNTEXTMACRO:
			if (sc.atLineEnd) {
				sc.ForwardSetState(SCE_JASS_DEFAULT);
			}
			break;
		case SCE_JASS_STRING:
			if (stringWs) {
				if (!IsASpace(sc.ch))
					stringWs = 0;
			}
			if (sc.ch == '\\') {
				if (setEscapeSkip.Contains(sc.chNext)) {
					sc.Forward();
				}
				else if (sc.chNext == 'z') {
					sc.Forward();
					stringWs = 0x100;
				}
			}
			else if (sc.ch == '\"') {
				sc.ForwardSetState(SCE_JASS_DEFAULT);
			}
			else if (strlineCount >= 0xFF && sc.atLineEnd) {
				sc.ChangeState(SCE_JASS_STRINGEOL);
				sc.ForwardSetState(SCE_JASS_DEFAULT);
			}
			break;
		case SCE_JASS_CHARACTER:
			if (stringWs) {
				if (!IsASpace(sc.ch))
					stringWs = 0;
			}
			if (sc.ch == '\\') {
				if (setEscapeSkip.Contains(sc.chNext)) {
					sc.Forward();
				}
				else if (sc.chNext == 'z') {
					sc.Forward();
					stringWs = 0x100;
				}
			}
			else if (sc.ch == '\'') {
				sc.ForwardSetState(SCE_JASS_DEFAULT);
			}
			else if (stringWs == 0 && sc.atLineEnd) {
				sc.ChangeState(SCE_JASS_STRINGEOL);
				sc.ForwardSetState(SCE_JASS_DEFAULT);
			}
			break;
		case SCE_JASS_COMMENT:
		case SCE_JASS_COMMENTFLAG:
			if (sc.Match('*', '/')) {
				sc.SetState(SCE_JASS_COMMENTFLAG);
				sc.ForwardSetState(SCE_JASS_COMMENTFLAG);
				nestLevel = 0;
				sc.ForwardSetState(SCE_JASS_DEFAULT);
			}
			break;
		default:
			break;
		}
		// Determine if a new state should be entered.

		if (sc.state == SCE_JASS_DEFAULT) {
			//最后一个连接字符不是G-Z_
			if (sc.ch == '$' && setNumber.Contains(sc.chNext)) {
				if (check_s_hex(styler,sc.currentPos, setNumber)) {
					sc.SetState(SCE_JASS_NUMBER);
					//sc.Forward();
				}
			}
		}
		if (sc.state == SCE_JASS_DEFAULT) {
			if (IsADigit(sc.ch) || (sc.ch == '.' && IsADigit(sc.chNext))) {
				sc.SetState(SCE_JASS_NUMBER);
				if (sc.ch == '0' && toupper(sc.chNext) == 'X') {
					sc.Forward();
				}
			}
			//lua start
			else if (sc.ch == '<' && sc.chNext == '?'){
				sc.SetState(SCE_LUA_OPERATOR);
				sc.ForwardSetState(SCE_LUA_OPERATOR);
				sc.Forward();//skip+2
				sc.SetState(SCE_LUA_DEFAULT);
				ColouriseJassLuaDoc(sc, SCE_LUA_DEFAULT, keywordlists,styler);
			}
			else if (setWordStart.Contains(sc.ch)) {
				// For matching various identifiers with dots and colons, multiple
				// matches are done as identifier segments are added. Longest match is
				// set to a word style. The non-matched part is in identifier style.
				std::string ident;
				idenPos = 0;
				idenWordPos = 0;
				idenStyle = SCE_JASS_IDENTIFIER;
				int cNext;
				do {
					int c;
					const Sci_Position idenPosOld = idenPos;
					std::string identSeg;
					identSeg += static_cast<char>(sc.GetRelative(idenPos++));
					while (setWord.Contains(c = sc.GetRelative(idenPos))) {
						identSeg += static_cast<char>(c);
						idenPos++;
					}
					//if (keywords.InList(identSeg.c_str()) && (idenPosOld > 0)) {
					//	idenPos = idenPosOld - 1;	// keywords cannot mix
					//	ident.pop_back();
					//	break;
					//}
					ident += identSeg;
					const char* s = ident.c_str();
					int newStyle = SCE_JASS_IDENTIFIER;
					if (keywords.InList(s)) {
						newStyle = SCE_JASS_WORD;
					}
					else if (keywords2.InList(s)) {
						newStyle = SCE_JASS_WORD2;
					}
					else if (keywords3.InList(s)) {
						newStyle = SCE_JASS_WORD3;
					}
					else if (keywords4.InList(s)) {
						newStyle = SCE_JASS_WORD4;
					}
					else if (keywords5.InList(s)) {
						newStyle = SCE_JASS_WORD5;
					}
					else if (keywords6.InList(s)) {
						newStyle = SCE_JASS_WORD6;
					}
					else if (keywords7.InList(s)) {
						newStyle = SCE_JASS_WORD7;
					}
					if (newStyle != SCE_JASS_IDENTIFIER) {
						idenStyle = newStyle;
						idenWordPos = idenPos;
					}
					if (idenStyle == SCE_JASS_WORD)	// keywords cannot mix
						break;
					cNext = sc.GetRelative(idenPos + 1);
					if ((c == '.') && setWordStart.Contains(cNext)) {
						ident += static_cast<char>(c);
						idenPos++;
					}
					else {
						cNext = 0;
					}
				} while (cNext);
				sc.SetState(SCE_JASS_IDENTIFIER);
			}
			else if (sc.ch == '\"') {
				sc.SetState(SCE_JASS_STRING);
				stringWs = 0;
				strlineCount = 1;
			}
			else if (sc.ch == '\'') {
				sc.SetState(SCE_JASS_CHARACTER);
				stringWs = 0;
			}
			else if (sc.Match("//!")) {
				sc.SetState(SCE_JASS_RUNTEXTMACRO);
				sc.Forward(2);
			}
			else if (sc.Match('/','/')) {
				sc.SetState(SCE_JASS_COMMENTLINE);
				sc.Forward();
			}
			else if (sc.Match('/', '*')) {
				sc.SetState(SCE_JASS_COMMENTFLAG);
				sc.ForwardSetState(SCE_JASS_COMMENTFLAG);
				sc.ForwardSetState(SCE_JASS_COMMENT);
				nestLevel = 1;
			}
			else if (sc.Match('#')) {
				sc.SetState(SCE_JASS_PREPROCESSOR);
			}
			else if (setOperator.Contains(sc.ch)) {
				sc.SetState(SCE_JASS_OPERATOR);
				if ((sc.ch == '-' || sc.ch == '+') && sc.chNext == '$') {
					sc.ForwardSetState(SCE_JASS_NUMBER);
				}
			}
		}
	}

	sc.Complete();
}
static size_t copy_styler_word(Accessor &styler, Sci_PositionU i, Sci_PositionU maxPos, char* const &s,size_t maxlen, int* style_len) {
	maxlen = (maxlen) ? maxlen-- : maxlen;
	Sci_PositionU n = 0;
	int count = 0;
	for (int style = styler.StyleAt(i);i < maxPos && style == styler.StyleAt(i + n) ; n++) {
		count++;
		if (n < maxlen) {
			if (iswordchar(styler[i + n])) {
				s[n] = styler[i + n];
				s[n + 1] = '\0';
			}
		}
	}
	*style_len = count;
	return n;
}
static char get_last_notspace_char(Accessor &styler, Sci_PositionU minPos, Sci_PositionU i) {
	char ch = 0;
	if (i==0 || i<minPos) {
		return 0;
	}
	--i;
	for (; i > minPos;--i) {
		ch = styler[i];
		if (ch == '\n' || ch == '\r') {
			return 0;
		}
		else if (ch != '\t' && ch != ' ') {
			return ch;
		}
	}
	return 0;
}

static void FoldJassDoc(Sci_PositionU startPos, Sci_Position length, int /* initStyle */, WordList *[],
	Accessor &styler) {
	const Sci_PositionU lengthDoc = startPos + length;
	int visibleChars = 0;
	Sci_Position lineCurrent = styler.GetLine(startPos);
	int levelPrev = styler.LevelAt(lineCurrent) & SC_FOLDLEVELNUMBERMASK;
	int levelCurrent = levelPrev;
	char chNext = styler[startPos];
	const bool foldCompact = styler.GetPropertyInt("fold.compact", 1) != 0;
	int styleNext = styler.StyleAt(startPos);
	char s[16] = {0};
	int style_len;

	bool line_lua_import_flag_start = false;
	bool line_lua_import_flag_end = false;

	bool interface_start = false;
	bool line_function_start = false;

	bool library_start = false;
	bool function_start = false;
	bool if_start = false;
	int highlevel_flag = 0;
	for (Sci_PositionU i = startPos; i < lengthDoc; i++) {
		const char ch = styler.SafeGetCharAt(i);
		chNext = styler.SafeGetCharAt(i + 1);
		const int style = styler.StyleAt(i);
		styleNext = styler.StyleAt(i + 1);
		const bool atEOL = (ch == '\r' && chNext != '\n') || (ch == '\n');
		size_t wordlen = 0;
		if (atEOL) {
			line_lua_import_flag_start = false;
			line_lua_import_flag_end = false;
			//重置前一关键词;
			line_function_start=false;

		}
		switch (style)
		{
		case SCE_JASS_WORD:
			wordlen = copy_styler_word(styler, i, lengthDoc, s, sizeof(s), &style_len);
			switch (ch)
			{
			case 'l':
				if ((strcmp(s, "loop") == 0)) {
					levelCurrent++;
				}
				else if ((strcmp(s, "library") == 0)) {
					highlevel_flag = SC_FOLDLEVELHIGHHEADERFLAG;
					levelCurrent++; library_start = true;
				}
				break;
			case 'g':
				if (strcmp(s, "globals") == 0) {
					levelCurrent++;
				}
				break;
			case 's':
				if (strcmp(s, "scope") == 0 || strcmp(s, "struct") == 0) {
					highlevel_flag = SC_FOLDLEVELHIGHHEADERFLAG;
					levelCurrent++; 
				}
				break;
			case 'm':
				if (strcmp(s, "module") == 0) {
					highlevel_flag = SC_FOLDLEVELHIGHHEADERFLAG;
					levelCurrent++; 
				}
				if (interface_start) { break; }
				if (strcmp(s, "method") == 0) {
					levelCurrent++; 
				}

				break;
			case 'f':
				if (strcmp(s, "function") == 0) {
					char last_notspace_char = get_last_notspace_char(styler, startPos, i);
					if (i == 0 || last_notspace_char == 0 || last_notspace_char == '_' || IsAlphaNumeric(last_notspace_char)) {
						levelCurrent++;
						line_function_start = true;
						function_start = true;
					}
				}
				break;
			case 'i':
				if (strcmp(s, "if") == 0) {
					levelCurrent++; if_start = true;

				}
				//如果前一关键词为function,则跳过
				if (line_function_start) { break; }
				if (strcmp(s, "interface") == 0) {
					levelCurrent++; interface_start = true;
				}
				break;
			case 'e':
				if (wordlen < sizeof("endif") - 1) {
					break;
				}
				if (strcmp(s, "endlibrary") == 0) {
					levelCurrent--; library_start = false;
				}
				else if (strcmp(s, "endfunction") == 0) {
					levelCurrent--; function_start = false; line_function_start = false;
				}
				else if (strcmp(s, "endif") == 0) {
					levelCurrent--; if_start = false;
				}
				else if (strcmp(s, "endinterface") == 0) {
					levelCurrent--; interface_start = false;
				}
				else if (strcmp(s, "endglobals") == 0
					|| strcmp(s, "endscope") == 0
					|| strcmp(s, "endmodule") == 0
					|| strcmp(s, "endstruct") == 0
					|| strcmp(s, "endmethod") == 0
					|| strcmp(s, "endloop") == 0
					) {
					levelCurrent--; visibleChars += wordlen;
				}
			}
			visibleChars += style_len;
			if (style_len>0) {
				i += style_len; i--;
			}
			continue;
			break;
		case SCE_JASS_WORD2:
		case SCE_JASS_WORD3:
		case SCE_JASS_WORD4:
		case SCE_JASS_WORD5:
		case SCE_JASS_WORD6:
		case SCE_JASS_WORD7:
			break;
		case SCE_JASS_COMMENTFLAG:
			if (ch == '/' && chNext == '*') {
				levelCurrent++; visibleChars++; i++;

			}
			else if (ch == '*' && chNext == '/') {
				levelCurrent--; visibleChars++; i++;
			}
			break;
		case SCE_JASS_OPERATOR:
			//兼容zinc
			if (ch == '{') {
				if (!(library_start || function_start || if_start)) {
					levelCurrent++;
				}

			}
			else if (ch == '}') {
				if (!(library_start || function_start || if_start)) {
					levelCurrent--;
				}
			}
			break;
		case SCE_LUA_WORD:
			wordlen = copy_styler_word(styler, i, lengthDoc, s, sizeof(s), &style_len);
			if (ch == 'i' || ch == 'd' || ch == 'f' || ch == 'e' || ch == 'r' || ch == 'u') {
				if ((strcmp(s, "if") == 0) || (strcmp(s, "do") == 0) || (strcmp(s, "function") == 0) || (strcmp(s, "repeat") == 0)) {
					levelCurrent++; 
				}
				else if ((strcmp(s, "end") == 0) || (strcmp(s, "until") == 0)) {  // || (strcmp(s, "elseif") == 0)
					levelCurrent--; 
				}
			}
			visibleChars += style_len;
			if (style_len > 0) {
				i += style_len; i--;
			}
			continue;
			break;
		case SCE_LUA_WORD5:
		case SCE_LUA_WORD6:
		case SCE_LUA_WORD7:
			break;
		case SCE_LUA_OPERATOR:
			if (ch == '{') {
				levelCurrent++;
			}
			else if (ch == '}') {
				levelCurrent--;
			}
			//lua 域
			else if (ch == '<' && chNext == '?') {
				highlevel_flag = SC_FOLDLEVELHIGHHEADERFLAG;
				levelCurrent++; visibleChars++; i++;
			}
			else if (ch == '?' && chNext == '>') {
				levelCurrent--; visibleChars++; i++;

			}
			break;
		case SCE_LUA_LITERALSTRING:
		case SCE_LUA_COMMENT:
			if (ch == '[') {
				levelCurrent++;
			}
			else if (ch == ']') {
				levelCurrent--;
			}
			break;
		case SCE_LUA_IMPORT_FALG:
			if (ch == '[' && !line_lua_import_flag_start) {
				highlevel_flag = SC_FOLDLEVELHIGHHEADERFLAG;
				line_lua_import_flag_start = true;
				levelCurrent++;
			}
			else if (ch == ']' && !line_lua_import_flag_end) {
				line_lua_import_flag_end = true;
				levelCurrent--;
			}
			break;
		default:
			break;
		}

		if (atEOL) {
			int lev = levelPrev;
			if (levelCurrent < levelPrev  && visibleChars > 0 && foldCompact) {
				lev |= SC_FOLDLEVELWHITEFLAG;
			}
			if ((levelCurrent > levelPrev) && (visibleChars > 0)) {
				lev |= SC_FOLDLEVELHEADERFLAG | highlevel_flag;
			}
			if (lev != styler.LevelAt(lineCurrent)) {
				styler.SetLevel(lineCurrent, lev);
			}
			lineCurrent++;
			levelPrev = levelCurrent;
			visibleChars = 0;
			highlevel_flag = 0;
		}
		if (!isspacechar(ch)) {
			visibleChars++;
		}
	}
	// Fill in the real level of the next line, keeping the current flags as they will be filled in later

	int flagsNext = styler.LevelAt(lineCurrent) & ~SC_FOLDLEVELNUMBERMASK;
	styler.SetLevel(lineCurrent, levelPrev | flagsNext);
}

//KEYWORDSET_MAX 8 关键词列表大小范围,最多9个,=max+1,不建议超过预定义值(仅更改预定义仍出错)。
static const char * const JassWordListDesc[] = {
	"jass_keywords",
	"lua_keywords",
	"jass/lua:functions",
	"jass/lua:string, (table) & math functions",
	"jass/lua:(coroutines), I/O & system facilities",
	"jass/lua:user1",
	"jass/lua:user2",
	"jass/lua:user3",
	0
};

namespace {

	LexicalClass lexicalClasses[] = {
		// Lexer Lua SCLEX_LUA SCE_LUA_:
		0, "SCE_JASS_DEFAULT", "default", "White space: Visible only in View Whitespace mode (or if it has a back colour)",
		1, "SCE_JASS_COMMENT", "comment", "Block comment (Lua 5.0)",
		2, "SCE_JASS_COMMENTLINE", "comment line", "Line comment",
		3, "SCE_JASS_COMMENTFLAG", "comment documentation", "Doc comment -- Not used in Lua (yet?)",
		4, "SCE_JASS_NUMBER", "literal numeric", "Number",
		5, "SCE_JASS_WORD", "keyword", "Keyword",
		6, "SCE_JASS_STRING", "literal string", "(Double quoted) String",
		7, "SCE_JASS_CHARACTER", "literal string character", "Character (Single quoted string)",
		8, "SCE_JASS_PREPROCESSOR", "preprocessor", "preprocessor",
		9, "SCE_JASS_OPERATOR", "operator", "Operators",
		10, "SCE_JASS_IDENTIFIER", "identifier", "Identifier (everything else...)",
		11, "SCE_JASS_STRINGEOL", "error literal string", "End of line where string is not closed",
		12, "SCE_JASS_RUNTEXTMACRO", "runtextmacro", "runtextmacro",
		13, "SCE_JASS_WORD2", "identifier", "Other keywords",
		14, "SCE_JASS_WORD3", "identifier", "Other keywords",
		15, "SCE_JASS_WORD4", "identifier", "Other keywords",
		16, "SCE_JASS_WORD5", "identifier", "Other keywords",
		17, "SCE_JASS_WORD6", "identifier", "Other keywords",
		18, "SCE_JASS_WORD7", "identifier", "Other keywords",
		19, "SCE_LUA_DEFAULT", "default", "White space: Visible only in View Whitespace mode (or if it has a back colour)",
		20, "SCE_LUA_COMMENT", "comment", "Block comment (Lua 5.0)",
		21, "SCE_LUA_COMMENTLINE", "comment line", "Line comment",
		22, "SCE_LUA_COMMENTDOC", "comment documentation", "Doc comment -- Not used in Lua (yet?)",
		23, "SCE_LUA_NUMBER", "literal numeric", "Number",
		24, "SCE_LUA_WORD", "keyword", "Keyword",
		25, "SCE_LUA_STRING", "literal string", "(Double quoted) String",
		26, "SCE_LUA_CHARACTER", "literal string character", "Character (Single quoted string)",
		27, "SCE_LUA_LITERALSTRING", "literal string", "Literal string",
		28, "SCE_LUA_PREPROCESSOR", "preprocessor", "Preprocessor (obsolete in Lua 4.0 and up)",
		29, "SCE_LUA_OPERATOR", "operator", "Operators",
		30, "SCE_LUA_IDENTIFIER", "identifier", "Identifier (everything else...)",
		31, "SCE_LUA_STRINGEOL", "error literal string", "End of line where string is not closed",
		32, "SCE_LUA_LABEL", "label", "Labels",
		33, "SCE_LUA_IMPORT_FALG", "luaimportflag", "luaimportflag",
		34, "SCE_LUA_NAME", "lua_name1", "lua_name1",
		35, "SCE_LUA_KEY", "lua_key1", "lua_key1",
		36, "SCE_LUA_WORD2", "identifier", "Other keywords",
		37, "SCE_LUA_WORD3", "identifier", "Other keywords",
		38, "SCE_LUA_WORD4", "identifier", "Other keywords",
		39, "SCE_LUA_WORD5", "identifier", "Other keywords",
		40, "SCE_LUA_WORD6", "identifier", "Other keywords",
		41, "SCE_LUA_WORD7", "identifier", "Other keywords",
	};

}

LexerModule lmJass(SCLEX_JASS, ColouriseJassDoc, "j", FoldJassDoc, JassWordListDesc, lexicalClasses, ELEMENTS(lexicalClasses));
