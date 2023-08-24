#include <ctype.h>  // isdigit
#include <math.h>   // M_PI, sin, cos
#include <stdbool.h>// true
#include <stdio.h>  // printf
#include <stdlib.h> // abs
#include <string.h> // strlen
// (c)2023 Richard Jones MIT License
// This program generates dxf files for a 3 manual organ console and music stand
// The keyboards used are M-Audio Keystation 61 Mk3
// The material used is ~9mm MDF
//
// Linear Dimensions in mm
// Angles in degrees

#define CONSTRUCTION_LINES_OFF // Toggle OFF/ON as required

float CutLength = 0; // Accumulate lenght of cut as we go

void preamble( void ){
  printf("%s", "\
0\n\
SECTION\n\
2\n\
ENTITIES\n" );
}

void postamble( void ){
  printf("%s", "\
0\n\
ENDSEC\n\
0\n\
EOF\n" );
}

const int BYBLOCK = 0;
const int BLACK = 0; 
const int RED   = 1;
const int YELLOW= 2;
const int GREEN = 3;
const int CYAN  = 4;
const int BLUE  = 5;
const int MAGENTA=6;
const int WHITE = 7;
const int DEFAULT_COLOUR = WHITE;
const int BYLAYER=256;

int colour = DEFAULT_COLOUR;
void setColour( int hue ) {
  colour = hue;
}

void line( float x1,float y1,float x2,float y2) {
  printf( "0\nLINE\n10\n%f\n20\n%f\n11\n%f\n21\n%f\n62\n%d\n",
  x1,y1,x2,y2,colour);
  CutLength += fabsf(x1-x2) + fabsf(y1-y2); // correct result for vertical or horizontal lines
  // fprintf( stderr, "line_colour: x1=%f y1=%f  x2=%f y2=%f colour=%d CutLength=%f\n",x1,y1,x2,y2,colour,CutLength);
}

void rectangle (float x1,float y1,float x2,float y2) {
  line( x1,y1, x1,y2);
  line( x1,y2, x2,y2);
  line( x2,y2, x2,y1);
  line( x2,y1, x1,y1);
}
// Draw toothy finger joint line
// FLAGS
const int DOUBLE_START = 0x100;	// default single
const int DOUBLE_END   = 0x200;	// default single
const int HORIZONTAL   =     0;	// default horizontal
const int VERTICAL     = 0x400; // default horizontal
const int CENTRE_SKIP  = 0x800; // default all gaps
const int FLAG_MASK    = 0x0FF; // middle finger adj mm 127>0>-128
// Set gap +ve or negtive for line direction
// set thickness +ve or negative to choose which side the fingers point
void fingerJoint( float x1,float y1, float gap,float thickness, int nfingers,int flags ) {
 float x2;
 float y2;
 int count = 0;
 int adjust = flags & FLAG_MASK ;
 if ( adjust > FLAG_MASK/2 )
   adjust -= ( FLAG_MASK + 1 ) ;
 for ( int i = 0 ; i < nfingers ; ++i ) {
   if ( flags & VERTICAL ) {
   // Vertical
    x2 = x1 + thickness;
    y2 = y1 + gap;
    if ( flags & DOUBLE_START || ( ( i == ( nfingers - 2 ) ) && ( flags & DOUBLE_END ) ) || 
      ( ( i == nfingers/2 ) && ( flags & CENTRE_SKIP) ) ) {
      y2 += gap;
      flags &= DOUBLE_START ^ -1 ;
      ++i;
    }
    if ( ( i == nfingers/2 ) && ( flags & FLAG_MASK )  ) {
      y2 += adjust;
    }
    // fprintf(stderr,"i=%d nfingers=%d\n",i,nfingers);
    if ( ++count & 1 ) {
      line ( x1,y1, x1,y2 );
      if ( i+1 < nfingers )
        line ( x1,y2, x2,y2 );
    } else {
      line ( x2,y1, x2,y2 );
      if ( i+1 < nfingers )
        line ( x2,y2, x1,y2 );
    }
    y1=y2;
   } else {
   // Hozirontal
    x2 = x1 + gap;
    y2 = y1 + thickness;
    if ( flags & DOUBLE_START || ( ( i == nfingers - 2 ) && ( flags & DOUBLE_END ) )  || 
      ( ( i+1 == nfingers/2 ) && ( flags & CENTRE_SKIP) ) ) {
      x2 += gap;
      flags &= ( DOUBLE_START ^ -1 ) ;
      i++;
    } 
    if ( ( i == nfingers/2 ) && ( flags & FLAG_MASK )  ) {
      x2 += adjust;
    }
    if ( ++count & 1 ) {
      line ( x1,y1, x2,y1 );
      if ( i+1 != nfingers )
        line ( x2,y1, x2,y2 );
    } else {
      line ( x1,y2, x2,y2 );
      if ( i+1 != nfingers )
        line ( x2,y2, x2,y1 );
    }
    x1=x2;
  }
 }
}

void text(float x, float y,float textHeight , char *text) {
  printf( "0\n\
TEXT\n\
  5\n\
6C\n\
100\n\
AcDbEntity0\n\
  8\n\
0\n\
  6\n\
ByLayer\n\
 62\n\
  256\n\
370\n\
   -1\n\
100\n\
AcDbText\n\
 10\n\
%f\n\
 20\n\
%f\n\
 30\n\
0\n\
 40\n\
%f\n\
  1\n\
%s\n\
 50\n\
0\n\
 41\n\
1\n\
 51\n\
0\n\
  7\n\
iso3098\n\
 71\n\
    0\n\
210\n\
0\n\
220\n\
0\n\
230\n\
1\n",x,y,textHeight,text );
}


