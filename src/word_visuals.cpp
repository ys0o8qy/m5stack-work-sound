#include "toy_visuals.h"
#include <cmath>
#include <cstring>

namespace toy {
namespace {
constexpr uint16_t BG=0x0926, INK=0x1148, WHITE=0xffbd, PEACH=0xfd52;
constexpr uint16_t MINT=0x7ef6, GOLD=0xff09, BLUE=0x6d9f, PINK=0xfc16, LILAC=0xb4ff;
bool is(const char* a, const char* b) { return !strcmp(a,b); }
}

// Related spoken variants share an intentional picture, e.g. rabbit / bunny.
// The spoken and printed word always remains the exact word the child typed.
bool Visuals::extendedIllustration(const char* w, int x, int y) {
  if (is(w,"elephant")) {
    c_.fillEllipse(x-27,y,17,25,BLUE); c_.fillEllipse(x+27,y,17,25,BLUE);
    c_.fillCircle(x,y,27,BLUE); face(x,y-6);
    c_.fillRoundRect(x-6,y+5,13,33,6,BLUE);
    c_.fillCircle(x+6,y+32,8,BLUE);
  } else if (is(w,"lion") || is(w,"tiger") || is(w,"panda") || is(w,"monkey") || is(w,"fox") || is(w,"mouse") || is(w,"sheep") || is(w,"horse")) {
    uint16_t coat = is(w,"panda") || is(w,"sheep") ? WHITE : is(w,"mouse") ? LILAC : PEACH;
    if (is(w,"lion")) for(int i=0;i<10;++i) c_.fillCircle(x+cosf(i*.6283f)*25,y+sinf(i*.6283f)*25,13,GOLD);
    for (int s : {-1,1}) {
      if (is(w,"fox")) c_.fillTriangle(x+s*6,y-15,x+s*28,y-39,x+s*30,y,PEACH);
      else c_.fillCircle(x+s*24,y-21,is(w,"mouse")?17:11,is(w,"panda")?INK:coat);
    }
    if (is(w,"sheep")) for(int i=0;i<10;++i) c_.fillCircle(x+cosf(i*.6283f)*24,y+sinf(i*.6283f)*21,10,WHITE);
    c_.fillEllipse(x,y,27,is(w,"horse")?32:25,coat);
    if (is(w,"panda")) for(int s : {-1,1}) {c_.fillEllipse(x+s*11,y-4,8,11,INK); c_.fillCircle(x+s*11,y-5,3,WHITE);}
    else {if(is(w,"monkey")) c_.fillEllipse(x,y+3,21,19,GOLD); face(x,y-4);}
    if(is(w,"tiger")) for(int s : {-1,1}) for(int i=0;i<3;++i) c_.drawWideLine(x+s*24,y-14+i*10,x+s*17,y-10+i*10,3,INK);
    if(is(w,"horse")) {c_.fillEllipse(x,y+20,20,11,GOLD);c_.fillCircle(x-7,y+19,2,INK);c_.fillCircle(x+7,y+19,2,INK);}
    else c_.fillTriangle(x-4,y+5,x+4,y+5,x,y+10,INK);
  } else if(is(w,"turtle") || is(w,"dinosaur")) {
    bool d=is(w,"dinosaur");
    c_.fillEllipse(x-7,y+4,28,20,MINT);
    c_.fillCircle(x+28,y-10,d?17:12,MINT); c_.fillCircle(x+32,y-14,3,INK);
    c_.fillRoundRect(x+13,y-6,13,23,5,MINT);
    for(int s : {-1,1}) c_.fillRoundRect(x-10+s*17,y+16,12,15,5,MINT);
    if(d) {c_.fillTriangle(x-24,y,x-47,y+18,x-17,y+17,MINT);for(int i=0;i<3;++i)c_.fillTriangle(x-28+i*13,y-10,x-22+i*13,y-24,x-16+i*13,y-10,GOLD);}
    else {c_.fillEllipse(x-7,y+3,24,18,0x354d); c_.drawCircle(x-7,y+3,10,MINT);}
  } else if(is(w,"whale")) {
    c_.fillTriangle(x-22,y+6,x-47,y-9,x-40,y+15,BLUE);
    c_.fillEllipse(x,y+4,35,24,BLUE);c_.fillEllipse(x+7,y+18,22,8,WHITE);c_.fillCircle(x+19,y,3,INK);
    for(int i=-1;i<=1;++i)c_.drawWideLine(x+5,y-21,x+5+i*12,y-36,3,MINT);
  } else if(is(w,"crab")) {
    for(int s : {-1,1}) {for(int i=0;i<3;++i)c_.drawWideLine(x+s*18,y+i*9,x+s*39,y+6+i*9,3,PEACH);
      c_.drawWideLine(x+s*20,y,x+s*35,y-16,4,PEACH);c_.fillCircle(x+s*35,y-21,10,PEACH);
      c_.fillTriangle(x+s*35,y-20,x+s*29,y-32,x+s*40,y-32,BG);
      c_.drawWideLine(x+s*10,y-5,x+s*14,y-20,4,PEACH);c_.fillCircle(x+s*14,y-20,4,WHITE);c_.fillCircle(x+s*14,y-21,2,INK);}
    c_.fillEllipse(x,y+6,24,18,PEACH);
  } else if(is(w,"butterfly")) {
    for(int s : {-1,1}) {c_.fillEllipse(x+s*20,y-13,20,22,LILAC);c_.fillEllipse(x+s*17,y+17,16,17,PINK);c_.fillCircle(x+s*23,y-15,8,GOLD);c_.drawLine(x,y-19,x+s*10,y-35,WHITE);}
    c_.fillRoundRect(x-4,y-23,8,48,4,PEACH);
  } else if(is(w,"snail")) {
    c_.fillRoundRect(x-32,y+16,68,13,6,MINT);c_.fillCircle(x-7,y,25,PEACH);
    c_.drawCircle(x-7,y,16,PINK);c_.drawCircle(x-7,y,7,PINK);
    for(int s : {-1,1}) {c_.drawWideLine(x+26,y+16,x+26+s*7,y-4,3,MINT);c_.fillCircle(x+26+s*7,y-5,4,WHITE);c_.fillCircle(x+26+s*7,y-5,2,INK);}
  } else if(is(w,"owl") || is(w,"penguin")) {
    bool owl=is(w,"owl");
    c_.fillEllipse(x,y,28,33,owl?PEACH:BLUE);c_.fillEllipse(x,y+9,20,23,WHITE);
    for(int s : {-1,1}) {c_.fillEllipse(x+s*27,y+4,8,19,owl?GOLD:BLUE);c_.fillCircle(x+s*11,y-11,owl?11:5,WHITE);c_.fillCircle(x+s*11,y-11,3,INK);c_.fillEllipse(x+s*12,y+32,9,4,GOLD);}
    c_.fillTriangle(x-5,y-3,x+5,y-3,x,y+5,GOLD);
    if(owl) for(int s : {-1,1})c_.fillTriangle(x+s*10,y-23,x+s*24,y-39,x+s*24,y-15,PEACH);
  } else if(is(w,"ant")) {
    for(int i=-1;i<=1;++i){c_.fillCircle(x+i*19,y,12,PEACH);for(int s : {-1,1})c_.drawWideLine(x+i*16,y+s*5,x+i*20-6,y+s*24,3,PEACH);}
    c_.fillCircle(x+23,y-3,2,INK);c_.drawLine(x+21,y-8,x+30,y-25,PEACH);
  } else if(is(w,"orange") || is(w,"pear") || is(w,"lemon") || is(w,"tomato") || is(w,"strawberry")) {
    uint16_t color=is(w,"orange")?0xfd20:is(w,"pear")?MINT:is(w,"lemon")?GOLD:PINK;
    if(is(w,"pear")){c_.fillCircle(x,y-12,16,color);c_.fillEllipse(x,y+9,25,22,color);}
    else if(is(w,"strawberry")){c_.fillCircle(x,y-7,26,color);c_.fillTriangle(x-25,y,x+25,y,x,y+32,color);for(int i=0;i<9;++i)c_.fillCircle(x-12+(i%3)*12,y-12+(i/3)*12,1,GOLD);}
    else c_.fillEllipse(x,y,is(w,"lemon")?34:28,26,color);
    c_.drawWideLine(x,y-24,x+3,y-36,3,PEACH);c_.fillEllipse(x+12,y-31,11,5,MINT);
    c_.fillEllipse(x-13,y-10,3,7,WHITE);
  } else if(is(w,"grape") || is(w,"cherry")) {
    if(is(w,"grape"))for(int row=0;row<4;++row)for(int i=0;i<4-row;++i)c_.fillCircle(x-24+row*8+i*16,y-21+row*15,10,LILAC);
    else for(int s : {-1,1}){c_.drawWideLine(x,y-30,x+s*19,y+5,3,MINT);c_.fillCircle(x+s*19,y+12,17,PINK);c_.fillCircle(x+s*19-5,y+6,3,WHITE);}
    c_.fillEllipse(x+8,y-31,12,5,MINT);
  } else if(is(w,"watermelon")) {
    c_.fillTriangle(x-41,y-22,x+41,y-22,x,y+33,MINT);
    c_.fillTriangle(x-33,y-19,x+33,y-19,x,y+24,PINK);
    for(int i=0;i<5;++i)c_.fillEllipse(x-20+i*10,y-10+(i%2)*12,2,3,INK);
  } else if(is(w,"carrot") || is(w,"corn")) {
    if(is(w,"carrot")){c_.fillTriangle(x-19,y-18,x+19,y-18,x-7,y+34,PEACH);for(int i=0;i<3;++i)c_.drawLine(x-13+i*2,y-6+i*10,x+2,y-6+i*10,GOLD);}
    else {c_.fillEllipse(x,y,19,32,GOLD);for(int i=-2;i<=2;++i)for(int j=-1;j<=1;++j)c_.fillCircle(x+j*10,y+i*10,3,PEACH);}
    for(int s : {-1,1})c_.fillEllipse(x+s*9,y-30,6,12,MINT);
  } else if(is(w,"bread") || is(w,"cookie") || is(w,"cheese")) {
    if(is(w,"bread")){c_.fillRoundRect(x-27,y-25,54,55,10,PEACH);c_.fillCircle(x-15,y-22,18,PEACH);c_.fillCircle(x+15,y-22,18,PEACH);c_.fillRoundRect(x-21,y-23,42,47,9,GOLD);}
    else if(is(w,"cookie")){c_.fillCircle(x,y,31,PEACH);for(int i=0;i<7;++i)c_.fillCircle(x+cosf(i*2.4f)*(i%2?12:22),y+sinf(i*2.4f)*(i%2?12:22),4,INK);}
    else {c_.fillTriangle(x-34,y+25,x+34,y+25,x-23,y-31,GOLD);c_.fillCircle(x-17,y-2,5,PEACH);c_.fillCircle(x+3,y+13,6,PEACH);c_.fillCircle(x-24,y+19,3,PEACH);}
  } else if(is(w,"cup") || is(w,"water") || is(w,"juice")) {
    if(is(w,"cup")){c_.drawCircle(x+27,y,13,WHITE);c_.drawCircle(x+27,y,10,WHITE);}
    c_.fillRoundRect(x-24,y-25,48,56,7,WHITE);c_.fillRect(x-19,y-18,38,22,BG);
    c_.fillRoundRect(x-19,y-2,38,28,4,is(w,"juice")?PEACH:BLUE);
    c_.drawWideLine(x+8,y-8,x+15,y-36,3,MINT);
  } else if(is(w,"bowl") || is(w,"rice")) {
    if(is(w,"rice")){c_.fillEllipse(x,y-8,31,18,WHITE);for(int i=0;i<8;++i)c_.drawLine(x-21+i*6,y-10,x-19+i*6,y-6,0xc638);}
    c_.fillEllipse(x,y+5,35,24,BLUE);c_.fillRect(x-36,y-24,72,25,BG);
    if(is(w,"rice"))c_.fillEllipse(x,y-3,30,8,WHITE);
    c_.drawWideLine(x-35,y,x+35,y,3,MINT);c_.fillRoundRect(x-17,y+25,34,5,2,BLUE);
  } else if(is(w,"spoon") || is(w,"fork")) {
    c_.fillRoundRect(x-4,y-1,8,35,4,BLUE);
    if(is(w,"spoon"))c_.fillEllipse(x,y-17,16,22,WHITE);
    else {c_.fillRoundRect(x-16,y-17,32,18,6,WHITE);for(int i=0;i<4;++i)c_.fillRoundRect(x-16+i*9,y-36,5,24,2,WHITE);}
  } else if(is(w,"icecream")) {
    c_.fillTriangle(x-23,y,x+23,y,x,y+36,PEACH);c_.fillCircle(x,y-13,27,PINK);
    c_.fillCircle(x-18,y+1,10,PINK);c_.fillCircle(x+18,y+1,10,PINK);c_.fillCircle(x,y-34,6,GOLD);
  } else if(is(w,"train") || is(w,"truck")) {
    c_.fillRoundRect(x-42,y-21,52,43,6,is(w,"train")?MINT:PEACH);
    c_.fillRoundRect(x+7,y-9,34,31,5,GOLD);c_.fillRect(x+14,y-4,17,12,BLUE);
    if(is(w,"train")){c_.fillRect(x-32,y-34,12,17,MINT);c_.fillRect(x-5,y-32,13,23,MINT);c_.fillRect(x-1,y-27,6,11,BLUE);}
    for(int i=0;i<3;++i){c_.fillCircle(x-28+i*28,y+23,9,INK);c_.fillCircle(x-28+i*28,y+23,4,WHITE);}
    c_.drawFastHLine(x-49,y+34,98,BLUE);
  } else if(is(w,"plane") || is(w,"rocket")) {
    if(is(w,"plane")){c_.fillTriangle(x-42,y+11,x+32,y+11,x+15,y-22,BLUE);c_.fillRoundRect(x-39,y-6,78,15,7,WHITE);c_.fillTriangle(x-34,y,x-35,y-22,x-19,y,WHITE);c_.fillTriangle(x-3,y,x+17,y+30,x+17,y,BLUE);for(int i=0;i<4;++i)c_.fillCircle(x-7+i*9,y,2,BLUE);}
    else {c_.fillTriangle(x-14,y+20,x,y+38,x+14,y+20,GOLD);c_.fillTriangle(x-16,y-5,x-31,y+24,x+31,y+24,PINK);c_.fillEllipse(x,y-4,18,32,WHITE);c_.fillCircle(x,y-11,10,BLUE);c_.fillCircle(x-3,y-14,3,MINT);}
  } else if(is(w,"bike")) {
    for(int s : {-1,1}){c_.drawCircle(x+s*27,y+15,19,BLUE);c_.drawCircle(x+s*27,y+15,17,BLUE);}
    c_.drawWideLine(x-27,y+15,x-10,y-12,3,GOLD);c_.drawWideLine(x-10,y-12,x+9,y+15,3,GOLD);c_.drawWideLine(x+9,y+15,x-27,y+15,3,GOLD);c_.drawWideLine(x-10,y-12,x+17,y-12,3,GOLD);c_.drawWideLine(x+17,y-12,x+9,y+15,3,GOLD);c_.drawWideLine(x+14,y-22,x+27,y+15,3,WHITE);c_.drawWideLine(x+14,y-22,x+25,y-24,3,WHITE);c_.drawWideLine(x-18,y-18,x-5,y-18,4,PINK);
  } else if(is(w,"cloud") || is(w,"snow")) {
    c_.fillRoundRect(x-34,y-11,68,26,12,WHITE);c_.fillCircle(x-12,y-13,19,WHITE);c_.fillCircle(x+13,y-15,16,WHITE);
    if(is(w,"snow"))for(int i=0;i<4;++i){int sx=x-27+i*18;for(int j=0;j<3;++j){float a=j*1.0472f;c_.drawLine(sx-cosf(a)*5,y+30-sinf(a)*5,sx+cosf(a)*5,y+30+sinf(a)*5,BLUE);}}
  } else if(is(w,"rainbow")) {
    uint16_t hues[]={PINK,PEACH,GOLD,MINT,BLUE,LILAC};
    for(int i=0;i<6;++i)c_.fillCircle(x,y+22,43-i*5,hues[i]);c_.fillCircle(x,y+22,13,BG);c_.fillRect(x-44,y+22,89,24,BG);
    c_.fillEllipse(x-33,y+22,17,7,WHITE);c_.fillEllipse(x+33,y+22,17,7,WHITE);
  } else if(is(w,"leaf")) {
    c_.fillEllipse(x,y-2,26,34,MINT);c_.drawWideLine(x,y-25,x,y+36,3,0x354d);
    for(int s : {-1,1})for(int i=0;i<3;++i)c_.drawLine(x,y-9+i*13,x+s*18,y-19+i*13,0x354d);
  } else if(is(w,"house")) {
    c_.fillRect(x-29,y-4,58,36,PEACH);c_.fillTriangle(x-41,y-4,x,y-36,x+41,y-4,PINK);
    c_.fillRoundRect(x-7,y+9,17,23,3,BLUE);c_.fillRect(x-23,y+4,11,11,MINT);c_.fillRect(x+16,y+4,9,11,MINT);
  } else if(is(w,"book")) {
    for(int s : {-1,1}){int bx=s<0?x-37:x+2;c_.fillRoundRect(bx,y-28,35,57,4,WHITE);for(int i=0;i<4;++i)c_.drawFastHLine(bx+6,y-12+i*10,22,BLUE);}
    c_.drawWideLine(x,y-26,x,y+32,3,GOLD);
  } else if(is(w,"bed") || is(w,"chair")) {
    if(is(w,"bed")){c_.fillRoundRect(x-40,y-4,80,27,5,BLUE);c_.fillRoundRect(x-32,y-11,26,16,5,WHITE);c_.fillRect(x-40,y-23,6,56,PEACH);c_.fillRect(x+34,y+9,6,24,PEACH);}
    else {c_.fillRoundRect(x-23,y-32,46,32,5,MINT);c_.fillRoundRect(x-26,y+3,52,9,3,PEACH);for(int s:{-1,1})c_.fillRect(x+s*20-3,y-5,6,38,PEACH);}
  } else if(is(w,"hat") || is(w,"shoe") || is(w,"sock")) {
    if(is(w,"hat")){c_.fillRoundRect(x-25,y-25,50,42,12,BLUE);c_.fillEllipse(x,y+18,43,9,BLUE);c_.fillRect(x-24,y+2,48,9,GOLD);}
    else if(is(w,"shoe")){c_.fillRoundRect(x-31,y-19,30,40,8,PEACH);c_.fillRoundRect(x-31,y+3,70,25,10,PEACH);c_.drawWideLine(x-30,y+27,x+34,y+27,5,WHITE);for(int i=0;i<3;++i)c_.drawWideLine(x-7+i*7,y+1,x-14+i*7,y+9,2,WHITE);}
    else {c_.fillRoundRect(x-12,y-32,27,50,5,LILAC);c_.fillRoundRect(x-32,y+5,47,26,12,LILAC);c_.fillRect(x-12,y-30,27,9,WHITE);}
  } else if(is(w,"kite")) {
    c_.fillTriangle(x,y-35,x-28,y-5,x,y+19,MINT);c_.fillTriangle(x,y-35,x+28,y-5,x,y+19,PINK);
    c_.drawLine(x,y+19,x+9,y+31,WHITE);c_.drawLine(x+9,y+31,x-2,y+37,WHITE);c_.fillTriangle(x+4,y+27,x+14,y+27,x+9,y+34,GOLD);
  } else if(is(w,"drum")) {
    c_.fillRect(x-31,y-10,62,36,PINK);c_.fillEllipse(x,y+25,31,8,PINK);c_.fillEllipse(x,y-10,31,11,WHITE);
    for(int i=0;i<4;++i){c_.drawLine(x-30+i*15,y-2,x-23+i*15,y+24,GOLD);c_.drawLine(x-23+i*15,y+24,x-16+i*15,y-2,GOLD);}
    c_.drawWideLine(x-31,y-32,x+4,y-7,4,PEACH);c_.drawWideLine(x+31,y-32,x-4,y-7,4,PEACH);
  } else if(is(w,"gift")) {
    c_.fillRoundRect(x-29,y-15,58,47,4,BLUE);c_.fillRect(x-33,y-20,66,12,MINT);c_.fillRect(x-4,y-20,8,52,GOLD);
    c_.drawEllipse(x-11,y-28,11,8,GOLD);c_.drawEllipse(x+11,y-28,11,8,GOLD);
  } else if(is(w,"clock")) {
    c_.fillCircle(x,y,33,PEACH);c_.fillCircle(x,y,28,WHITE);
    for(int i=0;i<12;++i)c_.fillCircle(x+cosf(i*.5236f)*23,y+sinf(i*.5236f)*23,2,BLUE);
    c_.drawWideLine(x,y,x,y-18,3,INK);c_.drawWideLine(x,y,x+13,y+8,3,INK);c_.fillCircle(x,y,3,PINK);
  } else if(is(w,"hand") || is(w,"clap")) {
    for(int h=0;h<(is(w,"clap")?2:1);++h){int hx=x+(is(w,"clap")?(h?19:-19):0);c_.fillRoundRect(hx-16,y-2,32,32,12,PEACH);for(int i=0;i<4;++i)c_.fillRoundRect(hx-16+i*9,y-30+abs(i-1)*4,7,37,3,PEACH);c_.drawWideLine(hx-14,y+17,hx-26,y-3,10,PEACH);}
    if(is(w,"clap"))for(int i=-1;i<=1;++i)c_.drawWideLine(x+i*13,y-32,x+i*19,y-40,2,GOLD);
  } else if(is(w,"foot")) {
    c_.fillEllipse(x,y+10,18,25,PEACH);for(int i=0;i<5;++i)c_.fillCircle(x-17+i*8,y-17+abs(i-1)*3,7-i/2,PEACH);
  } else if(is(w,"eye")) {
    c_.fillEllipse(x,y,39,24,WHITE);c_.fillCircle(x,y,20,BLUE);c_.fillCircle(x,y,11,INK);c_.fillCircle(x-6,y-6,5,WHITE);
  } else if(is(w,"ear")) {
    c_.fillEllipse(x,y-3,23,31,PEACH);c_.fillCircle(x-5,y+22,12,PEACH);c_.drawEllipse(x,y-4,14,22,PINK);c_.drawWideLine(x+9,y+1,x-3,y+8,5,PINK);
  } else if(is(w,"nose")) {
    c_.fillTriangle(x-16,y+18,x+16,y+18,x,y-32,PEACH);c_.fillEllipse(x,y+15,25,13,PEACH);c_.fillCircle(x-12,y+19,4,INK);c_.fillCircle(x+12,y+19,4,INK);
  } else if(is(w,"mouth")) {
    c_.fillEllipse(x,y,35,23,PINK);c_.fillEllipse(x,y,27,15,INK);c_.fillRoundRect(x-20,y-12,40,10,3,WHITE);c_.fillEllipse(x,y+8,15,5,PEACH);
  } else if(is(w,"pink") || is(w,"purple") || is(w,"white") || is(w,"black") || is(w,"brown")) {
    uint16_t color=is(w,"pink")?PINK:is(w,"purple")?LILAC:is(w,"white")?WHITE:is(w,"black")?0x0000:0x9b07;
    c_.fillCircle(x,y,32,WHITE);c_.fillCircle(x,y,29,color);c_.fillEllipse(x-13,y-14,3,6,is(w,"white")?BLUE:WHITE);
  } else if(is(w,"one") || is(w,"two") || is(w,"three") || is(w,"four") || is(w,"five") || is(w,"six") || is(w,"seven") || is(w,"eight") || is(w,"nine") || is(w,"ten")) {
    const char* numbers[]={"one","two","three","four","five","six","seven","eight","nine","ten"};int n=1;while(n<10 && !is(w,numbers[n-1]))++n;
    c_.setTextDatum(middle_center);c_.setTextFont(4);c_.setTextSize(2);c_.setTextColor(GOLD);c_.drawNumber(n,x,y-10);c_.setTextSize(1);
    for(int i=0;i<n;++i)c_.fillCircle(x-28+(i%5)*14,y+22+(i/5)*12,4,i<5?MINT:LILAC);
  } else if(is(w,"sleep") || is(w,"jump") || is(w,"big") || is(w,"small")) {
    int r=is(w,"small")?16:is(w,"big")?34:27;
    c_.fillCircle(x,y,r,MINT);face(x,y-4,is(w,"small")?6:10,is(w,"sleep"));
    if(is(w,"sleep")){c_.setTextDatum(middle_center);c_.setTextFont(2);c_.setTextColor(WHITE);c_.drawString("z",x+29,y-20);c_.drawString("Z",x+40,y-32);}
    if(is(w,"jump")){c_.drawWideLine(x-12,y+21,x-24,y+31,5,MINT);c_.drawWideLine(x+12,y+21,x+24,y+31,5,MINT);for(int s:{-1,1})c_.drawWideLine(x+s*37,y+17,x+s*37,y+30,2,GOLD);}
  } else return false;
  return true;
}
}
