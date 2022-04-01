#!/bin/sh
mkdir em_test
chmod x em_test
#train rf
echo "Training ranger model with a csv input..."
./cpp_version/bin/ranger --file samples/rf_test_to_csv.csv --depvarname CLOUD --treetype 1 --ntree 10 --write
#test rf
echo "Testing ranger model with a csv input..."
./cpp_version/bin/ranger --file samples/rf_test_to_csv.csv --predict ranger_out.forest
#write cloud mask
echo "Training ranger model with an img input..."
./cpp_version/bin/ranger --file samples/img_msec_1597559030639_2_thumbnail.jpeg --mask samples/img_msec_1597559030639_2_thumbnail.jpeg --depvarname CLOUD --treetype 1 --ntree 10 --write
#test rf
echo "Testing ranger model with an img input..."
./cpp_version/bin/ranger --file samples/img_msec_1597559030639_2_thumbnail.jpeg --predict ranger_out.forest
