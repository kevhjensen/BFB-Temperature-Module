/*
 * BFB Embedded Code
 * by Kevin Jensen
 * 
 * Reads 7 thermistors inputs from Energus Modules
 * Relays temperature info to Orion BMS v2 by emulating a thermistor expansion module, send CAN messages
 * Sets fault pin to high in case of fault (over or under temperature, missing temperature)
 */
 
const int tempMap[] = {499, 495, 491, 486, 480, 474, 464, 456, 443, 431, 419, 407, 392, 380, 368, 356, //thermistor values in 10 bit resolution from -40C to 120C
343, 333, 325, 317, 308, 302, 296, 292, 286, 282, 280, 276, 274, 272, 270, 268, 265};
const int tempMapLen = sizeof(tempMap) / sizeof(tempMap[0]);
const int tpPins[] = {0,1,2,3,4,5,6};
const int faultPin = 7;
int temps[7];
bool fault = false;

void setup() {
  Serial.begin(9600);
  
}

void loop() {
  /*
  for (int i=0; i<7; i++) {
    temps[i] = analogRead(tpPins[i]);

  }
  */
  for (int i=500; i >250; i--) {
    Serial.print(i);
    Serial.print(" Temp: ");
    Serial.println(mapTemp(i));
    
  }
  delay(5000);
}

int mapTemp(int analogIn) {
  int tIndex;
  if (analogIn > 499) { return 1; }
  else if(analogIn < 265) { return 0; }
  
  else {
    for (int i=1; i<tempMapLen; i++) {
      if (analogIn == tempMap[i]) {
        return -40 + 5*i;
      }
      else if (analogIn > tempMap[i]) {
        tIndex = i;
        break;
      }
    }
    float diff = tempMap[tIndex - 1] - tempMap[tIndex];
    float valDiff = analogIn - tempMap[tIndex];
    float adjust = valDiff / diff;
    return - 40 + 5*tIndex - int(5*adjust);
  }
}