void circle ( float x, float y,float r) {
  printf("0\nCIRCLE\n10\n%f\n20\n%f\n40\n%f\n62\n%d\n",x,y,r,colour); // 62=color,0-black,1=red,3-green ...
  CutLength += ( M_PI * 2 * r );
  // fprintf(stderr,"circle: radius=%f Cut Length = %fmm\n",r,M_PI * 2 * r );
}

/* http://images.autodesk.com/adsk/files/autocad_2012_pdf_dxf-reference_enu.pdf
Arc group codes
Group code	Description	
100		Subclass marker (AcDbCircle)
39		Thickness (optional; default = 0)
10		Center point (in OCS) DXF: X value; APP: 3D point
20,30		DXF: Y and Z values of center point (in OCS)
40		Radius
100		Subclass marker (AcDbArc)
50		Start angle
51		End angle
210		Extrusion direction (optional; default = 0, 0, 1)
		  DXF: X value; APP: 3D vector
220,230		DXF: Y and Z values of extrusion direction (optional)
*/
void arc(float x, float y, float radius, float startAngle, float endAngle){
  printf("0\nARC\n10\n%f\n20\n%f\n40\n%f\n50\n%f\n51\n%f\n62\n%d\n"
    ,x,y,radius,startAngle,endAngle,colour); // 62=color,0-black,1=red,3-green
  if(endAngle < startAngle ) startAngle+=360;
  CutLength += ( M_PI * 2 * radius*(endAngle-startAngle)/360 );
  // fprintf(stderr,"arc: startAngle=%f endAngle=%f radius=%f length=%f\n",startAngle,endAngle,radius,M_PI * 2 * radius*(endAngle-startAngle)/360);
}

void square ( float x, float y,float r) {
  line ( x-r,y-r, x+r,y-r );
  line ( x+r,y-r, x+r,y+r );
  line ( x+r,y+r, x-r,y+r );
  line ( x-r,y+r, x-r,y-r );
}

// Cuts a horizontal slot for ribbon cable passthrough
// adapt for vertical slots or diagonal when required
void slot ( float x1,float y1,float x2,float y2 , float width ) {
  float w2 = width/2;
  line ( x1,y1-w2,x2,y2-w2);
  line ( x1,y1+w2,x2,y2+w2);
  //circle ( x1,y1,w2);
  //circle ( x2,y2,w2);
  arc( x1,y1,w2,90,270 );
  arc( x2,y2,w2,270,90 );
}

void triangle(float x,float y,float length){
  line (x-length/2,y,x+length/2,y);
  line (x-length/2,y,x,y+length*13.0/15.0);
  line (x+length/2,y,x,y+length*13.0/15.0);
}

void polygon ( float cx,float cy, float radius, float N ){
  float x0=0,y0=0;
  float x,y;
  for ( int i = 0 ; i < N+2 ; i++ ) {
    x = cx + radius * cos(2.0*M_PI*i/N+2.0*M_PI/4.0);
    y = cy + radius * sin(2.0*M_PI*i/N+2.0*M_PI/4.0);
    if ( i > 1 ) {
      line (x0,y0,x,y);
    }
    x0=x; y0=y;
  }
}

void crossHair ( float cx,float cy, float size ){
  line (cx-size,cy     ,cx+size,cy     );
  line (cx     ,cy-size,cx     ,cy+size);
}


void display6digit(float cx, float cy ) {
  // NB pcb 90 x 26 mm
  float bezelx = 76.0/2;
  float bezely = 20.0/2;
  float holex = 84.0/2;
  float holey = 20.0/2;
  float holer=1.0;
  rectangle ( cx+bezelx,cy+bezely,cx-bezelx,cy-bezely);
  circle(cx+holex,cy+holey,holer);
  circle(cx-holex,cy+holey,holer);
  circle(cx-holex,cy-holey,holer);
  circle(cx+holex,cy-holey,holer);
}


void lcd (float cx, float cy ) {
  // NB PCB 81 x 37 mm
  float bezelx = 72.0/2;
  float bezely = 25.0/2;
  float holex = 75.0/2;
  float holey = 31.0/2;
  float holer=1.0;
  rectangle ( cx+bezelx,cy+bezely,cx-bezelx,cy-bezely);
  circle(cx+holex,cy+holey,holer);
  circle(cx-holex,cy+holey,holer);
  circle(cx-holex,cy-holey,holer);
  circle(cx+holex,cy-holey,holer);
}

void squareButton (float bx, float by , int rh_hole ){
  float switch_mm = 17.0;
  float fix = (25.4*1.1)/2;
  circle ( bx-fix, by, 1 ) ; // 2mm diameter screw holes        
  square ( bx, by, switch_mm/2.0 ) ; // 17mm wide squares
  crossHair(bx,by,1.0);
  if ( rh_hole )
    circle ( bx+fix, by, 1 ) ; // 2mm diameter screw holes          
}


