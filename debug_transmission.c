#include <stdio.h>
#include <unistd.h>
#include <signal.h>

void test_bit_mapping(unsigned int value)
{
    printf("\n=== Testing value %u (0x%x) ===\n", value, value);

    // Show expected bit pattern
    printf("Expected bits: ");
    for (int i = 31; i >= 0; i--)
    {
        if (value & (1U << i))
        {
            printf("bit_%d ", i);
        }
    }
    printf("\n");

    // Simulate client transmission (MSB first)
    printf("Client sends (MSB first):\n");
    for (int i = 31; i >= 0; i--)
    {
        int bit = (value & (1U << i)) ? 1 : 0;
        if (i <= 7 || bit == 1)
        { // Show last 8 bits or any 1 bits
            printf("  bit_pos_%d: %d -> %s\n", i, bit, bit ? "SIGUSR2" : "SIGUSR1");
        }
    }

    // Simulate server reception
    printf("Server should receive:\n");
    unsigned int reconstructed = 0;
    for (int sig_count = 0; sig_count < 32; sig_count++)
    {
        int bit_position = (32 - 1) - sig_count; // Server's calculation
        int client_bit_pos = 31 - sig_count;     // Client sends MSB first
        int bit_value = (value & (1U << client_bit_pos)) ? 1 : 0;

        if (bit_value == 1)
        {
            reconstructed |= (1U << bit_position);
        }

        if (client_bit_pos <= 7 || bit_value == 1)
        {
            printf("  sig_count=%d: bit_pos=%d, bit_value=%d, reconstructed=0x%x\n",
                   sig_count, bit_position, bit_value, reconstructed);
        }
    }

    printf("Final reconstructed value: %u (0x%x)\n", reconstructed, reconstructed);
    printf("Expected: %u (0x%x)\n", value, value);
    printf("Match: %s\n", (reconstructed == value) ? "YES" : "NO");
}

int main()
{
    printf("Debugging bit transmission patterns\n");
    printf("====================================\n");

    test_bit_mapping(3);
    test_bit_mapping(4);
    test_bit_mapping(5);

    // Test the problematic case we saw
    printf("\n=== Problem Analysis ===\n");
    printf("Client sends 3 (0x3), server receives 6 (0x6)\n");
    printf("3 has bits: 1, 0\n");
    printf("6 has bits: 2, 1\n");
    printf("This is a 1-bit LEFT SHIFT!\n");

    return 0;
}
