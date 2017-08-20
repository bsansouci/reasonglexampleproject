/***********************************************************************/
/*                                                                     */
/*                           CamlImages                                */
/*                                                                     */
/*                          Jun Furuse                                 */
/*                                                                     */
/*  Copyright 1999-2013                                                */
/*  Institut National de Recherche en Informatique et en Automatique.  */
/*  Distributed only by permission.                                    */
/*                                                                     */
/***********************************************************************/

#include <caml/mlvalues.h>
#include <caml/alloc.h>
#include <caml/memory.h>
#include <caml/fail.h>

#define NA(x) value x(){ failwith("unsupported"); }

NA(Val_ExifBytes)
NA(Val_ExifSBytes)
NA(Val_ExifShorts)
NA(Val_ExifSShorts)
NA(Val_ExifLongs)
NA(Val_ExifSLongs)
NA(Val_ExifRationals)
NA(Val_ExifSRationals)
NA(Val_ExifFloats)
NA(Val_ExifDoubles)
NA(caml_exif_tag_get_name_in_ifd)
NA(caml_val_exif_data)
NA(caml_exif_set_byte_order)
NA(caml_exif_get_byte_order)
NA(caml_exif_data_fix)
NA(caml_exif_data_unref)
NA(caml_exif_data_dump)
NA(caml_exif_data_contents)
NA(caml_exif_content_unref)
NA(caml_exif_content_entries)
NA(caml_exif_entry_unref)
NA(caml_exif_decode_entry)