/*
Draw a polyline rotated around the origin by rotation (radians) offset by x,y
x,y:      displacement
rotation: in degrees
points[0] number of points
points[1,2] x0,y0
points[3,4] x1,y1 .... */
void polyline(float x,float y, float rotation, float * points ) {
  int npoints = *points++;
  float x0;
  float y0;
  float xn = *points++;
  float yn = *points++;
  rotation = 2.0 * M_PI * rotation / 360;
  float x1 = x + xn * cos(rotation) - yn * sin( rotation );
  float y1 = y + xn * sin(rotation) + yn * cos( rotation );
  --npoints;
  while ( npoints-- > 0  ) {
    x0 = x1;
    y0 = y1;
    //fprintf(stderr,"x0=%f,y0=%f\n",x0,y0);
    xn = *points++;
    yn = *points++;
    x1 = x + xn * cos(rotation) - yn * sin( rotation );
    y1 = y + xn * sin(rotation) + yn * cos( rotation );
    line( x0,y0,x1,y1);
  }
}


void displayPanel ( float dpx,float dpy ) {
    float step = 1.1*25.4;
    
//    lcd ( dpx , dpy );
//    display6digit( 100+dpx, dpy );
    for ( int i = -15 ; i < 15 ; i++ ) {
      switch( i ) {
        case -14 :
          // NB 3mm left offset as 7 seg next to LCD is a tight fit
          display6digit(dpx+i*step+step/2.0-3,dpy );
        break;
        case -11 :
          // NB 2mm left offset as LCD next to button is a tight fit
          lcd((dpx+i*step+step/2.0-2.0),dpy );
        break;
        case  10 :
        case  13 :
          lcd(dpx+i*step+step/2.0+3,dpy );
        break;
        case -15 :
        // case -14 :
        case -13 :
        case -12 :
        // case -11 :
        case -10 :
        case   9 :
        // case 10 :
        case   11 :
        case   12 :
        // case 13 :
        case   14 :
        break;
        default :
          squareButton(dpx+i*step+step/2.0,dpy, i==8 );
        }
    }
}


float musicgap = 10; // Music stand and bracket gap fixed at 10mm
// Bracket to attach to music stand
void bracket ( float x , float y ,float thickness ) {
  float w=70; // width of bracket foot
  float pline[ ] = { 
    7, 0,0, w/2.0-thickness/2.0, 0, w/2.0-thickness/2.0,40,
            w/2.0+thickness/2.0,40, w/2.0+thickness/2.0, 0, 
            w,0, w,176.7 };
  polyline ( x+20,y, 19.546368 , pline);
  fingerJoint(x,y,musicgap,thickness,19,VERTICAL);
  line ( x,y+190, x+27.1,y+190 );// Top
  line (x,y,x+20,y );//Bottom
  rectangle(x+64,y+190,x+89,y+90); // spacers
  fingerJoint(x+64,y+190-25-thickness/2,5,thickness,5,HORIZONTAL);// perpendicular drilling guide
  line( x+64,y+140,x+89,y+140); // spacers
  if ( x == 0 ) {
    line( x+64+6.0,y+90   ,x+64+6.0,y+90+25); // 6.0mm divider For key support measurement
    line( x+64+6.0,y+90+25,x+64+6.5,y+90+25); // 0.5mm step
    line( x+64+6.5,y+90+25,x+64+6.5,y+90+50); // 6.5mm divider
    line( x+64+13.5,y+90   ,x+64+13.5,y+90+50); // 7.0-7.5 divider
    line( x+77.5+8.0,y+90   ,x+77.5+8.0,y+90+25); // 8.0mm divider
    line( x+77.5+8.0,y+90+25,x+77.5+8.5,y+90+25); // 0.5mm step
    line( x+77.5+8.5,y+90+25,x+77.5+8.5,y+90+50); // 8.5mm divider
  }
}

// The supports mount under the keys to limit the travel of black and white keys
// To protect the key contacts from energetic players
// Ten Thumb piston wires pass under the supports through individual gaps
// x,y start loation
// length keyboard size
// hw = height of white key support
// hb = height of black key support
// Note: Felt covering ranges from 0.66-0.88mm
// returns y increment
float  keySupports(float x,float y, float length, float hw,float hb ){
  float ww = 1.50; // wire width
  float wp = 2.54; // wire pitch
  float sh = 1.5; // slot height
  // Wire pass through 2.54mm pitch, 1.4mm slot
  float xd;
  for ( xd = 0 ; xd < length-2.54 ; xd += wp ) {
    line (x+xd,y+hw,x+xd+wp-ww,y+hw);
    if(hb > 4 ) { // is support thick enough to pass wires under?
      //setColour(GREEN);
      line (x+xd+wp-ww,y+hw,x+xd+wp-ww,y+hw+sh);
      arc ( x+xd+wp-ww/2,y+hw+sh,ww/2,0,180 );
      line (x+xd+wp,y+hw+sh,x+xd+wp,y+hw);
    }
    line (x+xd+wp,y+hw,x+xd+wp,y+hw-sh);
    arc ( x+xd+wp-ww/2,y+hw-sh,ww/2,180,360 );
    line (x+xd+wp-ww,y+hw-sh,x+xd+wp-ww,y+hw);
    line (x+xd+wp-ww,y+hw,x+xd+wp,y+hw);
  }
  line (x+xd,y+hw,x+length,y+hw); // complete cut to ne edge  
  line( x,y      , x,y+hw+hb);
  line( x,y+hw+hb, x+length,y+hw+hb);
  line(x+length,y+hw+hb,x+length,y);
  line(x+length,y, x,y); 
  return ( hw+hb ) ;
}


