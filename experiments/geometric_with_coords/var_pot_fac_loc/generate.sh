#!/usr/bin/env bash

${FCLA_ROOT}/bin/generator -n 5000 -o ${DATA_PATH}/geometric_with_coords/var_pot_fac_loc/ -g 0 -u 1 -s 100 --density 2
echo "Generation done."
