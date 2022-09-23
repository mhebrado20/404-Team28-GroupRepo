#include "Screen.h"
#include "Config.h"
#include "Globals.h"

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS_PIN, TFT_DC_PIN);
XPT2046_Touchscreen ts(TS_CS_PIN);


float convTouchToScrnY( uint16_t touch ) {
  // return ( (float)touch - config.yShift ) * (float)config.yScale ;
  return (float)touch * (float)config.yScale + config.yShift ;
}

float convTouchToScrnX( uint16_t touch ) {
  // return ( (float)touch  - config.xShift ) * (float)config.xScale ;
  return (float)touch * (float)config.xScale + config.xShift ;
}


bool inBox2( uint16_t xCurr,  uint16_t yCurr, uint16_t xTl,  uint16_t yTl,  uint16_t xBr ,  uint16_t yBr ) {
  bool in = (  xCurr > xTl && xCurr < xBr && yCurr > yTl && yCurr < yBr ) ;
  if ( debugMode > 1 ) {
    swSerial.print("xCurr: "); swSerial.print(xCurr); swSerial.print("  ");
    swSerial.print("yCurr: "); swSerial.print(yCurr); swSerial.print("  ");
    swSerial.print("xTl: "); swSerial.print(xTl); swSerial.print("  ");
    swSerial.print("yTl: "); swSerial.print(yTl); swSerial.print("  ");
    swSerial.print("xBr: "); swSerial.print(xBr); swSerial.print("  ");
    swSerial.print("yBr: "); swSerial.print(yBr); swSerial.print("  ");
    swSerial.print("inBox: "); swSerial.print(in); swSerial.print("  ");
    swSerial.println( );
  }
  return  in  ;
}




