#include <vx_cl.h>

//#if !defined(__IMAGE_SUPPORT__) // && defined(CL_USE_LUMINANCE)

__kernel void vx_add_u8_truncate
	(int aw, int ah, int asx, int asy, __global uchar *a, 
	 int bw, int bh, int bsx, int bsy, __global uchar *b,
	 int spolicy,
	 int cw, int ch, int csx, int csy, __global uchar *c)
{
    int x = get_global_id(0);
    int y = get_global_id(1);
    vxImagePixel(uchar, c, x, y, csx, csy) = 
        vxImagePixel(uchar, a, x, y, asx, asy) + vxImagePixel(uchar, b, x, y, bsx, bsy);
}

//#endif