#include <EEPROM.h>
#include <RHRouter.h>
#include <RHMesh.h>
#include <RH_RF95.h> 

uint8_t nodeId;
int solenoidPin = 6; 

// Singleton instance of the radio driver
RH_RF95 rf95;

// Class to manage message delivery and receipt, using the driver declared above
RHMesh *manager;

// message buffer
char buf[RH_MESH_MAX_MESSAGE_LEN];

int freeMem() {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

void setup() { 
    
  Serial.begin(115200);
  while (!Serial) ; // Wait for serial port to be available
  pinMode(solenoidPin, OUTPUT);
  
  nodeId = EEPROM.read(0);
  
  if (nodeId > 10) {
    Serial.print(F("EEPROM nodeId invalid: "));
    Serial.println(nodeId);
    nodeId = 1;
  }
  
  Serial.print(F("initializing node "));

  manager = new RHMesh(rf95, nodeId);

  if (!manager->init()) {
    Serial.println(F("init failed"));
  } else {
    Serial.println("done");
  }
  
  rf95.setTxPower(23, false);
  rf95.setFrequency(494.0);
  rf95.setCADTimeout(500);

  // Possible configurations:
  // Bw125Cr45Sf128 (the chip default)
  // Bw500Cr45Sf128
  // Bw31_25Cr48Sf512
  // Bw125Cr48Sf4096

  // long range configuration requires for on-air time
  boolean longRange = true;
  
  if (longRange) {
    
    RH_RF95::ModemConfig modem_config = {
      0x78, // Reg 0x1D: BW=125kHz, Coding=4/8, Header=explicit
      0xC4, // Reg 0x1E: Spread=4096chips/symbol, CRC=enable
      0x08  // Reg 0x26: LowDataRate=On, Agc=Off.  0x0C is LowDataRate=ON, ACG=ON
    };
    
    rf95.setModemRegisters(&modem_config);
    if (!rf95.setModemConfig(RH_RF95::Bw500Cr45Sf128)) {
      Serial.println(F("set config failed"));
    }
    
  }

  Serial.println("RF95 ready");
  Serial.print(F("mem = "));
  Serial.println(freeMem());
  
}
  
//draw battery level in position x,y
long batterylevel(){
  //read the voltage and convert it to volt
  double curvolt = double( readVcc() ) / 1000;
  // check if voltge is bigger than 4.2 volt so this is a power source
  Serial.print(curvolt);
  
}

//read internal voltage
long readVcc() {
  long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA, ADSC));
  result = ADCL;
  result |= ADCH << 8;
  result = 1126400L / result; // Back-calculate AVcc in mV
  return result;
}

void printNodeInfo(uint8_t node, String d, /*char* b,*/ int r) {
  Serial.print(F("{"));
  
  Serial.print(F("\""));
  Serial.print(F("nod"));
  Serial.print(F("\""));
  
  Serial.print(F(":"));
  
  Serial.print(F("\""));
  Serial.print(node);
  Serial.print(F("\"")); 
  
  Serial.print(F(", "));
  
  Serial.print(F("\""));
  Serial.print(F("dat")); 
  Serial.print(F("\""));
  
  Serial.print(F(":"));
  
  Serial.print(F("\""));
  Serial.print(d);
  Serial.print(F("\""));
  
  Serial.print(F(", "));
  /*
  Serial.print(F("\"")); 
  Serial.print(F("bat")); 
  Serial.print(F("\""));

  Serial.print(F(":"));
  
  Serial.print(F("\""));  
  Serial.print(b);
  Serial.print(F("\""));

  Serial.print(F(", "));
  */
  Serial.print(F("\"")); 
  Serial.print(F("rsi")); 
  Serial.print(F("\""));

  Serial.print(F(":"));

  Serial.print(F("\""));  
  Serial.print(r);
  Serial.print(F("\""));
  
  Serial.println(F("}"));
}
const __FlashStringHelper* getErrorString(uint8_t error) {
  switch(error) {
    case 1: return F("idn"); //invalid lenght
    break;
    case 2: return F("nRo"); //no route
    break;
    case 3: return F("tmo"); //timeout
    break;
    case 4: return F("noR"); //no replay
    break;
    case 5: return F("utd"); //unable to deliver
    break;
  }
  return F("unknown");
}  
  
void loop() {

    // listen for incoming messages. Wait a random amount of time before we transmit
    // again to the next node
    unsigned long nextTransmit = millis() + random(3000, 5000);

    while (nextTransmit > millis()) {

      int waitTime = nextTransmit - millis();
      uint8_t len = sizeof(buf);
      uint8_t from;
      String valveStatus; 
      
      //if (manager->recvfromAckTimeout((uint8_t *)buf, &len, waitTime, &from)) {
      if (manager->recvfromAck((uint8_t *)buf, &len, &from)) {
        // we received data from node 'from', but it may have actually come from an intermediate node
        RHRouter::RoutingTableEntry *route = manager->getRouteTo(from);
        
        if (route->next_hop == 0) { 
          
          buf[len] = '\0'; // null terminate string
          
          if(strcmp(buf, "ON") == 32){
             
            digitalWrite(solenoidPin, HIGH);    //Switch Solenoid ON
            valveStatus = "ON";
            
          }else if(strcmp(buf, "OFF") == 32){

            digitalWrite(solenoidPin, LOW);     //Switch Solenoid OFF
            valveStatus = "OFF";
            
          }

          //this is the data collected from node
          //buf[len++] = printNodeInfo(nodeId, valveStatus, rf95.lastRssi()); 
          //len = len % RH_MESH_MAX_MESSAGE_LEN;
          printNodeInfo(nodeId, valveStatus, rf95.lastRssi());
          uint8_t error = manager->sendtoWait((uint8_t *) buf, strlen(buf), from);
      
          if (error != RH_ROUTER_ERROR_NONE) {
            
            //if message didn't get sended let's doit again
            manager->sendtoWait((uint8_t *)buf, strlen(buf), from);
      
          } else { 
            
          }
        } 
      }
    } 
    
}