void  ICACHE_FLASH_ATTR touchScreenCalibration() {
  /*
      User must confirm 2 points on the screen
      This set global x/y scale & x/y shift.
  */
  swSerial.println(F("in touchScreenCalibration()" ) );

  TS_Point p1, p2 ;
  int x1 = 25, y1 = 25, x2 = 295, y2 = 215;

  swSerial.print(  "\nTestpoint x1  = " ) ; swSerial.print( x1 ) ;
  swSerial.print(  "\nTestpoint y1  = " ) ; swSerial.print( y1 ) ;
  swSerial.print(  "\nTestpoint x2  = " ) ; swSerial.print( x2 ) ;
  swSerial.print(  "\nTestpoint y2  = " ) ; swSerial.print( y2 ) ;
  swSerial.println() ;

  boolean istouched ;

  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(  2  );

  // user must tap screen within x seconds to enter calibration
  tft.setCursor( 0, 20 ) ;
  tft.println("Tap screen to calibrate.") ;
  tft.println("Waiting 5 seconds.") ;

  int j ;
  for ( j = 0 ; j <= 100 ; j++ ) {   // 100 * 50mS = 5 sec.
    delay( 50 ) ;
    if ( ts.touched() ) {
      break;
    }
  }
  if ( j >= 100 ) {
    tft.println("Calibration skipped.") ;
    delay( 1000 ) ;
    return ;
  }
  tft.fillScreen(ILI9341_BLACK);



  tft.setTextSize(  2  );
  tft.setCursor( x1 + 20, y1 - 7 );
  tft.print("<= click in centre");
  tft.drawCircle( x1, y1, 15, ILI9341_WHITE ) ;
  tft.drawCircle( x1, y1, 4, ILI9341_WHITE ) ;
  tft.fillCircle( x1, y1, 4, ILI9341_WHITE ) ;



  // v0.73
  for ( j = 0 ; j <= 200 ; j++ ) {   // 100 * 50mS = 5 sec.
    delay( 50 ) ;
    istouched = ts.touched();
    p1 = ts.getPoint();
    if ( istouched ) break;
  }
  if ( j >= 200 ) {
    tft.fillScreen(ILI9341_BLACK);
    tft.setCursor( 0, 20 ) ;
    tft.println("Timeout: ") ;
    tft.println("Calibration aborted.") ;
    delay( 2000 ) ;
    return ;
  }


  p1 = ts.getPoint();
  p1 = ts.getPoint();
  p1 = ts.getPoint();  // stable ?


  tft.fillScreen(ILI9341_BLACK);

  tft.setCursor( 20 , 120 );
  tft.print("please wait . . .");
  delay(1500) ;


  tft.setCursor( 60, y2 - 7 );
  tft.print("click in center =>");
  tft.drawCircle( x2, y2, 15, ILI9341_WHITE ) ;
  tft.drawCircle( x2, y2, 4, ILI9341_WHITE ) ;
  tft.fillCircle( x2, y2, 4, ILI9341_WHITE ) ;


  // v0.73
  for ( j = 0 ; j <= 200 ; j++ ) {   // 100 * 50mS = 5 sec.
    delay( 50 ) ;
    istouched = ts.touched();
    p2 = ts.getPoint();
    if ( istouched ) break;
  }
  if ( j >= 200 ) {
    tft.fillScreen(ILI9341_BLACK);
    tft.setCursor( 0, 20 ) ;
    tft.println("Timeout: ") ;
    tft.println("Calibration aborted.") ;
    delay( 2000 ) ;
    return ;
  }

  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor( 0, 20 ) ;
  tft.println("Screen calibration") ;
  tft.println(" completed.") ;


  p2 = ts.getPoint();
  p2 = ts.getPoint();
  p2 = ts.getPoint(); // stable ?

  swSerial.print(  "\nTS point x1  = " ) ; swSerial.print( p1.x ) ;
  swSerial.print(  "\nTS point y1  = " ) ; swSerial.print( p1.y ) ;
  swSerial.print(  "\nTS point x2  = " ) ; swSerial.print( p2.x ) ;
  swSerial.print(  "\nTS point y2  = " ) ; swSerial.print( p2.y ) ;
  swSerial.println() ;


  config.xScale = ( (float)x2 - x1 ) / ( (float)p2.x - p1.x ) ;
  config.xShift = (float)x1 - ( (float)p1.x * config.xScale ) ;

  config.yScale = ( (float)y2 - y1 ) / ( (float)p2.y - p1.y ) ;
  config.yShift = (float)y1 - ( (float)p1.y * config.yScale ) ;



  swSerial.print(  "\nxScale = " ) ; swSerial.print( config.xScale ) ;
  swSerial.print(  "\nxShift = " ) ; swSerial.print( config.xShift ) ;
  swSerial.print(  "\nyScale = " ) ; swSerial.print( config.yScale ) ;
  swSerial.print(  "\nyShift = " ) ; swSerial.print( config.yShift ) ;
  swSerial.println() ;
  eepromDump() ; // resave to eeprom
  eepromFetch() ;
  PrintEepromVariables() ;
  delay( 1000 ) ; // time to display completed message.
}

// ========================================================================



NavBar::NavBar( NavBarType navBarType  ) :

  _navBarType( navBarType ), _active ( false ), _hasChanged ( true ) ,
  _cellCount ( 0 ), _navBarIndex ( 0 ) ,  _forceClearOnNextUpdate ( true )  {
}


