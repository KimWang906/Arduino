# 1 "c:\\Users\\user\\Desktop\\arduino\\DFRobotDFPlayerMini.cpp"
/*!

 * @file DFRobotDFPlayerMini.cpp

 * @brief DFPlayer - An Arduino Mini MP3 Player From DFRobot

 * @n Header file for DFRobot's DFPlayer

 *

 * @copyright	[DFRobot]( http://www.dfrobot.com ), 2016

 * @copyright	GNU Lesser General Public License

 *

 * @author [Angelo](Angelo.qiao@dfrobot.com)

 * @version  V1.0.3

 * @date  2016-12-07

 */
# 14 "c:\\Users\\user\\Desktop\\arduino\\DFRobotDFPlayerMini.cpp"
# 15 "c:\\Users\\user\\Desktop\\arduino\\DFRobotDFPlayerMini.cpp" 2

void DFRobotDFPlayerMini::setTimeOut(unsigned long timeOutDuration){
  _timeOutDuration = timeOutDuration;
}

void DFRobotDFPlayerMini::uint16ToArray(uint16_t value, uint8_t *array){
  *array = (uint8_t)(value>>8);
  *(array+1) = (uint8_t)(value);
}

uint16_t DFRobotDFPlayerMini::calculateCheckSum(uint8_t *buffer){
  uint16_t sum = 0;
  for (int i=1; i<7; i++) {
    sum += buffer[i];
  }
  return -sum;
}

void DFRobotDFPlayerMini::sendStack(){
  if (_sending[4]) { //if the ack mode is on wait until the last transmition
    while (_isSending) {
      delay(0);
      available();
    }
  }
# 50 "c:\\Users\\user\\Desktop\\arduino\\DFRobotDFPlayerMini.cpp"
  _serial->write(_sending, 10);
  _timeOutTimer = millis();
  _isSending = _sending[4];

  if (!_sending[4]) { //if the ack mode is off wait 10 ms after one transmition.
    delay(10);
  }
}

void DFRobotDFPlayerMini::sendStack(uint8_t command){
  sendStack(command, 0);
}

void DFRobotDFPlayerMini::sendStack(uint8_t command, uint16_t argument){
  _sending[3] = command;
  uint16ToArray(argument, _sending+5);
  uint16ToArray(calculateCheckSum(_sending), _sending+7);
  sendStack();
}

void DFRobotDFPlayerMini::sendStack(uint8_t command, uint8_t argumentHigh, uint8_t argumentLow){
  uint16_t buffer = argumentHigh;
  buffer <<= 8;
  sendStack(command, buffer | argumentLow);
}

void DFRobotDFPlayerMini::enableACK(){
  _sending[4] = 0x01;
}

void DFRobotDFPlayerMini::disableACK(){
  _sending[4] = 0x00;
}

bool DFRobotDFPlayerMini::waitAvailable(unsigned long duration){
  unsigned long timer = millis();
  if (!duration) {
    duration = _timeOutDuration;
  }
  while (!available()){
    if (millis() - timer > duration) {
      return false;
    }
    delay(0);
  }
  return true;
}

bool DFRobotDFPlayerMini::begin(Stream &stream, bool isACK, bool doReset){
  _serial = &stream;

  if (isACK) {
    enableACK();
  }
  else{
    disableACK();
  }

  if (doReset) {
    reset();
    waitAvailable(2000);
    delay(200);
  }
  else {
    // assume same state as with reset(): online
    _handleType = 4;
  }

  return (readType() == 4) || (readType() == 9) || !isACK;
}

uint8_t DFRobotDFPlayerMini::readType(){
  _isAvailable = false;
  return _handleType;
}

uint16_t DFRobotDFPlayerMini::read(){
  _isAvailable = false;
  return _handleParameter;
}

bool DFRobotDFPlayerMini::handleMessage(uint8_t type, uint16_t parameter){
  _receivedIndex = 0;
  _handleType = type;
  _handleParameter = parameter;
  _isAvailable = true;
  return _isAvailable;
}