// Bracket to attach to music stand
void bracket_h ( float x , float y ,float thickness ) {
  float w=70; // width of bracket foot

   /* 7, 
    0,0, 
    w/2.0-thickness/2.0,0,
    w/2.0-thickness/2.0,40,
    w/2.0+thickness/2.0,40,
    w/2.0+thickness/2.0,0,
    w,0,w,
    176.7 };*/

  float pline[ ] = { 7,
    0,0,
    0,w/2.0-thickness/2.0,
    40,w/2.0-thickness/2.0,
    40,w/2.0+thickness/2.0,
    0,w/2.0+thickness/2.0,
    0,w,
    176.7,w
  };
  polyline ( x,y+20, 0-19.546368 , pline); // Rotate 19.5 degrees
  fingerJoint(x,y,musicgap,thickness,19,HORIZONTAL);
  line ( x+190,y, x+190,y+27.1 );// Top
  line (x,y,x,y+20 );//Bottom
  float xs=x+100,ys=y+60;
  rectangle(xs,ys,xs+100,ys+25); // spacers
  fingerJoint(xs+75-thickness/2,ys,5,thickness,5,VERTICAL);// perpendicular drilling guide
  line( xs+50,ys,xs+50,ys+25); // spacers
  if ( x == 0 ) { // gauges 4.0-7.5mm in 0.5mm steps
    line( xs,ys+4.0,xs+25,ys+4.0); // divider in 0.5mm steps for key support measurement
    line(xs+25,ys+4.0,xs+25,ys+4.5);
    line( xs+25,ys+4.5   ,xs+50,ys+4.5); // divider in 0.5mm steps for key support measurement
    line( xs,ys+9.5,xs+50,ys+9.5); 
    ys+=9.5;
    line( xs,ys+6.0,xs+25,ys+6.0); // divider in 0.5mm steps for key support measurement
    line(xs+25,ys+6.0,xs+25,ys+6.5);
    line( xs+25,ys+6.5   ,xs+50,ys+6.5); // divider in 0.5mm steps for key support measurement
    line( xs,ys+13.5,xs+50,ys+13.5); 
  }
}


