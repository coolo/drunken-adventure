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
/* ar_3901: Minifig Hair Male                                                  */
/*                                                                             */ 
/*******************************************************************************/
#ifdef(ar_3901)
#else
#include "ar_material.inc"
#include "ar_library.inc"
#declare setting = 2;
#declare ar_3901 =
object {
 male_hair(texture{})
 matrix <-0.1, 0, 0,
         0, 0, .1,
         0, 0.1, 0,
         0, 0, -LG_PLATE_HEIGHT>
}

#declare ar_3001_clear =
object {
 ar_3901
}
#end
