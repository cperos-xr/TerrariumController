#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_AHTX0.h>

#define SCREEN_W   128
#define SCREEN_H    32
#define OLED_ADDR   0x3C

// ── Timing ─────────────────────────────────────────────────────
const unsigned long MODE_TIME = 10000; // 10 s per mode
const unsigned long FRAME_MS  =   30;  // ~33 FPS for bloom, ~20 FPS elsewhere

// ── Display & Sensor ─────────────────────────────────────────
Adafruit_SSD1306 display(SCREEN_W, SCREEN_H, &Wire, -1);
Adafruit_AHTX0   aht;
unsigned long    startTime = 0, lastRead = 0;
float            lastHum = 0, lastTempF = 0;

// ── Shared Anim ──────────────────────────────────────────────
float phase = 0, dPhase = 0.1;
const float waveFreq = 2*PI/SCREEN_W, amplitude = 10;

// ── MODE 0: Vine + Hearts + Data Leaves ─────────────────────
const int starCount = 12;
int starX[starCount], starY[starCount];
const int spacing   = 32;
const int numLeaves = (SCREEN_W/spacing) + 2;
int leafX[numLeaves];
const int leafSize  = 8;
const int humIdx    = 1, tmpIdx = 2;

// ── MODE 1: Umbrellas + Gentle Rain ─────────────────────────
struct Umb { int16_t x,y; int8_t vx,vy; } umbrellas[3];
struct Raindot { int x,y; } rain[10];
const int IW = 12, IH = 8, R = 4, HW = 2;

// ── MODE 2: Bouncing & Spinning Bloom ───────────────────────
const int PETALS   = 8;
const int RADIUS   = 8;  // petal distance from center
const int PETAL_R  = 2;  // petal radius
const int BOX_W    = RADIUS * 2;
const int BOX_H    = RADIUS * 2;

int16_t bx = (SCREEN_W - BOX_W) / 2;
int16_t by = (SCREEN_H - BOX_H) / 2;
int8_t  vx2 = 2, vy2 = 1;
float  angle = 0, dAngle = 0.1;

// ── Mode Tracking ────────────────────────────────────────────
int lastMode = -1;

void setup() {
  Wire.begin(5,6);
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  if(!aht.begin()) while(1);
  randomSeed(micros());
  startTime = millis();

  // stars
  for(int i=0;i<starCount;i++){
    starX[i] = random(SCREEN_W);
    starY[i] = random(SCREEN_H/2);
  }
  // leaf X positions
  for(int i=0;i<numLeaves;i++){
    leafX[i] = i * spacing;
  }
  // umbrellas
  for(int i=0;i<3;i++){
    umbrellas[i].x  = random(SCREEN_W - IW);
    umbrellas[i].y  = random(SCREEN_H - IH);
    umbrellas[i].vx = random(1,3)*(random(2)?1:-1);
    umbrellas[i].vy = random(1,3)*(random(2)?1:-1);
  }
  // rain dots
  for(auto &r: rain){
    r.x = random(SCREEN_W);
    r.y = random(SCREEN_H);
  }
}