bool DFRobotDFPlayerMini::handleError(uint8_t type, uint16_t parameter){
  handleMessage(type, parameter);
  _isSending = false;
  return false;
}

uint8_t DFRobotDFPlayerMini::readCommand(){
  _isAvailable = false;
  return _handleCommand;
}

void DFRobotDFPlayerMini::parseStack(){
  uint8_t handleCommand = *(_received + 3);
  if (handleCommand == 0x41) { //handle the 0x41 ack feedback as a spcecial case, in case the pollusion of _handleCommand, _handleParameter, and _handleType.
    _isSending = false;
    return;
  }

  _handleCommand = handleCommand;
  _handleParameter = arrayToUint16(_received + 5);

  switch (_handleCommand) {
    case 0x3D:
      handleMessage(5, _handleParameter);
      break;
    case 0x3F:
      if (_handleParameter & 0x01) {
        handleMessage(9, _handleParameter);
      }
      else if (_handleParameter & 0x02) {
        handleMessage(4, _handleParameter);
      }
      else if (_handleParameter & 0x03) {
        handleMessage(10, _handleParameter);
      }
      break;
    case 0x3A:
      if (_handleParameter & 0x01) {
        handleMessage(7, _handleParameter);
      }
      else if (_handleParameter & 0x02) {
        handleMessage(2, _handleParameter);
      }
      break;
    case 0x3B:
      if (_handleParameter & 0x01) {
        handleMessage(8, _handleParameter);
      }
      else if (_handleParameter & 0x02) {
        handleMessage(3, _handleParameter);
      }
      break;
    case 0x40:
      handleMessage(6, _handleParameter);
      break;
    case 0x3C:
    case 0x3E:
    case 0x42:
    case 0x43:
    case 0x44:
    case 0x45:
    case 0x46:
    case 0x47:
    case 0x48:
    case 0x49:
    case 0x4B:
    case 0x4C:
    case 0x4D:
    case 0x4E:
    case 0x4F:
      handleMessage(11, _handleParameter);
      break;
    default:
      handleError(1);
      break;
  }
}

uint16_t DFRobotDFPlayerMini::arrayToUint16(uint8_t *array){
  uint16_t value = *array;
  value <<=8;
  value += *(array+1);
  return value;
}

bool DFRobotDFPlayerMini::validateStack(){
  return calculateCheckSum(_received) == arrayToUint16(_received+7);
}

bool DFRobotDFPlayerMini::available(){
  while (_serial->available()) {
    delay(0);
    if (_receivedIndex == 0) {
      _received[0] = _serial->read();





      if (_received[0] == 0x7E) {
        _receivedIndex ++;
      }
    }
    else{
      _received[_receivedIndex] = _serial->read();




      switch (_receivedIndex) {
        case 1:
          if (_received[_receivedIndex] != 0xFF) {
            return handleError(1);
          }
          break;
        case 2:
          if (_received[_receivedIndex] != 0x06) {
            return handleError(1);
          }
          break;
        case 9:



          if (_received[_receivedIndex] != 0xEF) {
            return handleError(1);
          }
          else{
            if (validateStack()) {
              _receivedIndex = 0;
              parseStack();
              return _isAvailable;
            }
            else{
              return handleError(1);
            }
          }
          break;
        default:
          break;
      }
      _receivedIndex++;
    }
  }

  if (_isSending && (millis()-_timeOutTimer>=_timeOutDuration)) {
    return handleError(0);
  }

  return _isAvailable;
}

void DFRobotDFPlayerMini::next(){
  sendStack(0x01);
}

void DFRobotDFPlayerMini::previous(){
  sendStack(0x02);
}

void DFRobotDFPlayerMini::play(int fileNumber){
  sendStack(0x03, fileNumber);
}

void DFRobotDFPlayerMini::volumeUp(){
  sendStack(0x04);
}

void DFRobotDFPlayerMini::volumeDown(){
  sendStack(0x05);
}

void DFRobotDFPlayerMini::volume(uint8_t volume){
  sendStack(0x06, volume);
}

