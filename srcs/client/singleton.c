/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   singleton.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: codespace <codespace@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/03 02:17:43 by codespace         #+#    #+#             */
/*   Updated: 2025/07/03 17:11:14 by codespace        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "client.h"

t_server_state	*get_server_instance(void)
{
	static t_server_state	server = {0, 0, 0, 0, 0, 0, 0, 0};

	return (&server);
}
