/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   checksum.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlesieur <dlesieur@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/03 16:19:08 by codespace         #+#    #+#             */
/*   Updated: 2025/07/04 11:47:39 by dlesieur         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "client.h"

int calculate_checksum(const char *data, int length)
{
	unsigned int checksum;
	int i;

	checksum = 0x5A5A5A5A; // Use a non-zero seed to avoid trivial checksums
	i = 0;
	while (i < length)
	{
		checksum ^= (unsigned char)data[i];
		// Use a simpler rotation to avoid overflow issues
		checksum = ((checksum << 3) | (checksum >> 29)) ^ (i & 0xFF);
		i++;
	}
	// Ensure we don't return -1 by masking off the sign bit if needed
	if ((int)checksum == -1)
		checksum = 0x7FFFFFFF;
	return ((int)checksum);
}
