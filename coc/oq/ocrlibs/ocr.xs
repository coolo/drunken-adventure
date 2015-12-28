#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#include "../ppmclibs/tinycv.h"
#include "ocr.h"

typedef Image *tinycv__Image;

MODULE = ocr     PACKAGE = tinycv::Image  PREFIX = Image

void chat_ocr(tinycv::Image self)
  CODE:
    image_chat_ocr(self);

int troop_count(tinycv::Image self, const char *filename)
  CODE:
    RETVAL = image_troop_count(self, filename);

  OUTPUT:
    RETVAL

int base_count(tinycv::Image self, const char *filename)
  CODE:
    RETVAL = image_base_count(self, filename);

  OUTPUT:
    RETVAL

void find_red_line(tinycv::Image self)
   PPCODE:
    std::vector<int> res = image_find_red_line(self);
    EXTEND(SP, 4);
    PUSHs(sv_2mortal(newSViv(res[0])));
    PUSHs(sv_2mortal(newSViv(res[1])));
    PUSHs(sv_2mortal(newSViv(res[2])));
    PUSHs(sv_2mortal(newSViv(res[3])));
