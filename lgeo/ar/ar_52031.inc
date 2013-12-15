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
/* ar_52031: Wedge 4 x 6 x 2/3 Curved                                          */
/*                                                                             */ 
/*******************************************************************************/
#ifdef(ar_52031)
#else
#include "ar_material.inc"
#include "ar_library.inc"
#declare setting = 2;
#declare ar_52031 =
object {
 curved_f_slope4_6(texture{})
 matrix <-0.1, 0, 0,
         0, 0, .1,
         0, 0.1, 0,
         3*LG_BRICK_WIDTH, -2.5*LG_BRICK_WIDTH, -2*LG_PLATE_HEIGHT>
}

#declare ar_52031_clear =
object {
 ar_52031
}
#end
