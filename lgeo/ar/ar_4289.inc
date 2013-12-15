/*******************************************************************************/
/*                                                                             */
/* LGEO Libray Include File     (C) lgeo@digitalbricks.org (Lutz Uhlmann)      */
/*                                                                             */
/* 20071104 Lutz Uhlmann created shortcut to original part by Anton Raves      */
/* 20080717 Lutz Uhlmann added missing semicolon                               */
/*                                                                             */
/* This file is in no way related to the LEGO(tm) Group.                       */
/* It is provided for private non-commercial use only.                         */
/*                                                                             */
/* ar_4289: Boat Mast 2 x 2                                                    */
/*                                                                             */ 
/*******************************************************************************/
#ifdef(ar_4289)
#else
#include "ar_material.inc"
#include "ar_library.inc"
#declare setting = 2;
#declare ar_4289 =
object {
 boat_mast(texture{})
 matrix <0, 0.1, 0,
         0, 0, .1,
         -.1, 0, 0,
         LG_BRICK_WIDTH/2, -LG_BRICK_WIDTH/2, -LG_PLATE_HEIGHT>
}

#declare ar_4289_clear =
object {
 ar_4289
}
#end
