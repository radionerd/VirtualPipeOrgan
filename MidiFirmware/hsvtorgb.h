#ifndef __HSVTORGB_H
#define __HSVTORGB_H

#ifdef __cplusplus
extern "C" {
#endif



typedef struct RGBColor {
	int r;
	int g;
	int b;
} RGBColor;

RGBColor hsv2rgb(float H, float S, float V) ;

#ifdef __cplusplus
}
#endif


#endif /* __HSVTORGB_H */
