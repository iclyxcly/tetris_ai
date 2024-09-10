#include "minotemplate.h"
#include "const.h"
#include "mino.hpp"
#include "utils.hpp"
int main()
{
	using namespace moenew;
	while (true)
		for (int m = 0; m < 7; ++m)
		{
			for (int r = 0; r < 4; r += 2)
			{
				for (int x = left_offset[m][r]; x < 29 - right_offset[m][r]; ++x)
				{
					const auto &mino = cache_get(m, r, x);
					for (int y = 3; y >= 0; y--)
					{
						printf("|");
						for (int i = 0; i < 32; ++i)
						{
							printf("%s", mino[y] & (1 << i) ? "[]" : "  ");
						}
						printf("|\n");
					}
					printf("\n");
					Sleep(10);
				}
				for (int x = 28 - right_offset[m][r + 1]; x >= 0; --x)
				{
					const auto &mino = cache_get(m, r + 1, x);
					for (int y = 3; y >= 0; y--)
					{
						printf("|");
						for (int i = 0; i < 32; ++i)
						{
							printf("%s", mino[y] & (1 << i) ? "[]" : "  ");
						}
						printf("|\n");
					}
					printf("\n");
					Sleep(10);
				}
			}
		}
}