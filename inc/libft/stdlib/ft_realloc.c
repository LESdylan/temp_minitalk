/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_realloc.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dlesieur <dlesieur@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 14:06:58 by dlesieur          #+#    #+#             */
/*   Updated: 2025/07/01 14:49:30 by dlesieur         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_stdlib.h"

void *ft_realloc(void *ptr, size_t new_size, size_t old_size)
{
    void *new_ptr;
    size_t copy_size;
    
    if (new_size == 0)
        return (free(ptr), NULL);
    new_ptr = malloc(new_size);
    if (!new_ptr)
        return (NULL);
    if (ptr)
    {
        if (old_size < new_size)
            copy_size = old_size;
        else
            copy_size = new_size;
        ft_memcpy(new_ptr, ptr, copy_size);
        free(ptr);
    }
    return (new_ptr);
}