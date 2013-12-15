/*******************************************************************************/
/*                                                                             */
/* LGEO Libray Include File     (C) lgeo@digitalbricks.org (Lutz Uhlmann)      */
/*                                                                             */
/* 20080306 Lutz Uhlmann created shortcut to original part by Anton Raves      */
/*                                                                             */
/* This file is in no way related to the LEGO(tm) Group.                       */
/* It is provided for private non-commercial use only.                         */
/*                                                                             */
/* ar_2446: Brick 2 x 4                                                        */
/*                                                                             */ 
/*******************************************************************************/
#ifdef(ar_2446)
#else
#include "ar_material.inc"
#include "ar_library.inc"
#declare setting = 2;
#declare ar_2446 =
object {
 helmet(texture{})
 matrix <-0.1, 0, 0,
         0, 0, .1,
         0, 0.1, 0,
         0*LG_BRICK_WIDTH, -0*LG_BRICK_WIDTH, -1*LG_PLATE_HEIGHT>
 rotate <0, 0, 0>
}

#declare ar_2446_clear =
object {
 ar_2446
}
#end
