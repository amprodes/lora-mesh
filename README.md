# LoRa Mesh Networking

This project implements the components of a system that demonstrates mesh networking between LoRa nodes and a way to visualize the network on a web page. For full details of the project.

Nodes in the network are Arduino-compatible boards with a LoRa tranceiver. Using Arduino PRO mini, loRa-02 and ESP8266 wifi module

There are several components of this project:

### SetNodeId

Arduino sketch to set the node's ID in EEPROM so that every node can have the same source code (without hard-coding the node ID). This is a one-time process for each node. Set the node ID in this sketch then upload to a node (e.g. a Moteino). When it runs it saves the node ID in EEPROM. Then you can load the LoRaMesh sketch to the node.

Pinout with Arduino Pro mini using an UNO as intermediate mechanism

| Mini  |  Uno  |
| ----- | ----- |
|  GND  |  GND  |
|  VCC  |  5V   |
|  RX1  |  RX0  |
|  TX0  |  TX1  |
|  DTR  |  RST  |

### LoRaMesh

Arduino sketch that attempts to talk to all other nodes in the mesh. Each node sends its routing information to every other node. The process of sending data and receiving acknowledgements lets a node determine which nodes it can successfully communicate with directly. This is how each node builds up it's routing table. You must set N_NODES to the max number of nodes in your mesh.




|    LoRa-02 Module   |  Arduino Nano Board  |
| ------------------- | -------------------- |
|        3.3V         |   Batery Lipo 3.6v   |
|        GND          |         GND          |
|        EN/Nss       |         D10          |
|        G0/DIO0      |         D2           |
|        SCK          |         D13          |
|        MISO         |         D12          |
|        MOSI         |         D11          |
|        RST          |         D9           |

Dependencies:

* [RadioHead library](http://www.airspayce.com/mikem/arduino/RadioHead/)


### Gateway

ESP8266 Arduino sketch that talks to a connected LoRa node via Serial (node number 1 in the mesh) and publishes mesh routing information to an MQTT topic. Node 1 in the mesh will eventually receive routing info from every other node.

Dependencies:

* [PubSubClient](https://github.com/knolleary/pubsubclient)

Pinout with Arduino Pro mini and ESP2866-01 Wifi;

Programming mode

| Mini  |  Esp  |
| ----- | ----- |
|  VCC  |  3V3  |
|  RX1  |  RX   |
|  TX1  |  TX   |
|  NAN  |  RST  |
|  GND  |  GP01 |
|  VCC  |   EN  |
|  NAN  |  GP02 |
|  GND  |  GND  |

Running Mode

| Mini  |  Esp  |
| ----- | ----- |
|  VCC  |  3V3  |
|  RX1  |  RX   |
|  TX1  |  TX   |
|  NAN  |  RST  |
|  NAN  |  GP01 |
|  VCC  |   EN  |
|  NAN  |  GP02 |
|  GND  |  GND  |

### mesh-server

Node.js server provides a web visualization of the mesh. Runs on port 4200. Install with `npm install`. The server subscribes to the MQTT topic to receive routing info about nodes. This server sends the received routing info to the web client using Socket.IO. The web client uses p5.js to draw a representation of the mesh based on the routing information received from each node.

Dependencies (install with `npm install`)

* express
* jquery
* mqtt
* socket.io
* rxjs
* p5




