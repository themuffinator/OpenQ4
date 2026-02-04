@echo off
del post_cas.vfp
cgc -quiet -profile arbvp1 cgvp_cas.cg >> post_cas.vfp
echo #====================================================================== >> post_cas.vfp
cgc -quiet -profile arbfp1 cgfp_cas.cg >> post_cas.vfp

