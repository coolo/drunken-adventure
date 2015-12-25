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

int troop_count(tinycv::Image self)
  CODE:
    RETVAL = image_troop_count(self);

  OUTPUT:
    RETVAL
