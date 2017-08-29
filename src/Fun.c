#include <stdio.h>
#include <string.h>
#include <math.h>

#include <caml/alloc.h>
#include <caml/custom.h>
#include <caml/memory.h>
#include <caml/mlvalues.h>
#include <caml/fail.h>
#include <caml/bigarray.h>

void unsafe_blit(value arr1, value arr2, value offset, value numOfBytes) {
  char *arr1Data = Caml_ba_data_val(arr1);
  char *arr2Data = Caml_ba_data_val(arr2);
  memcpy(arr2Data + Int_val(offset) * Int_val(numOfBytes), arr1Data, caml_ba_byte_size(Caml_ba_array_val(arr1)));
}

void unsafe_update_float32(value arr, value index, value mul, value add) {
  float *data = Caml_ba_data_val(arr);
  data[Int_val(index)] = floor(data[Int_val(index)] * Double_val(mul) + Double_val(add));
}

void unsafe_update_uint16(value arr, value index, value val) {
  int16_t *data = Caml_ba_data_val(arr);
  data[Int_val(index)] = data[Int_val(index)] + (int16_t)Int_val(val);
}

CAMLprim value caml_rdtsc( )
{
    unsigned hi, lo;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
    return Val_int( ((unsigned long long)lo)|( ((unsigned long long)hi)<<32 ));
}
