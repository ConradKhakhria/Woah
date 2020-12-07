/* This links all of the parsing stuff together */
#pragma once

#include "../datastructures/datastructures.h"

int traverse_block(Array prog, int start, int end, int open, int close);

int collect_blocks(Array tokens, Array* blocks);
