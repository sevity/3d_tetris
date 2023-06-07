#!/bin/bash

# Change directory to the scripts directory
cd "$(dirname "$0")"

# Execute python scripts sequentially
python make_stage.py
python make_blocks.py
python trans0.py
python trans1.py

# Move the generated .obj files to the obj/ directory
rm *block.obj
mv *.obj ../obj/

