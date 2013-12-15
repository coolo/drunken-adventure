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
/* ar_41854: Car Mudguard 2 x 4 Swept Back                                     */
/*                                                                             */ 
/*******************************************************************************/
#ifdef(ar_41854)
#else
#include "ar_material.inc"
#include "ar_library.inc"
#declare setting = 2;
#declare ar_41854 =
object {
 curved_wheelcover2_2(texture{})
 matrix <-0.1, 0, 0,
         0, 0, .1,
         0, 0.1, 0,
         0.5*LG_BRICK_WIDTH, -0.5*LG_BRICK_WIDTH, -LG_PLATE_HEIGHT>
}

#declare ar_41854_clear =
object {
 ar_41854
}
#end