// print dxf format features based on internal dimensions
void tray(float length,float width,float rearHeight,float frontHeight,float thickness) {

 // float switch_mm = 15.0; // size of switch plus a clearance gap  
  int kl  = 136;  // key length 
  int kls =   0;  // key length step height where tray above locates on tray below
  
  // Derive all coordinates from base location (current offset from origin)
  float xsw=kls+rearHeight+thickness;
  float ysw=frontHeight+thickness;
  float xne=xsw+length,yne=ysw+width;
#ifdef CONSTRUCTION_LINES_ON
  setColour(CYAN);
  rectangle ( xne,yne,xsw,ysw); // display base board internal dimensions, remove when design complete
  setColour(DEFAULT_COLOUR);
#endif
  
  // front panel button apertures
  // TODO: adjust button position to final pcb
  float mx = kls+rearHeight+thickness+length/2;
  float my = frontHeight/2;
  float max_y = 0;
  float step = 1.1*25.4; // Button spacing 1.1 inches, convert to mm
  int max_n = length/step/2;
  for ( int i = -max_n ; i < max_n ; i++ ) {
     squareButton( mx+i*step+step/2.0,my+2 , i==14); // 2mm offset to avoid pcb components
  }
  for ( float x = -400; x < 401 ; x+=100 ) { // mounting holes for music stand etc
    circle(x+(xne+xsw)/2 , yne+thickness+rearHeight/2, 1 );
  }
  
  // locating lugs and supports
  float lug_radius = 1.0;
  circle(xsw+lug_radius,ysw+lug_radius, lug_radius );// sw locating lug
  rectangle(xsw-thickness,ysw-thickness,xsw-thickness-rearHeight,ysw-frontHeight-thickness);//sw spacer
  
  circle(xne-lug_radius,ysw+lug_radius, lug_radius );// se locating lug
  rectangle(xne+thickness,ysw-thickness,xne+thickness+rearHeight,ysw-frontHeight-thickness);//se spacer
    
  circle(xsw+lug_radius,yne-lug_radius, lug_radius );// nw locating lug
  square(xsw-rearHeight/2-thickness,yne+rearHeight/2+thickness,rearHeight/2); // nw spacers
  
  line( xsw-rearHeight-thickness,yne+rearHeight/2+thickness,xsw-thickness,yne+rearHeight/2+thickness); // split space
  
  slot ( xsw+75,yne+thickness+13, xsw+85,yne+thickness+13 ,8 ); // USB Cable entry
  
  circle(xne-lug_radius,yne-lug_radius, lug_radius );// ne locating lug
  square(xne+rearHeight/2+thickness,yne+rearHeight/2+thickness,rearHeight/2); // ne spacers
  line  (xne+thickness,yne+rearHeight/2+thickness,xne+rearHeight+thickness,yne+rearHeight/2+thickness); // split spacer

  // Base Keyboard Mounting holes
  float kbd_screw_radius = 2.0;
  circle(xsw + 37.5,ysw+ 72.5, kbd_screw_radius ); 
  circle(xsw + 37.5,ysw+147.0, kbd_screw_radius ); 
  circle(xsw +189.0,ysw+ 72.5, kbd_screw_radius ); 
  circle(xsw +189.0,ysw+147.0, kbd_screw_radius ); 
  circle(xsw +384.0,ysw+ 72.5, kbd_screw_radius ); 
  circle(xsw +384.0,ysw+147.0, kbd_screw_radius ); 
  circle(xsw +491.0,ysw+ 72.5, kbd_screw_radius ); 
  circle(xsw +491.0,ysw+147.0, kbd_screw_radius ); 
  circle(xsw +671.0,ysw+ 72.5, kbd_screw_radius ); 
  circle(xsw +671.0,ysw+147.0, kbd_screw_radius ); 
  circle(xsw +836.0,ysw+ 72.5, kbd_screw_radius ); 
  circle(xsw +836.0,ysw+147.0, kbd_screw_radius ); 
  
  // slot( xsw+130,ysw+160,xsw+170, ysw+160,10 ); // keyboard ribbon cable 139mm from rear panel
    
  // Front panel
  int lnFingers = (length+thickness*2)/thickness/2; // Number of fingers across inside length
  lnFingers = lnFingers + lnFingers - 1;
  float lgap = (length+thickness*2)/lnFingers;
  //fprintf(stderr,"length=%f lnFingers=%d lgap=%f\n",length,lnFingers,lgap);
  int hnFingers = 5; // ((int)(rearHeight/thickness)); // Number of fingers up inside height at rear
  int fnFingers = 3; // number of front panel finger spaces
  float fgap = 9.0;
  int wnfingers = (width+thickness*2)/thickness/2;
  wnfingers = wnfingers + wnfingers -1 ;
  float wgap = (width+thickness*2)/wnfingers; // width of finger (now defaulted to thickness)
  //fprintf(stderr,"width=%f wnfingers=%d wgap=%f\n",width,wnfingers,wgap);
  
  fingerJoint(xsw-thickness,ysw-thickness,lgap,thickness,lnFingers,
    HORIZONTAL+DOUBLE_START+DOUBLE_END); // 1 S
  fingerJoint(xne+thickness,ysw-thickness,-fgap,-thickness,fnFingers,VERTICAL); // 2 se left side
  line( xne+thickness,ysw-frontHeight-thickness,xsw-thickness,ysw-frontHeight-thickness ); // 3
  fingerJoint(xsw-thickness,ysw-frontHeight-thickness,fgap,thickness,fnFingers,VERTICAL); // 4 sw right side

  // base
  fingerJoint( xsw-thickness,ysw-thickness,wgap,thickness, wnfingers ,
     VERTICAL  +DOUBLE_START+DOUBLE_END ); // 5
  fingerJoint( xsw-thickness,yne+thickness,lgap,-thickness, lnFingers,  
     HORIZONTAL+DOUBLE_START+DOUBLE_END); // 6
  fingerJoint( xne+thickness,yne+thickness,-wgap,-thickness, wnfingers,VERTICAL+DOUBLE_START+DOUBLE_END ); // 7
  // RH / West Side Panel
  fingerJoint( xne+thickness,ysw,fgap,-thickness,hnFingers,HORIZONTAL+DOUBLE_END); // 8 SE RH Side Panel
//  line( xne+thickness*(hnFingers+1),ysw-thickness,xne+thickness+rearHeight+kls,ysw-thickness ); // extend
  line( xne+thickness+rearHeight+kls,ysw-thickness,xne+thickness+rearHeight+kls,ysw+kl); // side piano key cut out
  line( xne+thickness+rearHeight+kls,ysw+kl,xne+thickness+rearHeight,ysw+kl); // side piano key cut out
  line( xne+thickness+rearHeight,ysw+kl, xne+rearHeight+thickness, yne); // 9
  fingerJoint( xne+rearHeight+thickness, yne,/*-thickness*/-rearHeight/hnFingers,thickness,hnFingers,HORIZONTAL); // 10
  fingerJoint(xne+thickness,yne+thickness,rearHeight/hnFingers,-thickness,hnFingers,VERTICAL); // 11 ne back/side
  if ( width > 200 ) {
    line( xne+thickness,yne+thickness+rearHeight, xsw-thickness,yne+thickness+rearHeight ); // 12
    max_y = yne+thickness+rearHeight;
  } else {
    float displayHeight = 50.0 ; // internal
    float displayWidth  = 25.0 ; // internal ToDo: Fix overall widths for enlarged display space
    max_y = yne+thickness+rearHeight+(displayHeight+thickness)*2 + displayWidth;
#ifdef CONSTRUCTION_LINES_ON
    setColour(CYAN);
    rectangle( xsw,yne+thickness*2+rearHeight+displayHeight,
               xne,yne+rearHeight+displayHeight+thickness*2+displayWidth);
    setColour(DEFAULT_COLOUR);
#endif
    fingerJoint(xne,yne+thickness*3+rearHeight+displayHeight+displayWidth,rearHeight/hnFingers,thickness,hnFingers,VERTICAL); // 11 ne back/side
    fingerJoint(xne+thickness,yne+rearHeight+displayHeight+thickness,(displayWidth+thickness*2)/5,-thickness,5,VERTICAL+DOUBLE_START+DOUBLE_END);
    line(xne+thickness,yne+rearHeight+displayHeight+displayWidth+thickness*3,
      xne+thickness,yne+rearHeight+displayHeight*2+displayWidth+thickness*3);
    line(xne+thickness+rearHeight,yne+rearHeight+displayHeight*2+displayWidth+thickness*3,
      xne+thickness+rearHeight,yne+rearHeight+displayHeight+thickness*1);
    rectangle(xne+thickness           ,yne+thickness+rearHeight,
              xne+thickness+rearHeight,yne+thickness+rearHeight+displayHeight);
    fingerJoint( xne+thickness,yne+thickness+rearHeight+displayHeight,
                -lgap,+thickness,lnFingers,HORIZONTAL+DOUBLE_START+DOUBLE_END); // 12a
    fingerJoint( xsw-thickness, yne+thickness*3+rearHeight+displayHeight+displayWidth,
                +lgap,-thickness,lnFingers,HORIZONTAL+DOUBLE_START+DOUBLE_END); // 12b
    line(xne+thickness+rearHeight,yne+thickness+rearHeight+(displayHeight+thickness)*2 + displayWidth,
         xsw-thickness-rearHeight,yne+thickness+rearHeight+(displayHeight+thickness)*2 + displayWidth ); // 12c display panel + sides ne to nw
    fingerJoint(xsw,yne+thickness*3+rearHeight+displayHeight+displayWidth,rearHeight/hnFingers,-thickness,hnFingers,VERTICAL); // 11 se back/side
    fingerJoint(xsw-thickness,yne+rearHeight+displayHeight+thickness,(displayWidth+thickness*2)/5,thickness,5,VERTICAL+DOUBLE_START+DOUBLE_END);
    line(xsw-thickness,yne+rearHeight+displayHeight+displayWidth+thickness*3,
         xsw-thickness,yne+rearHeight+displayHeight*2+displayWidth+thickness*3);
    line(xsw-thickness-rearHeight,yne+rearHeight+displayHeight*2+displayWidth+thickness*3,
         xsw-thickness-rearHeight,yne+rearHeight+displayHeight+thickness*1);
    rectangle(xsw-thickness           ,yne+thickness+rearHeight,
              xsw-thickness-rearHeight,yne+thickness+rearHeight+displayHeight);
    float sfy=yne+thickness+rearHeight+displayHeight+thickness;
    fingerJoint( xsw-thickness,sfy-thickness,-10,thickness,5,HORIZONTAL); // nw display side panel
    line( xsw-thickness,sfy+displayWidth,xsw-thickness-displayHeight,sfy+displayWidth); // nw display side panel
    line(xsw-thickness,sfy+50,xsw-thickness-rearHeight,sfy+50); // ne scrap spacer  
    line(xsw-thickness,max_y-25,xsw-thickness-rearHeight,max_y-25); // ne scrap spacer
    fingerJoint(xsw-thickness*1.5-rearHeight/2,max_y,-5,thickness,5,VERTICAL); // scrap right angle drilling jig     

    displayPanel( (xsw+xne) /2 , yne+thickness+rearHeight+thickness*2+displayHeight*1.5 + displayWidth  );

/*    rectangle(xne+thickness           ,yne+thickness+rearHeight+(displayHeight+thickness)*2 + displayWidth,
              xne+thickness+rearHeight,yne+thickness+rearHeight );*/ // 12e nw spacers 
    fingerJoint( xne+thickness,sfy-thickness,+10,+thickness,5,HORIZONTAL); // ne display side panel
    line( xne+thickness,sfy+displayWidth,xne+thickness+displayHeight,sfy+displayWidth); // ne display side panel
    line(xne+thickness,sfy+50,xne+thickness+rearHeight,sfy+50); // ne scrap spacer  
    line(xne+thickness,max_y-25,xne+thickness+rearHeight,max_y-25); // ne scrap spacer      
    fingerJoint(xne+thickness*1.5+rearHeight/2,max_y,-5,-thickness,5,VERTICAL); // ne scrap right angle drilling jig     
    max_y = yne+thickness+rearHeight+(displayHeight+thickness)*2 + displayWidth;
  }
  fingerJoint(xsw-thickness,yne+thickness+rearHeight,-rearHeight/hnFingers,thickness,hnFingers,VERTICAL); // 13
  fingerJoint( xsw-thickness, yne,-rearHeight/hnFingers,thickness,hnFingers,HORIZONTAL); // 14
  line( xsw-thickness-rearHeight,yne   , xsw-thickness-rearHeight,ysw+kl ); // 15 LH side panel top overlap
  line( xsw-thickness-rearHeight,ysw+kl,xsw-thickness-rearHeight-kls,ysw+kl); // side piano key step up
  line( xsw-thickness-rearHeight-kls,ysw+kl,xsw-thickness-rearHeight-kls,ysw-thickness); // side piano key cut out
  fingerJoint(xsw-thickness-4*fgap,ysw-thickness,fgap,thickness,hnFingers-1,HORIZONTAL); // 16 SW Side finger

#ifdef CONSTRUCTION_LINES_ON
  setColour(GREEN);
  line(mx,0,mx,max_y); // centre line
  setColour(DEFAULT_COLOUR);
#endif  
  float max_x = xne+thickness+rearHeight+kls;
  switch ( (int) width ) { // Late re-arrangment to simplify cutting from blanks
    case 172 :
      for ( int x = 0 ; x < 4*190 ; x+=190 ) {
        bracket_h(x,max_y,thickness);
      }
      for ( int i = 0 ; i <= 200 ; i+=50 ) { // use up remainder for more spacers
        line( max_x-200+i,max_y,max_x-200+i,max_y+90 );
      }
      for ( int i = 25 ; i < 90 ; i+=25 ) { // use up remainder for more spacers
        line( max_x-200,max_y+i,max_x,max_y+i );
      }
      line( max_x-200,max_y+90,max_x,max_y+90 );
      max_y += 90;
    break;
    case 444 :
      float endClearance=2.54;
      preamble();
      for ( int i = 0 ; i < 3 ; i++ ) {
        max_y+=keySupports(0,max_y,length-endClearance,7-0.6,2.25-0.6); // Limit travel of black and white keys
      }
    break;
  }
  fprintf(stderr,"Material: %3.1f x %3.1f x %3.2f mm MDF\n",max_x,max_y,thickness);
}

