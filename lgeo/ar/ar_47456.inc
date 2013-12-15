/*******************************************************************************/
/*                                                                             */
/* LGEO Libray Include File     (C) lgeo@digitalbricks.org (Lutz Uhlmann)      */
/*                                                                             */
/* 20080217 Lutz Uhlmann created shortcut to original part by Anton Raves      */
/* 20080717 Lutz Uhlmann added missing semicolon                               */
/*                                                                             */
/* This file is in no way related to the LEGO(tm) Group.                       */
/* It is provided for private non-commercial use only.                         */
/*                                                                             */
/* ar_47456: Slope 2 x 3 x 2/3 with Flaps                                      */
/*                                                                             */ 
/*******************************************************************************/
#ifdef(ar_47456)
#else
#include "ar_material.inc"
#include "ar_library.inc"
#declare setting = 2;
#declare ar_47456 =
object {
 curved_f_slope2_3(texture{})
 matrix <-0.1, 0, 0,
         0, 0, .1,
         0, 0.1, 0,
         2*LG_BRICK_WIDTH, -0.5*LG_BRICK_WIDTH, -2*LG_PLATE_HEIGHT>
}

#declare ar_47456_clear =
object {
 ar_47456
}
#end
