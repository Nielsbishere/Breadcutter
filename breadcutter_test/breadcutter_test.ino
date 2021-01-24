//Abstraction for a NEMA17 motor with an A3967 EasyDriver V44

enum Resolution {
  
  FULL_STEP,
  HALF_STEP,
  QUARTER_STEP,
  EIGHT_STEP,

  MS1 = 1,
  MS2 = 2
};

struct NEMA17 {

private:
  
  union {
    int outputs[5];
    struct {
      int enableBit;
      int ms1Bit;
      int ms2Bit;
      int stepBit;
      int reverseBit;
    };
  };

  Resolution r;     //Set to EIGHT_STEP by default if not initialized
  bool isReversed;
  bool isStep;
  bool isEnabled;
  
  bool hasInit = false;

  void setEnabled(bool _isEnabled) {

    if (isEnabled == _isEnabled && hasInit)
      return;

    isEnabled = _isEnabled;
    digitalWrite(enableBit, !isEnabled ? HIGH : LOW);    //Enable bit to high will disable, so reverse it
  }

public:

  static const int steps = 200;

  NEMA17(int enableBit, int ms1Bit, int ms2Bit, int stepBit, int reverseBit):
    enableBit(enableBit), ms1Bit(ms1Bit), ms2Bit(ms2Bit), stepBit(stepBit), reverseBit(reverseBit) { }

  void init(Resolution r, bool isReversed) {

    for(int i = 0; i < 5; ++i)
      pinMode(outputs[i], OUTPUT);
    
    setResolution(r);
    setDirection(isReversed);
    stopRotate();
    hasInit = true;
  }
  
  void setResolution(Resolution _r) {

    if (_r == r && hasInit)
      return;

    r = _r;
    
    digitalWrite(ms1Bit, r & MS1 ? HIGH : LOW);
    digitalWrite(ms2Bit, r & MS2 ? HIGH : LOW);
  }
  
  void setDirection(bool _isReversed) {

    if (isReversed == _isReversed && hasInit)
      return;

    isReversed = _isReversed;
    digitalWrite(reverseBit, isReversed ? HIGH : LOW);
  }

  void flipDirection() {
    setDirection(!isReversed);
  }

  void startRotate() {
    setEnabled(true);
    digitalWrite(stepBit, HIGH);
    isStep = true;
  }

  void stopRotate() {
    digitalWrite(stepBit, LOW);
    setEnabled(false);
    isStep = false;
  }

  void doStep(int dt) {
    startRotate();
    delay(dt);
    stopRotate();
    delay(dt);
  }
};

//Constants

static const int potentioSteps = 764;   //for the B10k, mostly 760 at max, but sometimes exceeds

//Globals

static int lastPotentio = 0;
static bool isOn = false;

static int potentio = A0;
static NEMA17 tray = NEMA17{ A1, A2, A3, A4, A5 };
static int knifeBit = 2;

void setup() { 
   tray.init(FULL_STEP, false);
   pinMode(knifeBit, OUTPUT);
   lastPotentio = analogRead(potentio);
}

void loop() {

  int currPotentio = analogRead(potentio);

  if(!isOn) {

    if((lastPotentio > potentioSteps / 2) != (currPotentio > potentioSteps / 2))
      isOn = true;
    
    else delay(10);    //wait 10ms to not waste too much power on this

    return;
  }

  lastPotentio = currPotentio;
  
  //Test the tray

  for(int i = 0; i < NEMA17::steps; ++i)
    tray.doStep(2);

  tray.flipDirection();

  for(int i = 0; i < NEMA17::steps; ++i)
    tray.doStep(2);

  tray.flipDirection();
  
  //Test the knife
  
  digitalWrite(knifeBit, HIGH);
  delay(1000);
  digitalWrite(knifeBit, LOW);
  
  //

  isOn = false;
}
