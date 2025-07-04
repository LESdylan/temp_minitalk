#include <stdio.h>

int main()
{
    unsigned int value = 3; // Binary: 00000011
    printf("Value: %u (0x%x)\n", value, value);
    printf("Expected bits that should be 1: ");
    for (int i = 31; i >= 0; i--)
    {
        if (value & (1U << i))
        {
            printf("bit_%d ", i);
        }
    }
    printf("\n");

    printf("Binary representation: ");
    for (int i = 31; i >= 0; i--)
    {
        printf("%d", (value & (1U << i)) ? 1 : 0);
        if (i % 8 == 0 && i > 0)
            printf(" ");
    }
    printf("\n");

    // Simulate what the server should receive
    printf("\nServer should receive for value 3:\n");
    printf("sig_count=30 -> bit_pos=1, bit_value=1\n");
    printf("sig_count=31 -> bit_pos=0, bit_value=1\n");
    printf("All other sig_counts -> bit_value=0\n");

    return 0;
}
