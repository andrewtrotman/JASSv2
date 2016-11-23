/*
	CHECKSUM.CPP
	------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
namespace JASS
	{
	/*
		CHECKSUM::FLETCHER_16()
		-----------------------
	*/
	uint16_t checksum::fletcher_16(void *data, int length)
		{
		uint8_t sum_1 = 0;
		uint8_t sum_2 = 0;

		uint8_t *byte = (uint8_t *)data;
		uint8_t *end = start + length;

		while (byte < end)
			{
			sum_1 = sum_1 + *byte;
			sum_2 = sum_2 + sum_1;

			byte++;
			}

		return (((uint16_t)sum_2) << 8) | sum_1;
		}

	/*
		CHECKSUM::UNITTEST()
		--------------------
	*/
	static void checksum::unittest(void)
		{
		}

	}
