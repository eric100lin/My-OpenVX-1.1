
#include <vx_cl.h>

#if defined(CL_USE_IMAGES)

__kernel void vx_single_threshold(read_only image2d_t in, uchar value, write_only image2d_t out) {
    int2 coord = (int2)(get_global_id(0), get_global_id(1));
    uint4 a = read_imageui(in, nearest_clamp, coord);
    uint4 b = clamp(a, value-1u, 255u);
    uint4 c = 0u;
}

#else

__kernel void vx_single_threshold
	(int aw, int ah, int asx, int asy, __global void *a, 
	 uchar threshold,
	 int bw, int bh, int bsx, int bsy, __global void *b)
{
    int x = get_global_id(0);
    int y = get_global_id(1);
	uchar a_pixel = vxImagePixel(uchar, a, x, y, asx, asy);
	if(a_pixel > threshold)
		vxImagePixel(uchar, b, x, y, bsx, bsy) = 255;
	else
		vxImagePixel(uchar, b, x, y, bsx, bsy) = 0;
}

#endif
