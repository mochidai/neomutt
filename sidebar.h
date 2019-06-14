/**
 * @file
 * GUI display the mailboxes in a side panel
 *
 * @authors
 * Copyright (C) 2004 Justin Hibbits <jrh29@po.cwru.edu>
 * Copyright (C) 2004 Thomer M. Gil <mutt@thomer.com>
 * Copyright (C) 2015-2016 Richard Russon <rich@flatcap.org>
 *
 * @copyright
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MUTT_SIDEBAR_H
#define MUTT_SIDEBAR_H

#include <stdbool.h>

struct ConfigSet;
struct Mailbox;

void mutt_sb_change_mailbox(int op);
void mutt_sb_draw(void);
struct Mailbox *mutt_sb_get_highlight(void);
void mutt_sb_notify_mailbox(struct Mailbox *m, bool created);
void mutt_sb_set_open_mailbox(void);
bool init_sidebar(struct ConfigSet *cs);

#endif /* MUTT_SIDEBAR_H */
