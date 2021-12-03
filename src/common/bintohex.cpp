

void BinToHex(const char *data, int length, char *hex_buff)
{
	static const char *hex_map = "0123456789ABCDEF";

	const char *data_end = data + length;
	while (data < data_end)
	{
		unsigned char c = *data++;
		*hex_buff++ = hex_map[(c & 0xf0) >> 4];
		*hex_buff++ = hex_map[c & 0x0f];
	}
}


void HexToBin(const char *hex_buff, int length, char *data)
{
	static const unsigned char bin_map[256] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,1,2,3,4,5,6,7,8,9,0,0,0,0,0,0,0,10,11,12,13,14,15};

	const char *hex_end = hex_buff + length;
	while (hex_buff < hex_end)
	{
		unsigned char c = 0;
		c |= bin_map[int(*hex_buff++)] << 4;
		*data++ = c | bin_map[int(*hex_buff++)];
	}
}

