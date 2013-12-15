/*******************************************************************************/
/*                                                                             */
/* LGEO Libray Include File     (C) lgeo@digitalbricks.org (Lutz Uhlmann)      */
/*                                                                             */
/* 20080512 Lutz Uhlmann created shortcut to original part by Anton Raves      */
/* 20080717 Lutz Uhlmann added missing semicolon                               */
/*                                                                             */
/* This file is in no way related to the LEGO(tm) Group.                       */
/* It is provided for private non-commercial use only.                         */
/*                                                                             */
/* ar_3834: Minifig Fire Helmet                                                */
/*                                                                             */ 
/*******************************************************************************/
#ifdef(ar_3834)
#else
#include "ar_material.inc"
#include "ar_library.inc"
#declare setting = 2;
#declare ar_3834 =
object {
 helmet_2(texture{})
 matrix <-0.1, 0, 0,
         0, 0, .1,
         0, 0.1, 0,
         0, 0, -LG_PLATE_HEIGHT>
 rotate <0, 0, 90>
}

#declare ar_3001_clear =
object {
 ar_3834
}
#end
