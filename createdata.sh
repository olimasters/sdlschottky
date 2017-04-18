#!/bin/bash

for x in {10..500..10}
	do
		./$1 $x >> $2
	done
