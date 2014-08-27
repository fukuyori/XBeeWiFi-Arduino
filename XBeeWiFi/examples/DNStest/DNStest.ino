#include <XBee.h>
#include <SPI.h>
#include <Ethernet.h>
#include <XBeeWiFi.h>
#include <SoftwareSerial.h>
#include "dbg.h"
#define SECURITY SECURITY_WPA2
#define SSID "*********"
#define PASSPHRASE "**********"

AtCommandRequest atRequest;
AtCommandResponse atResponse = AtCommandResponse();

// Define SoftSerial TX/RX pins
// Connect Arduino pin 8 to TX of usb-serial device
uint8_t ssRX = 8;
// Connect Arduino pin 9 to RX of usb-serial device
uint8_t ssTX = 9;
// Remember to connect all devices to a common Ground: XBee, Arduino and USB-Serial device
SoftwareSerial nss(ssRX, ssTX);
// Name Server IP and Port.
IPAddress nameServer(192,168,2,1);
int nameServerPort = 53;

XBeeWiFi xbee = XBeeWiFi();

char hostname[] = "www.google.co.jp";

void setup()
{
    // start soft serial
    nss.begin(9600);
    // start xbee
    Serial.begin(115200);
    Serial1.begin(9600);
    xbee.begin(Serial);
    if (init_wifi(20)) {
        nss.print("XBee error\r\n");
        while(1) {}
    }
    // Startup delay to wait for XBee radio to initialize.
    // you may need to increase this value if you are not getting a response
    delay(1000);

    int r, i;
    IPAddress ip;
    r = xbee.setNameserver(nameServer,nameServerPort);
    r = xbee.getHostByName(hostname,ip);
    nss.print(hostname);
    nss.print(" : ");
    if (r >= 0) {
    	nss.println(ip);
    }
}

void loop()
{
}

int init_wifi (int timeout)
{
    int i, r;

    nss.print("reset\r\n");
    r = xbee.reset();
    if (r < 0) {
        nss.print("error reset\r\n");
        return -1;
    }
    xbee.getWiResponse(MODEM_STATUS_RESPONSE, 5000);
    r = xbee.setup(SECURITY, SSID, PASSPHRASE);
    if (r < 0) {
        nss.print("error setup \r\n");
        return -1;
    }

    for (i = 0; i < timeout; i ++) {
        delay(1000);
        r = xbee.getStatus();
        // nss.print("status %02x: ", r);
        switch (r) {
        case JOINED_AP:
            nss.print("Successfully joined an access point.\r\n");
            return 0;
        case INITIALIZATION:
            nss.print("WiFi initialization in progress.\r\n");
            break;
        case SSID_NOT_FOUND:
            nss.print("SSID not found.\r\n");
            return -1;
        case SSID_NOT_CONFIGURED:
            nss.print("SSID not configured.\r\n");
            return -1;
        case JOIN_FAILED:
            nss.print("SSID join failed.\r\n");
            return -1;
        case WAITING_IPADDRESS:
            nss.print("Waiting for IP configuration.\r\n");
            break;
        case WAITING_SOCKETS:
            nss.print("Listening sockets are being set up.\r\n");
            break;
        case SCANNING_SSID:
            nss.print("Currently scanning for SSID.\r\n");
            break;
        default:
            nss.print("\r\n");
            break;
        }
    }
    return -1;
}
