#!/bin/sh
echo "Training ranger model with a batch img input from Landsat..."
echo "Using 10 trees and a probability forest..."
./../cpp_version/bin/ranger --batch --file train_png_test --mask train_mask_png_test --depvarname CLOUD --treetype 1 --probability --ntree 10 --memmode 3 --savemem --maxdepth 18 --write
echo "Wrote random forest out to ranger_out.forest"
