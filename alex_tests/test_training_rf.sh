#!bin/sh
./../cpp_version/bin-arm-mfpu/ranger --file train_png_test --mask train_mask_png_test --depvarname CLOUD --treetype 1 --probability --ntree 10 --memmode 3 --maxdepth 18 --write --verbose --batch --savemem
#./../cpp_version/bin-ccc-arm/ranger --file test.csv --mask test.csv --depvarname CLOUD --treetype 1 --probability --ntree 10 --memmode 2 --savemem --maxdepth 18 --write --verbose
#./../cpp_version/bin-ccc-arm/ranger --file samples/img_msec_1597559030639_2_thumbnail.jpeg --mask samples/img_msec_1597559030639_2_thumbnail.jpeg --depvarname CLOUD --treetype 1 --probability --ntree 10 --memmode 3 --savemem --maxdepth 18 --write --verbose
