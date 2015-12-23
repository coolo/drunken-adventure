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

void ocr(tinycv::Image self)
  CODE:
    image_ocr(self);
