@echo off
setlocal

echo "Extract Gxx Files"
set /P DST_DIRNAME="output dir name:"
mkdir %DST_DIRNAME%
echo "Top layer:*.GTL"
move *.GTL %DST_DIRNAME%
echo "Bottom layer:*.GBL"
move *.GBL %DST_DIRNAME%
echo "Solder Stop Mask Top:*.GTS"
move *.GTS %DST_DIRNAME%
echo "Solder Stop Mask Bottom:*.GBS"
move *.GBS %DST_DIRNAME%
echo "Silk Top:*.GTO"
move *.GTO %DST_DIRNAME%
echo "Silk Bottom:*.GBO"
move *.GBO %DST_DIRNAME%
echo "NC Drill:*.TXT"
move *.TXT %DST_DIRNAME%
echo "Mechanical layer :*.GML"
move *.GML %DST_DIRNAME%
 
echo "end."
pause