#!/bin/bash

# Check if correct number of arguments are provided
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <program> <reference_dir>"
    exit 2
fi

PROGRAM="$1"
REFERENCE_DIR="$2"

# Run the program with --test 5
$PROGRAM --test 5
if [ $? -ne 0 ]; then
    echo "Error: Program failed to run."
    exit 1
fi

ERR=0

# Compare generated images with reference images
for i in {0000..0004}; do
    GENERATED="$1-$i.png"
    REFERENCE="$REFERENCE_DIR/$GENERATED"
    DIFF="$1-$i-diff.png"

    if [ ! -f "$REFERENCE" ]; then
        echo "Skipping compare: Reference image $REFERENCE not found!"
        continue
    fi

    if [ ! -f "$GENERATED" ]; then
        echo "Error: Expected generated image $GENERATED not found!"
        exit 1
    fi

    # Use ImageMagick's compare to check for differences
    compare -metric RMSE "$GENERATED" "$REFERENCE" "$DIFF" 2>&1 | grep -qE '[1-9]'  # Nonzero difference
    if [ $? -eq 0 ]; then
        echo "Difference found in $GENERATED!"
        echo "Run to visualize: display $DIFF"
        ERR=1
    else
        rm -f $GENERATED $DIFF
    fi
done

if [ $ERR -eq 0 ]; then
   echo "All images match the reference!"
   exit 0
else
   echo "Not all images match the reference!"
   exit 1
fi
