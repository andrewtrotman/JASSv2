import std.stdio;
import std.uni;

extern (C++, JASS)
	{
	class unicode_ctype
		{
		static bool isalpha(uint codepoint);
		}
	}

int main()
{
int codepoint;

for (codepoint = 0; codepoint < 0x10FFFF; codepoint++)
	{
	if (std.uni.isAlpha(codepoint) != unicode_isalpha(codepoint))
		writefln("ALPHA:Codepoint:0x%X", codepoint, "D says", std.uni.isControl(codepoint));

	if (std.uni.isControl(codepoint) != unicode_iscntrl(codepoint))
		writefln("CNTRL:Codepoint:0x%X", codepoint, "D says", std.uni.isControl(codepoint));

	if (std.uni.isGraphical(codepoint) != unicode_isgraph(codepoint))
		writefln("GRAPH:Codepoint:0x%X", codepoint, "D says", std.uni.isGraphical(codepoint));

	if (std.uni.isLower(codepoint) != unicode_islower(codepoint))
		writefln("LOWER:Codepoint:0x%X", codepoint, "D says", std.uni.isLower(codepoint));

	if (std.uni.isUpper(codepoint) != unicode_isupper(codepoint))
		writefln("UPPER:Codepoint:0x%X", codepoint, "D says", std.uni.isUpper(codepoint));

	if (std.uni.isMark(codepoint) != unicode_ismark(codepoint))
		writefln("MARK:Codepoint:0x%X", codepoint, "D says", std.uni.isMark(codepoint));

	if (std.uni.isNumber(codepoint) != unicode_isdigit(codepoint))
		writefln("DIGIR:Codepoint:0x%X", codepoint, "D says", std.uni.isNumber(codepoint));

	if (std.uni.isPunctuation(codepoint) != unicode_ispunct(codepoint))
		writefln("PUNCT:Codepoint:0x%X", codepoint, "D says", std.uni.isPunctuation(codepoint));

	if (std.uni.isSpace(codepoint) != unicode_isuspace(codepoint))
		writefln("USPACE:Codepoint:0x%X", codepoint, "D says", std.uni.isSpace(codepoint));

	if (std.uni.isWhite(codepoint) != unicode_isspace(codepoint))
		writefln("USPACE:Codepoint:0x%X", codepoint, "D says", std.uni.isWhite(codepoint));

	if (std.uni.isSymbol(codepoint) != unicode_issymbol(codepoint))
		writefln("USPACE:Codepoint:0x%X", codepoint, "D says", std.uni.isSymbol(codepoint));
	}

return 0;
}
