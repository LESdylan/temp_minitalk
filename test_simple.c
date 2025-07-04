#include <stdio.h>
#include <unistd.h>
#include <signal.h>

int main()
{
    unsigned int value = 4; // Test value

    printf("Testing value %u (0x%x)\n", value, value);
    printf("Binary representation: ");

    for (int i = 31; i >= 0; i--)
    {
        int bit = (value & (1U << i)) ? 1 : 0;
        printf("%d", bit);
        if (i % 8 == 0 && i > 0)
            printf(" ");
    }
    printf("\n");

    // Show which bits should be 1
    printf("Bits that should be 1: ");
    for (int i = 31; i >= 0; i--)
    {
        if (value & (1U << i))
        {
            printf("bit_%d ", i);
        }
    }
    printf("\n");

    return 0;
}
