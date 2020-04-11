#!/bin/bash

fa_symbols=("0xF015" "0xF043"  "0xF2CD" "0xf7d8" "0xf0eb")

fa_range=""

for sym in ${fa_symbols[@]}; do
  fa_range+=" -r $sym "
done
echo $fa_range

lv_font_conv --font Roboto-Regular.ttf -r 0x20-0x7F --font fa-solid-900.ttf  $fa_range --size 12  --bpp 4 --format lvgl -o font_12.c
lv_font_conv --font Roboto-Regular.ttf -r 0x20-0x7F --font fa-solid-900.ttf  $fa_range --size 16  --bpp 4 --format lvgl -o font_16.c
lv_font_conv --font Roboto-Regular.ttf -r 0x20-0x7F --font fa-solid-900.ttf  $fa_range --size 22  --bpp 4 --format lvgl -o font_22.c
lv_font_conv --font Roboto-Regular.ttf -r 0x20-0x7F --font fa-solid-900.ttf  $fa_range --size 28  --bpp 4 --format lvgl -o font_28.c