void NavBar::Show() {

  if (  _navBarIndex > 0 ) {

    if ( _hasChanged ) {
      // rebuild it
      _buttonWidth = ( screenWidth - (  _navBarIndex + 1 ) * buttonMargin ) /  _navBarIndex ;
      for ( byte i = 0 ; i <  _navBarIndex ; i ++ ) {
        _navBarItem[ i ].xButtonStart = buttonMargin + ( _buttonWidth + buttonMargin ) * i ;
        _navBarItem[ i ].xTextStart = _navBarItem[ i ].xButtonStart + 4 ; // check
        _navBarItem[ i ].xButtonEnd = _navBarItem[ i ].xButtonStart + _buttonWidth ;
      }
    }

    tft.setTextSize(  2  );
    tft.fillRect(  buttonMargin, buttonY, (screenWidth - 2 * buttonMargin ) , buttonHeight, ILI9341_BLACK ) ; // clean menu area
    for ( byte i = 0 ; i <  _navBarIndex ; i ++ ) {

      if ( _navBarItem[ i ].active )  {
        tft.drawRoundRect( _navBarItem[ i ].xButtonStart , buttonY, _buttonWidth, buttonHeight, 10, ILI9341_WHITE );
        tft.setCursor( _navBarItem[ i ].xTextStart , yTextStart );
        tft.print( _navBarItem[ i ].text  );
      }
      else  {
        tft.drawRoundRect( _navBarItem[ i ].xButtonStart , buttonY, _buttonWidth, buttonHeight, 10, ILI9341_DARKGREY );
        tft.setCursor( _navBarItem[ i ].xTextStart , yTextStart );
        tft.setTextColor( ILI9341_DARKGREY );  // gray
        tft.print( _navBarItem[ i ].text  );
        tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);  // back to (current) default
      }

    }
    _hasChanged = false;
    _forceClearOnNextUpdate = true ;
    NavBar::ptrCurrentActiveNavBar = this ;
    _active = true ;
  }
}



void NavBar::Clear() {
  // clean up
  _active = false ;
  _hasChanged = true ;
  _forceClearOnNextUpdate = false ;
  _cellCount = 0 ;
  _navBarIndex = 0 ;
  tft.fillRect(  buttonMargin, buttonY, (screenWidth - 2 * buttonMargin ) , buttonHeight, ILI9341_BLACK ) ;  // clean menu area
}


void NavBar::UpdateNavBarItem( const char * text, FunctionPointer callBack, byte callBackPar, bool active ) {
  // user must padd leading spaces and ensure text is not too long.

  if ( _forceClearOnNextUpdate ) {
    Clear() ;
    _forceClearOnNextUpdate = false ;
  }
  if ( _navBarIndex < 6 ) {
    _navBarItem[ _navBarIndex ].text = text ;
    _navBarItem[ _navBarIndex ].callBack = callBack ;
    _navBarItem[ _navBarIndex ].callBackPar = callBackPar ;
    _navBarItem[ _navBarIndex ].active = active ;
    _navBarIndex++ ;
  }
}



void NavBar::EventWatcher() {
  // called from loop say every 100mS
  if ( _active  ) {    // temp v0.23
    bool isTouched ;
    isTouched = ts.touched() ;
    // isTouched = ts.touched() ;  // atempt to stabilise
    if ( isTouched ) {    //( isTouched && millis() - lastTouchTestAtMs > 100 ) {
      swSerial.println("Is Touched" ) ;
      TS_Point p;
      // p = ts.getPoint();
      // p = ts.getPoint();
      // p = ts.getPoint();
      p = ts.getPoint(); // stable ??
      uint16_t calcY = convTouchToScrnY( p.y ) ;
      uint16_t calcX = convTouchToScrnX( p.x ) ;
      for ( byte i = 0 ; i < _navBarIndex ; i ++ ) {
        if ( inBox2( calcX , calcY , _navBarItem[ i ].xButtonStart , buttonY,   _navBarItem[ i ].xButtonEnd , yButtonEnd  )) {



          if ( _navBarItem[ i ].callBack != NULL &&  _navBarItem[ i ].active ) {

            // some visual effect on active button press
            tft.fillRoundRect( _navBarItem[ i ].xButtonStart , buttonY, _buttonWidth, buttonHeight, 10, ILI9341_WHITE );
            tft.setCursor( _navBarItem[ i ].xTextStart , yTextStart );
            tft.setTextColor( ILI9341_BLACK ) ;
            tft.print( _navBarItem[ i ].text  );
            delay( 100 ) ;  // review
            tft.fillRoundRect( _navBarItem[ i ].xButtonStart , buttonY, _buttonWidth, buttonHeight, 10, ILI9341_BLACK );
            tft.drawRoundRect( _navBarItem[ i ].xButtonStart , buttonY, _buttonWidth, buttonHeight, 10, ILI9341_WHITE );
            tft.setCursor( _navBarItem[ i ].xTextStart , yTextStart );
            tft.setTextColor( ILI9341_WHITE ) ;
            tft.print( _navBarItem[ i ].text  );

            _navBarItem[ i ].callBack(  _navBarItem[ i ].callBackPar ) ;  // execute call back

          }
          else swSerial.println( F("\nNULL pointer in NavBar::EventWatcher() or inactive navbar item selected" )) ;
          break ;  // one is enough  v0.44b
        }
      }
      if ( debugMode > 1 ) {
        tft.drawCircle( calcX, calcY, 5, ILI9341_WHITE ) ;  // v0.75 show point touched
        tft.drawCircle( calcX, calcY, 2, ILI9341_BLACK ) ;  // v0.75 show point touched
      }
    }
  }
}