void DFRobotDFPlayerMini::EQ(uint8_t eq) {
  sendStack(0x07, eq);
}

void DFRobotDFPlayerMini::loop(int fileNumber) {
  sendStack(0x08, fileNumber);
}

void DFRobotDFPlayerMini::outputDevice(uint8_t device) {
  sendStack(0x09, device);
  delay(200);
}

void DFRobotDFPlayerMini::sleep(){
  sendStack(0x0A);
}

void DFRobotDFPlayerMini::reset(){
  sendStack(0x0C);
}

void DFRobotDFPlayerMini::start(){
  sendStack(0x0D);
}

void DFRobotDFPlayerMini::pause(){
  sendStack(0x0E);
}

void DFRobotDFPlayerMini::playFolder(uint8_t folderNumber, uint8_t fileNumber){
  sendStack(0x0F, folderNumber, fileNumber);
}

void DFRobotDFPlayerMini::outputSetting(bool enable, uint8_t gain){
  sendStack(0x10, enable, gain);
}

void DFRobotDFPlayerMini::enableLoopAll(){
  sendStack(0x11, 0x01);
}

void DFRobotDFPlayerMini::disableLoopAll(){
  sendStack(0x11, 0x00);
}

void DFRobotDFPlayerMini::playMp3Folder(int fileNumber){
  sendStack(0x12, fileNumber);
}

void DFRobotDFPlayerMini::advertise(int fileNumber){
  sendStack(0x13, fileNumber);
}

void DFRobotDFPlayerMini::playLargeFolder(uint8_t folderNumber, uint16_t fileNumber){
  sendStack(0x14, (((uint16_t)folderNumber) << 12) | fileNumber);
}

void DFRobotDFPlayerMini::stopAdvertise(){
  sendStack(0x15);
}

void DFRobotDFPlayerMini::stop(){
  sendStack(0x16);
}

void DFRobotDFPlayerMini::loopFolder(int folderNumber){
  sendStack(0x17, folderNumber);
}

void DFRobotDFPlayerMini::randomAll(){
  sendStack(0x18);
}

void DFRobotDFPlayerMini::enableLoop(){
  sendStack(0x19, 0x00);
}

void DFRobotDFPlayerMini::disableLoop(){
  sendStack(0x19, 0x01);
}

void DFRobotDFPlayerMini::enableDAC(){
  sendStack(0x1A, 0x00);
}

void DFRobotDFPlayerMini::disableDAC(){
  sendStack(0x1A, 0x01);
}

int DFRobotDFPlayerMini::readState(){
  sendStack(0x42);
  if (waitAvailable()) {
    if (readType() == 11) {
      return read();
    }
    else{
      return -1;
    }
  }
  else{
    return -1;
  }
}

int DFRobotDFPlayerMini::readVolume(){
  sendStack(0x43);
  if (waitAvailable()) {
    return read();
  }
  else{
    return -1;
  }
}

int DFRobotDFPlayerMini::readEQ(){
  sendStack(0x44);
  if (waitAvailable()) {
    if (readType() == 11) {
      return read();
    }
    else{
      return -1;
    }
  }
  else{
    return -1;
  }
}

int DFRobotDFPlayerMini::readFileCounts(uint8_t device){
  switch (device) {
    case 1:
      sendStack(0x47);
      break;
    case 2:
      sendStack(0x48);
      break;
    case 5:
      sendStack(0x49);
      break;
    default:
      break;
  }

  if (waitAvailable()) {
    if (readType() == 11) {
      return read();
    }
    else{
      return -1;
    }
  }
  else{
    return -1;
  }
}

int DFRobotDFPlayerMini::readCurrentFileNumber(uint8_t device){
  switch (device) {
    case 1:
      sendStack(0x4B);
      break;
    case 2:
      sendStack(0x4C);
      break;
    case 5:
      sendStack(0x4D);
      break;
    default:
      break;
  }
  if (waitAvailable()) {
    if (readType() == 11) {
      return read();
    }
    else{
      return -1;
    }
  }
  else{
    return -1;
  }
}

