/*
 *  Switch class driver
 *
 * Copyright (C) 2008 Google, Inc.
 * Author: Mike Lockwood <lockwood@android.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
*/
#ifndef __LINUX_SWITCH_H__
#define __LINUX_SWITCH_H__
#define NAME_MAX_LEN 10
struct switch_dev {
	const char	*name;
	struct device	*dev;
	int		index;
	int		state;
	ssize_t	(*print_name)(struct switch_dev *sdev, char *buf);
	ssize_t	(*print_state)(struct switch_dev *sdev, char *buf);
};

extern int switch_dev_register(struct switch_dev *sdev);
extern void switch_dev_unregister(struct switch_dev *sdev);
static inline int switch_get_state(struct switch_dev *sdev)
{
	return sdev->state;
}
extern void switch_set_state(struct switch_dev *sdev, int state);
#endif