void musicStand(float length,float height, float base, float thickness) {
  int nFingers = length / thickness;
  float r = 15;
  float my = height+base+thickness;
  for (int i = 0 ; i < 7 ; i++) { // rod holders to retain music
    circle( i*length/8+length/8,thickness+4, 2 );
  }
  for (int i = 0 ; i < 4 ; i++) { // bracket attachment
    for ( int j = 0 ; j < 10 ; j++ ) {
      rectangle( i*length/4+length/8-thickness/2, 10+base+musicgap  +j*musicgap*2,
                 thickness/2+i*length/4+length/8, 10+base+musicgap*2+j*musicgap*2 );
    }
  }
  float gap = length/nFingers;
  fingerJoint( 0,base,gap,thickness,nFingers,HORIZONTAL);
  rectangle (0,0,length,my);
  arc(0+r,0+r,r,180,270);
  arc(length-r,0+r,r,270,0);
  arc(length-r,my-r,r,0,90);
  arc(0+r,my-r,r,90,180);
  fprintf(stderr,"Material: %3.1f x %3.1f x %3.2f mm MDF\n",length,base+thickness+height,thickness);
}

// Generate a test puzzle coaster to check joints, cutting speed and button clearances
void test(float maxx,float maxy,float thickness) {
  float cx = maxx/2;
  float cy = maxy/2;
  line(1,8,0,10); // Y axis arrow
  line(0,10,0,0); // Y axis arrow
  line(0,0,10,0); // X axis arrow
  line(10,0,8,1); // X axis arrow
  text(89,0,5,"X" );
  text(0,88,5,"Y" );  
  fingerJoint( cx-thickness/2,cy+thickness/2,-thickness,-thickness,4,HORIZONTAL         );
  fingerJoint( cx-thickness/2,cy-thickness/2          , thickness, thickness,5,HORIZONTAL+DOUBLE_START);
  fingerJoint( cx-thickness/2,cy-thickness/2          ,-thickness, thickness,4,VERTICAL);
  fingerJoint( cx-thickness/2,cy-thickness/2          , thickness, thickness,5,VERTICAL  +DOUBLE_START);
  //circle( centrex,centrey,centrex );
  //circle( centrex,centrey,centrex-thickness );
  // surround 
  float ar = 1.5*thickness;
  arc( cx-thickness*3  ,cy-thickness*3 ,ar,180,270 );
  arc( cx-thickness*3  ,cy+thickness*3 ,ar, 90,180);
  arc( cx+thickness*3  ,cy+thickness*3 ,ar,  0, 90);
  arc( cx+thickness*3  ,cy-thickness*3 ,ar,270,360);
  ar += thickness/2;
  ar = 1.5*thickness + (maxx-thickness*9)/2;
  arc( cx-thickness*3  ,cy-thickness*3 ,ar,180,270 );
  arc( cx-thickness*3  ,cy+thickness*3 ,ar, 90,180);
  arc( cx+thickness*3  ,cy+thickness*3 ,ar,  0, 90);
  arc( cx+thickness*3  ,cy-thickness*3 ,ar,270,360);
  //rectangle(thickness,thickness,thickness*10,thickness*10 ); // inside surround
  float xl=thickness*3;
  float yl=thickness*4.5;
  line(cx-xl,cy-yl,cx+xl,cy-yl);
  line(cx+yl,cy-xl,cx+yl,cy+xl);
  line(cx+xl,cy+yl,cx-xl,cy+yl);
  line(cx-yl,cy+xl,cx-yl,cy-xl);
  //rectangle(0,0,maxx,maxy ); // outside surround
  yl = maxy/2;
  line(cx-xl,cy-yl,cx+xl,cy-yl);
  line(cx+yl,cy-xl,cx+yl,cy+xl);
  line(cx+xl,cy+yl,cx-xl,cy+yl);
  line(cx-yl,cy+xl,cx-yl,cy-xl);

  float fixr = 1.0; // fixing screw tap hole radius
  float cbx = cx-thickness*2.5;
  float cby = cy+thickness*2.5;
  circle(cbx   ,cby,15.0/2.0 ); // circular button
  circle(cbx-14,cby,fixr); // fixing
  circle(cbx+14,cby,fixr); // fixing
  crossHair(cbx,cby,1);

  cby = cy+thickness*2.5;
  squareButton (cbx,cy-thickness*2.5 , true );
  //polygon( centrex+23,centrey+22,8.5,3 ); // triangle
  cbx = cx+thickness*2.5;
  arc(cbx,cby,15.0/2.0,0,180 ); // semi circle
  line(cbx-7.5,cby,cbx+7.5,cby);
  cby = cy-thickness*2.5;
  polygon  ( cbx,cby,9,5 ); // Pentagon
  crossHair( cbx,cby,1.0   );
  fprintf(stderr,"Material: %3.1f x %3.1f x %3.2f mm MDF\n",maxx,maxy,thickness);
}

