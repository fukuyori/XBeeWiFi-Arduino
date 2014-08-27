#include <XBee.h>
#include <SPI.h>
#include <Ethernet.h>
#include <XBeeWiFi.h>
#include <SoftwareSerial.h>

#define SECURITY SECURITY_WPA2
#define SSID "**********"
#define PASSPHRASE "**********"

// Define SoftSerial TX/RX pins
// Connect Arduino pin 8 to TX of usb-serial device
uint8_t ssRX = 8;
// Connect Arduino pin 9 to RX of usb-serial device
uint8_t ssTX = 9;
// Remember to connect all devices to a common Ground: XBee, Arduino and USB-Serial device
SoftwareSerial nss(ssRX, ssTX);

XBeeWiFi xbee = XBeeWiFi();
IPAddress ip(192,168,2,2);

void setup()
{
    // start soft serial
    nss.begin(9600);
    // start xbee
    Serial.begin(115200);
    xbee.begin(Serial);
    if (init_wifi(20)) {
        nss.print("XBee error\r\n");
        while(1) {}
    }
    IPAddress my(0,0,0,0),mk(0,0,0,0),gw(0,0,0,0),nm(0,0,0,0);
    xbee.getAddress(my,mk,gw,nm);
    nss.print("My IP address is ");
    nss.println(my);

    delay(1000);
}

void loop()
{
    char buf[] = "HELLO WORLD!";

    int r;
    r = xbee.wifisend(ip, 8080, 8081, PROTOCOL_UDP, OPTION_TERMINATE, (uint8_t*)buf, sizeof(buf)-1);
    nss.println("Sending message");
    nss.print("wifi TX: ");
    nss.println(r);
    delay(10000);
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
            nss.println();
            xbee.readPacket(5000);
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