//static member init

NavBar * NavBar::ptrCurrentActiveNavBar = NULL ;
bool NavBar::navBarSessionActive = false ;



// ========================================================================





uint16_t ICACHE_FLASH_ATTR getOutputLevel( uint16_t inputLevel ) {
  //
  //  convert an input value into an output value with hysteresis to
  //  avoid instability at cutover points ( eliminate thrashing/flicker etc.)
  //
  //  6v6gt 03.feb.2018

  // adjust these 4 constants to suit your application

  // ========================================
  // margin sets the 'stickyness' of the hysteresis or the relucatance to leave the current state.
  // It is measured in units of the the input level. As a guide it is a few percent of the
  // difference between two end points. Don't make the margin too wide or ranges may overlap.
  const uint16_t margin = 10 ;   //  +/- 10

  // set the number of output levels. These are numbered starting from 0.
  const uint16_t numberOfLevelsOutput = 2 ;  // 0..1


  // define input to output conversion table/formula by specifying endpoints of the levels.
  // the number of end points is equal to the number of output levels plus one.
  // in the example below, output level 0 results from an input of between 0 and 550.
  // 1 results from an input of between 551 and 1023 etc.
  const uint16_t endPointInput[ numberOfLevelsOutput + 1 ] = { 0, 550, 1023 } ;

  // initial output level (usually zero)
  const  uint16_t initialOutputLevel = 0 ;
  // ========================================


  // the current output level is retained for the next calculation.
  // Note: initial value of a static variable is set at compile time.
  static uint16_t currentOutputLevel = initialOutputLevel ;

  // get lower and upper bounds for currentOutputLevel
  uint16_t lb = endPointInput[ currentOutputLevel ] ;
  if ( currentOutputLevel > 0 ) lb -= margin  ;   // subtract margin

  uint16_t ub = endPointInput[ currentOutputLevel + 1 ] ;
  if ( currentOutputLevel < numberOfLevelsOutput ) ub +=  margin  ;  // add margin

  // now test if input is between the outer margins for current output value
  if ( inputLevel < lb || inputLevel > ub ) {
    // determine new output level by scanning endPointInput array
    uint16_t i;
    for ( i = 0 ; i < numberOfLevelsOutput ; i++ ) {
      if ( inputLevel >= endPointInput[ i ] && inputLevel <= endPointInput[ i + 1 ] ) break ;
    }
    currentOutputLevel = i ;
  }
  return currentOutputLevel ;
}




void ICACHE_FLASH_ATTR setBacklightPwm () {
  //
  //
  //
  // v0.74 - should be called direct from loop()
  digitalWrite( pwmPin , ( getOutputLevel( analogRead( A0 )  ) == 1  ?  HIGH : LOW   ) ) ;
}



void  ICACHE_FLASH_ATTR  screen_setup() {
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  ts.begin();
  pinMode ( pwmPin, OUTPUT ) ;
}










