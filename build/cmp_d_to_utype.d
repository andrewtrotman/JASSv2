/*
	CMP_D_TO_UTYPE.D
	----------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)

	Program written in D to compare the JASS unicode utype methods to the D impleentations of the same
	routines.  At time of writing the D Runtime library (Phobos) was correct against version 6.2 of the
	Unicode specification and JASS was correct against version 9.0.  So, unless you check against an old
	version of Unicode you won't get a perfect match.
*/
import std.stdio;
import std.uni;

extern (C++)
	{
	static bool JASS_unicode_isalpha(uint codepoint);
	static bool JASS_unicode_iscntrl(uint codepoint);
	static bool JASS_unicode_isgraph(uint codepoint);
	static bool JASS_unicode_islower(uint codepoint);
	static bool JASS_unicode_isupper(uint codepoint);
	static bool JASS_unicode_ismark(uint codepoint);
	static bool JASS_unicode_isdigit(uint codepoint);
	static bool JASS_unicode_ispunct(uint codepoint);
	static bool JASS_unicode_isuspace(uint codepoint);
	static bool JASS_unicode_isspace(uint codepoint);
	static bool JASS_unicode_issymbol(uint codepoint);
	}

int main()
{
int codepoint;

for (codepoint = 0; codepoint < 0x10FFFF; codepoint++)
	{
	if (std.uni.isAlpha(codepoint) != JASS_unicode_isalpha(codepoint))
		writefln("ALPHA:Codepoint:0x%X", codepoint, "D says", std.uni.isControl(codepoint));

	if (std.uni.isControl(codepoint) != JASS_unicode_iscntrl(codepoint))
		writefln("CNTRL:Codepoint:0x%X", codepoint, "D says", std.uni.isControl(codepoint));

	if (std.uni.isGraphical(codepoint) != JASS_unicode_isgraph(codepoint))
		writefln("GRAPH:Codepoint:0x%X", codepoint, "D says", std.uni.isGraphical(codepoint));

	if (std.uni.isLower(codepoint) != JASS_unicode_islower(codepoint))
		writefln("LOWER:Codepoint:0x%X", codepoint, "D says", std.uni.isLower(codepoint));

	if (std.uni.isUpper(codepoint) != JASS_unicode_isupper(codepoint))
		writefln("UPPER:Codepoint:0x%X", codepoint, "D says", std.uni.isUpper(codepoint));

	if (std.uni.isMark(codepoint) != JASS_unicode_ismark(codepoint))
		writefln("MARK:Codepoint:0x%X", codepoint, "D says", std.uni.isMark(codepoint));

	if (std.uni.isNumber(codepoint) != JASS_unicode_isdigit(codepoint))
		writefln("DIGIR:Codepoint:0x%X", codepoint, "D says", std.uni.isNumber(codepoint));

	if (std.uni.isPunctuation(codepoint) != JASS_unicode_ispunct(codepoint))
		writefln("PUNCT:Codepoint:0x%X", codepoint, "D says", std.uni.isPunctuation(codepoint));

	if (std.uni.isSpace(codepoint) != JASS_unicode_isuspace(codepoint))
		writefln("USPACE:Codepoint:0x%X", codepoint, "D says", std.uni.isSpace(codepoint));

	if (std.uni.isWhite(codepoint) != JASS_unicode_isspace(codepoint))
		writefln("USPACE:Codepoint:0x%X", codepoint, "D says", std.uni.isWhite(codepoint));

	if (std.uni.isSymbol(codepoint) != JASS_unicode_issymbol(codepoint))
		writefln("USPACE:Codepoint:0x%X", codepoint, "D says", std.uni.isSymbol(codepoint));
	}

return 0;
}
