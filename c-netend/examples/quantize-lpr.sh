
#
#
# Copyright (c) 2023 C-NET
# All rights reserved.
# Created by XiangWei-Zeng
#
#

./tools/quantize --param=models/plate/lpr-opt.param \
          --bin=models/plate/lpr-opt.bin \
          --images=plate-images/ \
          --output=models/plate/lpr-opt.table \
          --size=128,32 \
          --mean=116.407,133.722,124.187  \
          --norm=1,1,1 \
          --thread=8 \
          --swapRB

./tools/quantize --param=models/plate/lpc-opt.param \
          --bin=models/plate/lpc-opt.bin \
          --images=plate-images/ \
          --output=models/plate/lpc-opt.table \
          --size=110,22 \
          --mean=89.9372,81.1989,73.6352  \
          --norm=1,1,1 \
          --thread=8    \
          --swapRB

./nn-case lpr-opt.param lpr-opt.bin lpr-opt.h lpr-opt.c lpr-opt.table
./nn-case lpc-opt.param lpc-opt.bin lpc-opt.h lpc-opt.c lpc-opt.table
./nn-case plate-opt.param plate-opt.bin plate-opt.h plate-opt.c plate-opt.table
./nn-case yolo-v7-opt.param yolo-v7-opt.bin yolo-v7-opt.h yolo-v7-opt.c yolo-v7-opt.table
./nn-case crnn-opt.param crnn-opt.bin crnn-opt.h crnn-opt.c crnn-opt.table

./tools/quantize \
          --param=models/face/face-opt.param \
          --bin=models/face/face-opt.bin \
          --images=images/face-detector \
          --output=models/face/face-opt.table \
          --size=320,240 \
          --mean=104.f,117.f,123.f  \
          --norm=1,1,1 \
          --thread=8

./tools/quantize \
          --param=models/face/mobilefacenet-opt.param \
          --bin=models/face/mobilefacenet-opt.bin \
          --images=images/face-images \
          --output=models/face/mobilefacenet-opt.table \
          --size=112,112 \
          --mean=0,0,0 \
          --norm=1.0,1.0,1.0 \
          --thread=8

./nn-case face-opt.param face-opt.bin face-opt.h face-opt.c face-opt.table
./nn-case mobilefacenet-opt.param mobilefacenet-opt.bin mobilefacenet-opt.h mobilefacenet-opt.c mobilefacenet-opt.table