void drawVine() {
  // line
  int px=0, py=SCREEN_H/2 + sin(phase)*amplitude;
  for(int x=4;x<SCREEN_W;x+=4){
    int y = SCREEN_H/2 + sin(waveFreq*x+phase)*amplitude;
    display.drawLine(px,py,x,y,SSD1306_WHITE);
    px=x; py=y;
  }
  // stars
  for(int i=0;i<starCount;i++){
    if(random(10)>2) display.drawPixel(starX[i], starY[i], SSD1306_WHITE);
  }
  // hearts
  int pan = (millis() - startTime)/FRAME_MS;
  for(int i=0;i<numLeaves;i++){
    if(i==humIdx||i==tmpIdx) continue;
    int sx = (leafX[i] + pan) % SCREEN_W;
    if(sx<0) sx += SCREEN_W;
    int vy = SCREEN_H/2 + sin(waveFreq*sx+phase)*amplitude;
    int sign = (i%2)?1:-1;
    int hy = vy + sign*(leafSize/2 + 2);
    if(sign>0){
      display.fillCircle(sx - leafSize/3, hy - leafSize/4, leafSize/4, SSD1306_WHITE);
      display.fillCircle(sx + leafSize/3, hy - leafSize/4, leafSize/4, SSD1306_WHITE);
      display.fillTriangle(sx - leafSize/2, hy - leafSize/6,
                           sx + leafSize/2, hy - leafSize/6,
                           sx,              hy + leafSize/2,
                           SSD1306_WHITE);
    } else {
      display.fillCircle(sx - leafSize/3, hy + leafSize/4, leafSize/4, SSD1306_WHITE);
      display.fillCircle(sx + leafSize/3, hy + leafSize/4, leafSize/4, SSD1306_WHITE);
      display.fillTriangle(sx - leafSize/2, hy + leafSize/6,
                           sx + leafSize/2, hy + leafSize/6,
                           sx,              hy - leafSize/2,
                           SSD1306_WHITE);
    }
  }
  // data leaves on top
  char buf[6];
  for(int k=0;k<2;k++){
    int idx = (k==0?humIdx:tmpIdx);
    int sx  = (leafX[idx] + pan) % SCREEN_W;
    if(sx<0) sx += SCREEN_W;
    int vy0 = SCREEN_H/2 + sin(waveFreq*sx+phase)*amplitude;
    if(k==0) snprintf(buf,6,"H:%d%%",int(lastHum));
    else     snprintf(buf,6,"T:%dF",int(lastTempF));
    int tw=strlen(buf)*6, th=8;
    GFXcanvas1 c(tw,th);
    c.setTextSize(1); c.setTextColor(1);
    c.setCursor(0,th-8); c.print(buf);
    display.fillRect(sx - th/2 - 1, vy0 - tw/2 - 1, th+2, tw+2, SSD1306_BLACK);
    int cx=tw/2, cy=th/2;
    for(int ix=0;ix<tw;ix++) for(int iy=0;iy<th;iy++){
      if(c.getPixel(ix,iy)){
        int xr=sx+(iy-cy), yr=vy0-(ix-cx);
        if(xr>=0&&xr<SCREEN_W&&yr>=0&&yr<SCREEN_H)
          display.drawPixel(xr,yr,SSD1306_WHITE);
      }
    }
  }
}

void drawUmbrella(Umb &u){
  // gentle rain → (1px)
  for(auto &r: rain){
    display.drawPixel(r.x, r.y, SSD1306_WHITE);
    r.x += 1; 
    if(r.x>=SCREEN_W){
      r.x=0;
      r.y=random(SCREEN_H);
    }
  }
  int16_t cy=u.y+IH/2, cx=u.x+R;
  display.fillCircle(cx,cy,R,SSD1306_WHITE);
  display.fillRect(u.x+R,u.y,R,IH,SSD1306_BLACK);
  display.fillRect(u.x+R,cy-HW/2,IW-R,HW,SSD1306_WHITE);
}

void drawBloomMode(){
  // clear & draw bloom
  int16_t cx = bx + RADIUS;
  int16_t cy = by + RADIUS;
  for(int i=0;i<PETALS;i++){
    float a = angle + i*(TWO_PI/PETALS);
    int16_t px = cx + round(cos(a)*RADIUS);
    int16_t py = cy + round(sin(a)*RADIUS);
    display.fillCircle(px,py,PETAL_R,SSD1306_WHITE);
  }
  display.fillCircle(cx,cy,PETAL_R,SSD1306_WHITE);

  // bounce
  bx += vx2; by += vy2;
  if(bx <= 0 || bx >= SCREEN_W - BOX_W){ vx2 = -vx2; bx += vx2; }
  if(by <= 0 || by >= SCREEN_H - BOX_H){ vy2 = -vy2; by += vy2; }

  // spin
  angle += dAngle;
  if(angle > TWO_PI) angle -= TWO_PI;
}

void loop(){
  unsigned long now=millis();
  // sensor every 3 s
  if(now - lastRead > 3000){
    lastRead = now;
    sensors_event_t eh, et; aht.getEvent(&eh,&et);
    lastHum   = eh.relative_humidity;
    lastTempF = et.temperature * 9.0/5.0 + 32.0;
  }

  display.clearDisplay();
  int mode = ((now - startTime)/MODE_TIME)%3;

  switch(mode){
    case 0: drawVine();      break;
    case 1:
      for(auto &u: umbrellas) drawUmbrella(u), u.x+=u.vx, u.y+=u.vy,
        (u.x<=0||u.x>=SCREEN_W-IW)?(u.vx=-u.vx, u.x+=u.vx):0,
        (u.y<=0||u.y>=SCREEN_H-IH)?(u.vy=-u.vy, u.y+=u.vy):0;
      break;
    case 2:
      drawBloomMode();
      break;
  }

  display.display();
  phase -= dPhase; if(phase<0) phase += TWO_PI;
  delay(FRAME_MS);
}