void panel(float x,float y,float length,float width,float margin,float cut,char *name) {
  char size[80];
  float textHeight = 30;
  if ( textHeight > width ) textHeight = width;
  sprintf(size,"%3.1f x %3.1f",length,width);
  rectangle(x+margin,y+margin,x+length+margin,y+width+margin);
  rectangle(x,y,x+length+margin*2,y+width+margin*2);
  text(x+length/2-strlen(name)*textHeight, y+width/2,textHeight , name);
  text(x+length/2, y+width/2,textHeight/2 , size);
}

void cuttingPlan() {
  float x = 0 ;
  float y = 0;
  float x1 = 1200;
  float y1 = 0;
  float width;
  float margin = 10; // mm
  float cut = 10; //
  preamble();      
  rectangle(0,0,1200,1200); // Draw blank material sheets
  rectangle(1200,0,2400,1200);
  text(1200-150, 1230,30 , "Cutting Plan");

  panel(x,y,973.7,width= 403.7,margin,cut,"tray308");
  y += width+margin*2+cut;

  panel(x,y,873.7,width= 379.4,margin,cut,"MusicStand");
  text(x+873.7+30,y+379.4/2,15 , "Test");
  y += width+margin*2+cut;

  panel(x1,y1,973.7,width= 563.9,margin,cut,"tray444");  
  y1 += width+margin*2+cut;

  panel(x1,1200-521.4,973.7,width=501.4,margin,cut,"tray172");
  y1 += width+margin*2+cut; 

/*  panel(x1,y1,852,width=26.2,margin,cut,"KeySupports");
  y1 += width+margin*2+cut;  

  panel(x1,y1,width=800,90,margin,cut,"Brackets");
  y1 += width+margin*2+cut; */

  setColour(RED); // Left hand side
  line ( 0,403.7+20+5,0+973.7+30+5,403.7+20+5); // Horizontal
  line ( 0,403.7+379.4+40+15,1200/*0+973.7+30+5*/,403.7+379.4+40+15); // Horizontal
  line ( 0+973.7+30+5,0,0+973.7+30+5,403.7+379.4+40+15); // Vertical

  setColour (BLUE); // Right hand side
  line (1200,600,1200+973.7+20+5 /*2400*/,600); // Short Half 
  setColour(GREEN); 
  //line(1200,539.7+20+5,1200+973.7+20+5,539.7+20+5); // Horizontal
  line ( 1200+973.7+20+5,0,1200+973.7+20+5,1200 ); // Vertical
  postamble();
}


