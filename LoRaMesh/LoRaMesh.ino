#include <EEPROM.h>
#include <RHRouter.h>
#include <RHMesh.h>
#include <RH_RF95.h>
#define  RH_HAVE_SERIAL 

uint8_t nodeId;
//uint8_t routes[N_NODES]; // full routing table for mesh
//int16_t rssi[N_NODES]; // signal strength info

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
/*
void updateRoutingTable() {
  
  for(uint8_t n=1;n<=N_NODES;n++) {
    RHRouter::RoutingTableEntry *route = manager->getRouteTo(n);
    if (n == nodeId) {
      routes[n-1] = 255; // self
    } else {
      routes[n-1] = route->next_hop;
      if (routes[n-1] == 0) {
        // if we have no route to the node, reset the received signal strength
        rssi[n-1] = 0;
      }
    }
  }
}
*/
/*
void getRouteInfoString(char *p, size_t len) {
  p[0] = '\0';
  strcat(p, "[");
  for(uint8_t n=1;n<=N_NODES;n++) {
    strcat(p, "{\"n\":");
    sprintf(p+strlen(p), "%d", routes[n-1]);
    strcat(p, ",");
    strcat(p, "\"r\":");
    sprintf(p+strlen(p), "%d", rssi[n-1]);
    strcat(p, "}");
    if (n<N_NODES) {
      strcat(p, ",");
    }
  }
  strcat(p, "]");
}
*/
void printNodeInfo(uint8_t node, char *s) {
  //Serial.print(F("node: "));
  Serial.print(F("{"));
  Serial.print(F("\""));
  Serial.print(node);
  Serial.print(F("\""));
  Serial.print(F(": "));
  Serial.print(s);
  Serial.println(F("}"));
}

void loop() {

    //updateRoutingTable();
    //getRouteInfoString(buf, RH_MESH_MAX_MESSAGE_LEN);
    //int n = 1;
    /*
    Serial.print(F("Incoming")); 
    Serial.print(F(": "));
    Serial.print(buf);
*/
/*
    // send an acknowledged message to the target node
    uint8_t error = manager->sendtoWait((uint8_t *)buf, strlen(buf), n);

    if (error != RH_ROUTER_ERROR_NONE) {
 
      Serial.println(getErrorString(error));

    } else {

      Serial.println(F(" OK"));
      // we received an acknowledgement from the next hop for the node we tried to send to.
      RHRouter::RoutingTableEntry *route = manager->getRouteTo(n);
      //Serial.print(route->next_hop);
      /*
      if (route->next_hop != 0) {
         Serial.print(F("Send Hop; "));
         Serial.print(route->next_hop);
         //printNodeInfo(nodeId, buf);
        //rssi[route->next_hop-1] = rf95.lastRssi();
      }*/
      
   //} 

    // listen for incoming messages. Wait a random amount of time before we transmit
    // again to the next node
    unsigned long nextTransmit = millis() + random(3000, 5000);

    while (nextTransmit > millis()) {

      int waitTime = nextTransmit - millis();
      uint8_t len = sizeof(buf);
      uint8_t from;
      
      if (manager->recvfromAckTimeout((uint8_t *)buf, &len, waitTime, &from)) {

        buf[len] = '\0'; // null terminate string
 
        // we received data from node 'from', but it may have actually come from an intermediate node
        RHRouter::RoutingTableEntry *route = manager->getRouteTo(from);
        
        if (route->next_hop == 0) { 
          
          if(strcmp(buf, "ON") == 32){
            Serial.println(F("Debo encender"));
          }else if(strcmp(buf, "OFF") == 32){
            Serial.println(F("Debo apagar"));
          }

          uint8_t error = manager->sendtoWait((uint8_t *)buf, strlen(buf), from);
      
          if (error != RH_ROUTER_ERROR_NONE) {
       
            Serial.println(getErrorString(error));
      
          } else {
      
          }
          
        } 
      }
    } 
    
}
