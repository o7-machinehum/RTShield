#!/bin/bash

VER="r2"
PRJ="blocky_breakout_pcb"
PRJDIR="pcb"

cd $PRJDIR
mkdir out/$VER  2>/dev/null
rm -r out/$VER/ 2>/dev/null
mkdir out/$VER/ 2>/dev/null
mkdir out/$VER/fab/ 2>/dev/null

kikit fab pcbway $PRJ.kicad_pcb --schematic $PRJ.kicad_sch --assembly out/$VER/fab/
kicad-cli sch export pdf $PRJ.kicad_sch -o out/$VER/$PRJ.pdf
kicad-cli pcb export step $PRJ.kicad_pcb -o out/$VER/$PRJ.step

rm out/$VER/fab/gerber.zip
cd out/$VER/

OUT=$PRJ\_$VER.zip
zip -r $OUT fab/*
echo Created release: $VER $OUT