int main (int argc , char * argv[] ) {
  // internal dimensions in mm, revised for 9mm constant finger joint gap
  float thickness=9.36;
  thickness=9.0;
  float length=855.0; // 95*9=855
  float width = -1.0; // set by argv[1] 117.0+thickness+134.0=254 round up 30*9=270 (was 254)
  float rearHeight=50.0; // internal
  float frontHeight=27.0; // 3*9=27 ie 3 finger joint gap size internal
  if ( argc == 2 ) sscanf(argv[1],"%f",&width);
  if ( width < 0 ) {
    fprintf(stderr,"USAGE: enclosure [width]\n\
    Generates a dxf drawing file for an organ keyboard enclosure with specified width.\n\
    A width of zero generates a test file\n" );
    fprintf(stderr,"argc=%d width=%f argv[1]='%s'\n",argc,width,argv[1]);
    return -1;
  }
  switch ( (int)width ) {
    case 0 : // Test all the used shapes in dxf
      preamble();
      test( thickness*10.5, thickness*10.5, thickness );
      postamble();
    break;
    case 1: // Music stand brackets
      preamble();
      for ( int x = 0 ; x < 4*90 ; x+=90 )
        bracket(x,0,thickness);
      for ( float y = 0 ; y < 176 ; y+=25 ) { // qty 7, 50x25mm spacers
        line ( 360,y,410,y);
      }
      line( 410.0,175.0, 410.0,  0.0);
      line( 360.0,  0.0, 360.0,175.0);
      postamble();
      fprintf(stderr,"Material: %3.1f x %3.1f x %4.2f mm MDF\n",4.0*90.0+50.0,190.0,thickness);
    break;
    case 2: // Music stand
      preamble();
      musicStand(length+thickness*2, 330, 40,thickness);
      postamble();
    break;
    case 3: // Piano Key Supports (reduce excess pressure on switches)
      float y = 0;
      float endClearance=2.54;
      preamble();
      for ( int i = 0 ; i < 3 ; i++ ) {
        y+=keySupports(0,y,length-endClearance,7-0.6,2.25-0.6); // Limit travel of black and white keys
        y++;
      }
      postamble();
      fprintf(stderr,"Material: %3.1f x %3.1f x %3.2f mm MDF\n",length-endClearance,--y,thickness);
    break;
    case 4 :
      cuttingPlan();
    break;
    
   
    default: 
      preamble();
      tray(length,width,rearHeight,frontHeight,thickness);
      postamble();
  }
  fprintf(stderr,"Cut Length = %.0fmm\n\n",CutLength);
  return 0;
}
