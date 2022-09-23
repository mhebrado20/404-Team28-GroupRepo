#ifndef _Screen_H
#define _Screen_H


// tft see https://learn.adafruit.com/adafruit-gfx-graphics-library/coordinate-system-and-units

#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
const byte TFT_DC_PIN = 5 ;
const byte TFT_CS_PIN = 4 ;
extern Adafruit_ILI9341 tft ;

// This is a hacked version of Paul Stoffregen's library to handle some
// rough touch screens. If yours works with the standard library, then use that instead.
#include "XPT2046_Touchscreen.h"


const byte TS_CS_PIN = 16 ;
extern XPT2046_Touchscreen ts ;

const byte pwmPin =  0 ;  //


enum NavBarType { NAVBARBOTTOM , FULLSCREEN } ;

const uint16_t screenWidth = 320 ;
const uint16_t screenHeight = 240 ;

const uint16_t buttonHeight = 40 ;
const uint16_t buttonMargin = 4 ;
const uint16_t buttonY = screenHeight - buttonHeight - buttonMargin ;
const uint16_t yTextStart = buttonY + 15 ;  // assume tft.setTextSize(  2  );
const uint16_t yButtonEnd = buttonY + buttonHeight ;

typedef void (*FunctionPointer)(byte);

typedef struct NavBarItem {
  uint16_t xButtonStart ;
  uint16_t xTextStart ;
  uint16_t xButtonEnd ;
  const char * text ;
  FunctionPointer callBack ;
  byte callBackPar ;
  bool active ;
}  NavBarItem ;


class NavBar
{

  public:

    static NavBar * ptrCurrentActiveNavBar ;

    static bool navBarSessionActive ;  // prevents interruption by locking


    NavBar( NavBarType navBarType  ) ;

    void Show() ;   // shows menu. Sets object active.

    void Clear() ;   // masks menu. Sets object inactive

    void UpdateNavBarItem( const char * text, FunctionPointer callBack, byte callBackPar,  bool active ) ;  // create/update navbar item.

    void EventWatcher() ;    // detects touch, tests if in range of a menu item and calls menu item's call back function.

  private:

    byte _cellCount ;
    byte _navBarIndex ;
    const NavBarType _navBarType ;
    bool _active ;
    bool _forceClearOnNextUpdate ;
    bool _hasChanged ;   // cleared by show(), set by UpdateNavBarItem() & Clear()
    uint16_t _buttonWidth ;
    NavBarItem _navBarItem[ 6 ] ;   // max is 6 (worst case) ?? use vector

} ;





// prototypes
float convTouchToScrnY( uint16_t touch ) ;
float convTouchToScrnX( uint16_t touch ) ;
bool inBox2( uint16_t xCurr,  uint16_t yCurr, uint16_t xTl,  uint16_t yTl,  uint16_t xBr ,  uint16_t yBr ) ;
void touchScreenCalibration() ;
uint16_t ICACHE_FLASH_ATTR getOutputLevel( uint16_t inputLevel ) ;
void setBacklightPwm () ;
void screen_setup() ;


#endif






