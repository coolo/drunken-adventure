/*******************************************************************************/
/*                                                                             */
/* LGEO Libray Include File     (C) lgeo@digitalbricks.org (Lutz Uhlmann)      */
/*                                                                             */
/* 20080306 Lutz Uhlmann created shortcut to original part by Anton Raves      */
/* 20080717 Lutz Uhlmann added missing semicolon                               */
/*                                                                             */
/* This file is in no way related to the LEGO(tm) Group.                       */
/* It is provided for private non-commercial use only.                         */
/*                                                                             */
/* ar_3020: Plate 2 x 4                                                        */
/*                                                                             */ 
/*******************************************************************************/
#ifdef(ar_3020)
#else
#include "ar_material.inc"
#include "ar_library.inc"
#declare setting = 2;
#declare ar_3020 =
object {
 f_block2_4(texture{})
 matrix <-0.1, 0, 0,
         0, 0, .1,
         0, 0.1, 0,
         1.5*LG_BRICK_WIDTH, -0.5*LG_BRICK_WIDTH, -LG_PLATE_HEIGHT>
 rotate <0, 0, 90>
}

#declare ar_3020_clear =
object {
 ar_3020
}
#end