int DFRobotDFPlayerMini::readFileCountsInFolder(int folderNumber){
  sendStack(0x4E, folderNumber);
  if (waitAvailable()) {
    if (readType() == 11) {
      return read();
    }
    else{
      return -1;
    }
  }
  else{
    return -1;
  }
}

int DFRobotDFPlayerMini::readFolderCounts(){
  sendStack(0x4F);
  if (waitAvailable()) {
    if (readType() == 11) {
      return read();
    }
    else{
      return -1;
    }
  }
  else{
    return -1;
  }
}

int DFRobotDFPlayerMini::readFileCounts(){
  return readFileCounts(2);
}

int DFRobotDFPlayerMini::readCurrentFileNumber(){
  return readCurrentFileNumber(2);
}
# 1 "c:\\Users\\user\\Desktop\\arduino\\test.ino"






# 8 "c:\\Users\\user\\Desktop\\arduino\\test.ino" 2
DFRobotDFPlayerMini myDFPlayer;
// Potentiometer is connected to GPIO 34 (Analog ADC1_CH6) 

// variable for storing the potentiometer value
int potValue = 0;

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600);

  if (!myDFPlayer.begin(Serial2)) { //Use softwareSerial to communicate with mp3.
    Serial.println((reinterpret_cast<const __FlashStringHelper *>(
# 19 "c:\\Users\\user\\Desktop\\arduino\\test.ino" 3
                  (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 19 "c:\\Users\\user\\Desktop\\arduino\\test.ino"
                  "Unable to begin:"
# 19 "c:\\Users\\user\\Desktop\\arduino\\test.ino" 3
                  ); &__c[0];}))
# 19 "c:\\Users\\user\\Desktop\\arduino\\test.ino"
                  )));
    Serial.println((reinterpret_cast<const __FlashStringHelper *>(
# 20 "c:\\Users\\user\\Desktop\\arduino\\test.ino" 3
                  (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 20 "c:\\Users\\user\\Desktop\\arduino\\test.ino"
                  "1.Please recheck the connection!"
# 20 "c:\\Users\\user\\Desktop\\arduino\\test.ino" 3
                  ); &__c[0];}))
# 20 "c:\\Users\\user\\Desktop\\arduino\\test.ino"
                  )));
    Serial.println((reinterpret_cast<const __FlashStringHelper *>(
# 21 "c:\\Users\\user\\Desktop\\arduino\\test.ino" 3
                  (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 21 "c:\\Users\\user\\Desktop\\arduino\\test.ino"
                  "2.Please insert the SD card!"
# 21 "c:\\Users\\user\\Desktop\\arduino\\test.ino" 3
                  ); &__c[0];}))
# 21 "c:\\Users\\user\\Desktop\\arduino\\test.ino"
                  )));
    while(true){
      delay(0); // Code to compatible with ESP8266 watch dog.
    }
  }
  myDFPlayer.volume(30); //Set volume value. From 0 to 30
}

void loop() {
  boolean play_state = digitalRead(23);
  unsigned int Value = 0.0;
  float VRL; //Voltage drop across the MQ sensor
  float Rs; //Sensor resistance at gas concentration 
  float ratio; //Define variable for ratio

  VRL = analogRead(34 /*Sensor is connected to A4*/)*(3.3/4095.0); //Measure the voltage drop and convert to 0-5V
  Rs = ((3.3*47 /*The value of resistor RL is 47K*/)/VRL)-47 /*The value of resistor RL is 47K*/; //Use formula to get Rs value
  ratio = Rs/20 /*Enter found Ro value*/; // find ratio Rs/Ro

  float ppm = pow(10, ((log10(ratio)-0.42 /*Enter calculated interceptm*/)/-0.263 /*Enter calculated Slope */)); //use formula to calculate ppm
  Value = ppm;
  Serial.print(ppm);
  Serial.print(" ppm,");
  Serial.print(VRL);
  Serial.println(" v");

  if (VRL > 1) {
    if (play_state == 0x1) {
      myDFPlayer.play(1);
    }
  }
}
