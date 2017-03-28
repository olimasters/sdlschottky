#!/bin/bash

for x in {10..500..10}
	do
		./standard $x >> $1
	done
