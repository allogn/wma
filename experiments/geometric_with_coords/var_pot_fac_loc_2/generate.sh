#!/usr/bin/env bash

${FCLA_ROOT}/bin/generator -n 10000 -o ${DATA_PATH}/geometric_with_coords/var_pot_fac_loc_2/ -g 0 -s 2000 -u 1 --density 2
echo "Generation done."
