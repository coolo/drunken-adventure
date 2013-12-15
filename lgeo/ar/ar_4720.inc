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
/* ar_4720: Minifig Bicycle Wheel                                              */
/*                                                                             */ 
/*******************************************************************************/
#ifdef(ar_4720)
#else
#include "ar_material.inc"
#include "ar_library.inc"
#declare setting = 2;
#declare ar_4720 =
object {
 bicycle_wheel(texture{lg_clear})
 matrix <-0.1, 0, 0,
         0, 0, .1,
         0, 0.1, 0,
         0, 0, 0>
}

#declare ar_4720_clear =
object {
 ar_4720
}

#end
