/*
 * BFB Embedded Code
 * by Kevin Jensen
 * 
 * Reads 7 thermistors inputs from Energus Modules
 * Relays temperature info to Orion BMS v2 by emulating a thermistor expansion module, send CAN messages
 * Sets fault pin to high in case of fault (over or under temperature, missing temperature)
 */
 #include <Arduino.h>
 #include "FlexCAN.h"
 #include "Metro.h"

const int tempMap[] = {499, 495, 491, 486, 480, 474, 464, 456, 443, 431, 419, 407, 392, 380, 368, 356, //thermistor values in 10 bit resolution from -40C to 120C
343, 333, 325, 317, 308, 302, 296, 292, 286, 282, 280, 276, 274, 272, 270, 268, 265};
const int tempMapLen = sizeof(tempMap) / sizeof(tempMap[0]);
const int tpPins[] = {0,1,2,3,4,5,6};
const int faultPin = 7;
int temps[7];
const int numTemps = sizeof(temps) / sizeof(temps[0]);
bool fault = false;
int low, high, avg;

Metro MS200 = Metro(200);
Metro MS100 = Metro(100);

FlexCAN CANbus(125000); //baud rate of 125kbps
static CAN_message_t addressClaimMsg;



int mapTemp(int analogIn)  //converts analog reading to celsius
{
  if (analogIn > 499) { return -41; }
  else if(analogIn < 265) { return 121; }
  
  else {
    for (int i=1; i<tempMapLen; i++) {
      if (analogIn == tempMap[i]) {
        return -40 + 5*i;
      }
      else if (analogIn > tempMap[i]) {
        int tIndex = i;
        break;
      }
    }
    float diff = tempMap[tIndex - 1] - tempMap[tIndex];
    float valDiff = analogIn - tempMap[tIndex];
    float adjust = valDiff / diff;
    return - 40 + 5*tIndex - int(5*adjust);
  }
}

void getTemps()
{
  for (int i=0; i<7; i++) {
    temps[i] = mapTemp(analogRead(tpPins[i]));
  }
}
 
void lowHighAvg(int *low, int *high, int *avg) //finds low high
{
  int numActive = 0;
  int sum = 0;
  int l = 41;
  int h = -121;
  for(int i=0; i<numTemps; i++) {
    if (!(temps[i] < -40 || temps[i] > 120)) {
      numActive ++;
      sum += temps[i];
    }
    l = min(l, temps[i]);
    h = max(h, temps[i]);
  }
  *low = l;
  *high = h;
  *avg = int(sum/numActive);
}


void setup() {
  Serial.begin(9600);
  CANbus.begin();

  addressClaimMsg.ext = true;
  addressClaimMsg.id = 0x18EEFF80;
  addressClaimMsg.len = 8;
  addressClaimMsg.buf[0] = 0xF3;
  addressClaimMsg.buf[1] = 0x00;
  addressClaimMsg.buf[2] = 0x80;
  addressClaimMsg.buf[3] = 0xF3;
  addressClaimMsg.buf[4] = 0x00;
  addressClaimMsg.buf[5] = 0x40;
  addressClaimMsg.buf[6] = 0x1E;
  addressClaimMsg.buf[7] = 0x90;
}

void loop() {
  
  getTemps();
  lowHighAvg(&low, &high, &avg);
  
  for (int i=0;) {
    Serial.print(i);
    Serial.print(" Temp: ");
    Serial.println(mapTemp(i));
    
  }
  delay(5000);
}

