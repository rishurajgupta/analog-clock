#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_TFTLCD.h> // Hardware-specific library
#include <dht.h>


#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0
#define XYZ A5

#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

#define BLACK   0x0000
#define	BLUE    0x001F
#define	RED     0xF800
#define	GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

float sx = 0, sy = 1, mx = 1, my = 0, hx = -1, hy = 0;    // Saved H, M, S x & y multipliers
float sdeg = 0, mdeg = 0, hdeg = 0;
uint16_t osx = 120, osy = 120, omx = 120, omy = 120, ohx = 120, ohy = 120; // Saved H, M, S x & y coords
int16_t x0 = 0, x1 = 0, yy0 = 0, yy1 = 0, x00 = 0, yy00 = 0;
uint32_t targetTime = 0;                    // for next 1 second timeout

uint16_t xpos; // x posisi jam
uint8_t conv2d(const char* p) {
  uint8_t v = 0;
  if ('0' <= *p && *p <= '9')
    v = *p - '0';
  return 10 * v + *++p - '0';
}
uint8_t hh = conv2d(__TIME__), mm = conv2d(__TIME__ + 3), ss = conv2d(__TIME__ + 6); // mengambil data waktu dari jam Compile-Upload
boolean initial = 1;
char d;

dht DHT;

void setup(void) {
  Serial.begin(9600);
  Serial.println(F("TFT LCD test"));

#ifdef USE_ADAFRUIT_SHIELD_PINOUT
  Serial.println(F("Using Adafruit 2.8\" TFT Arduino Shield Pinout"));
#else
  Serial.println(F("Using Adafruit 2.8\" TFT Breakout Board Pinout"));
#endif

  Serial.print("TFT size is "); Serial.print(tft.width()); Serial.print("x"); Serial.println(tft.height());

  tft.reset();

  uint16_t identifier = tft.readID();


  tft.begin(0x9325);

  tft.setRotation(1);

  tft.setTextColor(WHITE);//warna text
  tft.fillScreen(BLACK);//warna latar

  // Draw clock face
  xpos = tft.width() / 2; // mencari titik koordinat tengah LCD
  tft.drawCircle(xpos, 120, 125, YELLOW);
  tft.fillCircle(xpos, 120, 118, BLUE); //warna lingkaran luar
  tft.fillCircle(xpos, 120, 110, BLACK); //warna jam bagian dalam
  for (int a=95; a<104; a++){
  tft.drawCircle(xpos, 120, a, WHITE);} //warna lingkaran luar II

 

  // Draw 12 lines
  for (int i = 0; i < 360; i += 30) {
    sx = cos((i - 90) * 0.0174532925);
    sy = sin((i - 90) * 0.0174532925);
    x0 = sx * 114 + xpos;
    yy0 = sy * 114 + 120;
    x1 = sx * 100 + xpos;
    yy1 = sy * 100 + 120;

    tft.drawLine(x0, yy0, x1, yy1, YELLOW);//garis penanda angka jam
  }

  // Draw 60 dots
  for (int i = 0; i < 360; i += 6) {
    sx = cos((i - 90) * 0.0174532925);
    sy = sin((i - 90) * 0.0174532925);
    x0 = sx * 102 + xpos;
    yy0 = sy * 102 + 120;
    x00 = sx * 92 + xpos;
    yy00 = sy * 92 + 120;
    // Draw minute markers
    tft.drawPixel(x0, yy0, GREEN); //titik penanda menit
    tft.drawLine(x0, yy0, x00, yy00, BLACK);//garis penanda menit
    tft.drawLine(x0+1, yy0+1, x00+1, yy00+1, BLACK);//garis penanda menit

    // Draw main quadrant dots
    if (i == 0 || i == 180) tft.fillCircle(x0, yy0, 2, WHITE); //penanda 12 dan 6
    if (i == 90 || i == 270) tft.fillCircle(x0, yy0, 2, WHITE); //penanda 3 dan 9
  }

  tft.fillCircle(xpos, 121, 3, WHITE);
  targetTime = millis() + 1000;
  }

