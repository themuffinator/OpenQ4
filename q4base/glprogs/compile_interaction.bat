@echo off
del interaction.vfp
cgc -quiet -profile arbvp1 cgvp_interaction.cg >> interaction.vfp
echo #====================================================================== >> interaction.vfp
cgc -quiet -profile arbfp1 cgfp_interaction.cg >> interaction.vfp
