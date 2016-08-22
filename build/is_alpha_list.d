import std.stdio;
import std.uni;

extern (C) uint unicode_isalpha(uint codepoint);			// 32-bit integer
int main()
{
int codepoint;

for (codepoint = 0; codepoint < 0x10FFFF; codepoint++)
	if (std.uni.isAlpha(codepoint))
		{
		if (!unicode_isalpha(codepoint))
			writefln("Failed (should be true) at Codepoint:0x%X", codepoint);
		}
	else
		if (unicode_isalpha(codepoint))
			writefln("Failed (should be false) at Codepoint:0x%X", codepoint);

return 0;
}
