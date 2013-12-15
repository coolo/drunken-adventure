/*******************************************************************************/
/*                                                                             */
/* LGEO Libray Include File     (C) lgeo@digitalbricks.org (Lutz Uhlmann)      */
/*                                                                             */
/* 20080210 Lutz Uhlmann created shortcut to original part by Anton Raves      */
/* 20080717 Lutz Uhlmann added missing semicolon                               */
/*                                                                             */
/* This file is in no way related to the LEGO(tm) Group.                       */
/* It is provided for private non-commercial use only.                         */
/*                                                                             */
/* ar_4719: Minifig Bicycle Body                                               */
/*                                                                             */ 
/*******************************************************************************/
#ifdef(ar_4719)
#else
#include "ar_material.inc"
#include "ar_library.inc"
#declare setting = 2;
#declare ar_4719 =
object {
 bicycle_frame(texture{})
 matrix <-0.1, 0, 0,
         0, 0, .1,
         0, 0.1, 0,
         0, 0, -1.5*LG_BRICK_WIDTH>
}

#declare ar_4719_clear =
object {
 ar_4719
}

#end
