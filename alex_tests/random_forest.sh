#!/bin/sh
#train rf
echo "Training ranger model with a csv input..."
./../cpp_version/bin/ranger --file samples/rf_test_to_csv.csv --depvarname CLOUD --treetype 1 --ntree 10 --write
#test rf
echo "Testing ranger model with a csv input..."
rm -rf ranger_out.prediction
./../cpp_version/bin/ranger --file samples/rf_test_to_csv.csv --predict ranger_out.forest
cp ranger_out.prediction csv_ranger_out.csv
#write cloud mask
echo "Training ranger model with an img input..."
./../cpp_version/bin/ranger --file samples/img_msec_1597559030639_2_thumbnail.jpeg --mask samples/img_msec_1597559030639_2_thumbnail.jpeg --depvarname CLOUD --treetype 1 --ntree 10 --write
#echo "Training ranger model with an img input and mismatched mask..."
#./../cpp_version/bin/ranger --file samples/img_msec_1597559030639_2_thumbnail.jpeg --mask samples/img_msec_1597559030639_2.png --depvarname CLOUD --treetype 1 --ntree 10 --write
#test rf
echo "Testing ranger model with an img input..."
rm -rf ranger_out.prediction
./../cpp_version/bin/ranger --file samples/img_msec_1597559030639_2_thumbnail.jpeg --predict ranger_out.forest --writetoimg --kernelsize 3
#train rf
echo "Training ranger model with a batch img input from Landsat..."
./../cpp_version/bin/ranger --batch --file train_png --mask train_mask_png --depvarname CLOUD --treetype 1 --probability --ntree 10 --write
echo "Testing with an img from OPS-SAT..."
./../cpp_version/bin/ranger --file samples/IMG_F9F8D255E10E-1.jpeg --predict ranger_out.forest --writetoimg


/home/exp185/bin/include/ranger/cpp_version/ranger --file /home/exp185/bin/rf_test_to_csv.csv --depvarname CLOUD --treetype 1 --ntree 10 --write --verbose