void loop(void) {
 /* for(uint8_t rotation=0; rotation<4; rotation++) {
    tft.setRotation(rotation);
    testText();
    delay(2000);

    
  }*/

    DHT.read11(XYZ);


  if (targetTime < millis()) {
    targetTime = millis() + 1000;
    ss++;              // Advance second
    if (ss == 60) {
      ss = 0;
      mm++;            // Advance minute
      if (mm > 59) {
        mm = 0;
        hh++;          // Advance hour
        if (hh > 23) {
          hh = 0;
        }
      }
    }

    // Pre-compute hand degrees, x & y coords for a fast screen update
    sdeg = ss * 6;                     // 0-59 -> 0-354
    mdeg = mm * 6 + sdeg * 0.01666667; // 0-59 -> 0-360 - includes seconds, but these increments are not used
    hdeg = hh * 30 + mdeg * 0.0833333; // 0-11 -> 0-360 - includes minutes and seconds, but these increments are not used
    hx = cos((hdeg - 90) * 0.0174532925);
    hy = sin((hdeg - 90) * 0.0174532925);
    mx = cos((mdeg - 90) * 0.0174532925);
    my = sin((mdeg - 90) * 0.0174532925);
    sx = cos((sdeg - 90) * 0.0174532925);
    sy = sin((sdeg - 90) * 0.0174532925);

    if (ss == 0 || initial) {
      initial = 0;
      // Erase hour and minute hand positions every minute
      tft.drawLine(ohx, ohy, xpos, 121, BLACK);
      ohx = hx * 62 + xpos + 1;
      ohy = hy * 62 + 121;
      tft.drawLine(omx, omy, xpos, 121, BLACK);
      omx = mx * 84 + xpos;
      omy = my * 84 + 121;
    }

    // Redraw new hand positions, hour and minute hands not erased here to avoid flicker
    tft.drawLine(osx, osy, xpos, 121, BLACK);
    osx = sx * 90 + xpos + 1;
    osy = sy * 90 + 121;
    tft.drawLine(osx, osy, xpos, 121, RED);
    tft.drawLine(ohx, ohy, xpos, 121, CYAN);
    tft.drawLine(omx, omy, xpos, 121, WHITE);
    tft.drawLine(osx, osy, xpos, 121, RED);
    tft.fillCircle(xpos, 121, 3, RED);

  tft.fillRect(xpos-40, 45,23,15,BLACK);
  tft.setCursor(xpos-40, 45);
  tft.setTextSize(2);
  tft.print(DHT.temperature); 
  tft.print("'C");
  
 
  // Draw MINI clock face "SECOND"
  tft.drawCircle(xpos, 155, 20, YELLOW);
  tft.drawCircle(xpos, 155, 18, BLUE);
  tft.drawCircle(xpos, 155, 17, CYAN);
  tft.drawCircle(xpos, 155, 16, CYAN);
  tft.fillRect(xpos-10, 149,22,15,BLACK); //erase
  if(ss<10){tft.setCursor(xpos-10, 149); tft.setTextSize(2);
  tft.print('0'); tft.setCursor(xpos+2, 149);}
  else{
  tft.setCursor(xpos-10, 149);}
  tft.setTextSize(2);
  tft.print(ss);
  
  // Draw MINI clock face "Minutes"
  tft.drawCircle(xpos+35, 117, 20,YELLOW);
  tft.drawCircle(xpos+35, 117, 18,BLUE);
  tft.drawCircle(xpos+35, 117, 17,CYAN);
  tft.drawCircle(xpos+35, 117, 16,CYAN);
 tft.fillRect(xpos+25, 111,22,15,BLACK); //erase
  if(mm<10){tft.setCursor(xpos+25, 111); tft.setTextSize(2);
  tft.print('0'); tft.setCursor(xpos+37, 111);}
  else{
  tft.setCursor(xpos+25, 111);}
  tft.println(mm);
  
  // Draw MINI clock face "Hour"
  tft.drawCircle(xpos-35, 117, 20,YELLOW);
  tft.drawCircle(xpos-35, 117, 18,BLUE);
  tft.drawCircle(xpos-35, 117, 17,CYAN);
  tft.drawCircle(xpos-35, 117, 16,CYAN);
  tft.fillRect(xpos-45, 111,22,15,BLACK); //erase
  if(hh<10){tft.setCursor(xpos-45, 111); tft.setTextSize(2);
  tft.print('0'); tft.setCursor(xpos-33, 111);}
  else{
  tft.setCursor(xpos-45, 111);}
  tft.setTextSize(2);
  tft.print(hh);
  //tft.setCursor(xpos-65, 111);
  //tft.println(':');

  if (hh>=0 && hh<12) d='A'; else {d='P';}
  tft.drawRoundRect(xpos-14,72,29,21,5,CYAN);
  tft.fillRect(xpos-11, 75,23,15,BLACK); //erase
  tft.setCursor(xpos-11, 75);
  tft.setTextSize(2);
  tft.print(d);
  tft.println('M');

  tft.fillRect(xpos-40, 182,23,15,BLACK);
  tft.setCursor(xpos-40, 182);
  tft.print(DHT.humidity);
  tft.print("%");
  
  }
}

