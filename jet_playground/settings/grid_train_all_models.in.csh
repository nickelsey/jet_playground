#!/bin/csh

set arg = " bin/models/test_models.py --linear=True --forest=True --tree=True --knn=True --poly=3 --maxjobs=34"
qsub -V -q erhiq -lmem=72GB -lnodes=1:ppn=36 -o /wsu/home/dx/dx54/dx5412/analysis/jet_playground/build/log/train.log -e /wsu/home/dx/dx54/dx5412/analysis/jet_playground/build/log/train.err -N train_models -- /wsu/home/dx/dx54/dx5412/analysis/jet_playground/build/settings/qwrap.sh /wsu/home/dx/dx54/dx5412/analysis/jet_playground/build python $arg
