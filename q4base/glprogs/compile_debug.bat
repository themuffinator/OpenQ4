@echo off
del interaction.vfp
cgc  -profile arbvp1 cgvp_interaction.cg
echo #====================================================================== >> interaction.vfp
cgc -profile arbfp1 cgfp_interaction.cg 
pause