/* tapecart - a tape port storage pod for the C64

   Copyright (C) 2013-2017  Ingo Korb <ingo@akana.de>
   All rights reserved.
   Idea by enthusi

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
   FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
   OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
   OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
   SUCH DAMAGE.


   conutil.c: a few convenient routines for a text mode user interface

*/

#include <conio.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "conutil.h"

/*
 * DEL  : 0x14 (20)
 * Ins  : 0x94 (148)
 * Up   : 0x91 (145)
 * Down : 0x11 (17)
 * Left : 0x9d (157)
 * Right: 0x1d (29)
 * RET  : 0x0d (13)
 * Stop : 0x03 (3)
 */

#define KEY_DEL    0x14
#define KEY_INS    0x94
#define KEY_UP     0x91
#define KEY_DOWN   0x11
#define KEY_LEFT   0x9d
#define KEY_RIGHT  0x1d
#define KEY_RETURN 0x0d
#define KEY_STOP   0x03

void read_string(char *buffer, unsigned char maxlen,
                 unsigned char xpos, unsigned char ypos) {
  unsigned char pos;
  unsigned char i;
  char ch;
  char curs_state;
  
  curs_state = cursor(1);
  cputsxy(xpos, ypos, buffer);
  pos = strlen(buffer);
  gotoxy(xpos + pos, ypos);

  while (1) {
    ch = cgetc();
    if (isprint(ch)) {
      /* add to string if possible */
      if (pos < maxlen) {
        buffer[pos] = ch;
        pos++;
        cputc(ch);
      }
    } else {
      switch (ch) {
      case KEY_LEFT:
        if (pos > 0) {
          --pos;
          gotoxy(xpos + pos, ypos);
        }
        break;

      case KEY_RIGHT:
        if (pos < maxlen && buffer[pos] != 0) {
          ++pos;
          gotoxy(xpos + pos, ypos);
        }
        break;

      case KEY_RETURN:
        cursor(curs_state);
        return;

      case KEY_DEL:
        if (pos > 0) {
          gotoxy(xpos + pos - 1, ypos);
          /* move remainder of string back one char */
          for (i = pos; i <= maxlen; ++i) {
            buffer[i-1] = buffer[i];
            if (buffer[i])
              cputc(buffer[i]);
            else
              cputc(' ');
          }
          --pos;
          gotoxy(xpos + pos, ypos);
        }
        break;
      }
    }
  }
}

static char hexbuffer[12];

uint32_t read_uint(uint32_t preset, unsigned char width,
                   unsigned char xpos, unsigned char ypos) {
  unsigned char pos;
  char ch;
  char curs_state;

  curs_state = cursor(1);
  sprintf(hexbuffer, "$%0*lx", width - 1, preset);
  cputsxy(xpos, ypos, hexbuffer);
  pos = 1;
  gotoxy(xpos + pos, ypos);

  while (1) {
    ch = cgetc();
    switch (ch) {
    case KEY_DEL:
    case KEY_LEFT:
      if (pos > 0) {
        --pos;
        gotoxy(xpos + pos, ypos);
      }
      break;

    case KEY_RIGHT:
      if (pos < width && hexbuffer[pos] != 0) {
        ++pos;
        gotoxy(xpos + pos, ypos);
      }
      break;

    case KEY_RETURN:
      cursor(curs_state);
      if (hexbuffer[0] == '$') {
        return strtol(hexbuffer+1, NULL, 16);
      } else {
        return strtol(hexbuffer, NULL, 10);
      }

    default:
      if (pos == 0) {
        if (ch == '$' || isdigit(ch)) {
          hexbuffer[0] = ch;
          ++pos;
          cputc(ch);
        }
      } else {
        if ((hexbuffer[0] == '$' && isxdigit(ch)) ||
            (hexbuffer[0] != '$' && isdigit(ch))  ||
            ch == ' ') {
          hexbuffer[pos] = ch;
          ++pos;
          cputc(ch);
        }
      }
      break;

    }
  }
}


unsigned char show_menu(unsigned char count, const char **items, unsigned char sel,
                        unsigned char xpos, unsigned char ypos) {
  unsigned char i;
  char ch;

  while (1) {
    /* print menu */
    for (i = 0; i < count; ++i) {
      revers(i == sel);
      cputsxy(xpos, ypos + i, items[i]);
      revers(0);
    }

    ch = cgetc();

    /* numeric key: direct selection */
    if (ch >= '1' && ch <= '9') {
      i = ch - '0';
      if (i <= count)
        return i-1;
      else continue;
    }

    switch (ch) {
    case KEY_RETURN:
      return sel;

    case KEY_UP:
      if (sel == 0)
        sel = count - 1;
      else
        sel = sel - 1;
      break;

    case KEY_DOWN:
      if (sel >= count - 1)
        sel = 0;
      else
        sel = sel + 1;
      break;
    }
  }
}

void wait_key(void) {
  (void)cgetc();
}

void clear_mainarea(void) {
  unsigned int i;

  for (i = 2; i < STATUS_START - 2; ++i)
    cclearxy(0, i, 40);
}

void clear_mainarea_full(void) {
  unsigned int i;

  for (i = 2; i < STATUS_START; ++i)
    cclearxy(0, i, 40);
}

