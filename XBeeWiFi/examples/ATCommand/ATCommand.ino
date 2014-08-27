#include <XBee.h>
#include <SPI.h>
#include <Ethernet.h>
#include <XBeeWiFi.h>
#include <SoftwareSerial.h>

#define SECURITY SECURITY_WPA2
#define SSID "***********"
#define PASSPHRASE "**********"

// Define SoftSerial TX/RX pins
// Connect Arduino pin 8 to TX of usb-serial device
uint8_t ssRX = 8;
// Connect Arduino pin 9 to RX of usb-serial device
uint8_t ssTX = 9;
// Remember to connect all devices to a common Ground: XBee, Arduino and USB-Serial device
SoftwareSerial nss(ssRX, ssTX);

// IP address
char myCmd[] = "MY";
// Network Mask
char mkCmd[] = "MK";
// Default Gateway
char gwCmd[] = "GW";
// Communication port
char ipCmd[] = "IP";
uint8_t ipValue = 0;
// Communication port
char c0Cmd[] = "C0";
// Destination Address
char dlCmd[] = "DL";
String dlValue = "192.168.2.17";
// Destination Port
char deCmd[] = "DE";
uint16_t deValue = 80;


XBeeWiFi xbee = XBeeWiFi();
AtCommandRequest atRequest;
AtCommandResponse atResponse = AtCommandResponse();

void setup()
{
    char buf[100];
    // start soft serial
    nss.begin(9600);
    // start xbee
    // start xbee
    Serial.begin(115200);
    xbee.begin(Serial);
    if (init_wifi(20)) {
        nss.print("XBee error\r\n");
        while(1) {}
    }
    // Startup delay to wait for XBee radio to initialize.
    // you may need to increase this value if you are not getting a response
    delay(5000);

    atRequest.setCommand((uint8_t*)dlCmd);
    sendAtCommand();
    atRequest.setCommand((uint8_t*)deCmd);
    sendAtCommand();
    atRequest.setCommand((uint8_t*)ipCmd);
    sendAtCommand();

    // set Destination Address
    atRequest.setCommand((uint8_t*)dlCmd);
    dlValue.toCharArray(buf,100);
    atRequest.setCommandValue((uint8_t*)buf);
    atRequest.setCommandValueLength(dlValue.length());
    sendAtCommand();
    atRequest.clearCommandValue();
    // set Destination Port
    atRequest.setCommand((uint8_t*)deCmd);
    buf[0] = (deValue >> 8) & 0xff;
    buf[1] = deValue & 0xff;
    atRequest.setCommandValue((uint8_t*)buf);
    atRequest.setCommandValueLength(2);
    sendAtCommand();
    atRequest.clearCommandValue();
    // set IP protocol
    atRequest.setCommand((uint8_t*)ipCmd);
    atRequest.setCommandValue(&ipValue);
    sendAtCommand();
    atRequest.clearCommandValue();
    nss.println();
}

void loop()
{
    // IP address
    atRequest.setCommand((uint8_t*)myCmd);
    sendAtCommand();
    // Network Mask
    atRequest.setCommand((uint8_t*)mkCmd);
    sendAtCommand();
    // Default Gateway
    atRequest.setCommand((uint8_t*)gwCmd);
    sendAtCommand();
    // Destination Address
    atRequest.setCommand((uint8_t*)dlCmd);
    sendAtCommand();
    // Destination Port
    atRequest.setCommand((uint8_t*)deCmd);
    sendAtCommand();
    // IP protocol
    atRequest.setCommand((uint8_t*)ipCmd);
    sendAtCommand();
    nss.println();
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
            nss.println("Successfully joined an access point.");
            nss.println();
            xbee.readPacket(5000);
            return 0;
        case INITIALIZATION:
            nss.println("WiFi initialization in progress.");
            break;
        case SSID_NOT_FOUND:
            nss.println("SSID not found.");
            return -1;
        case SSID_NOT_CONFIGURED:
            nss.println("SSID not configured.");
            return -1;
        case JOIN_FAILED:
            nss.println("SSID join failed.");
            return -1;
        case WAITING_IPADDRESS:
            nss.println("Waiting for IP configuration.");
            break;
        case WAITING_SOCKETS:
            nss.println("Listening sockets are being set up.");
            break;
        case SCANNING_SSID:
            nss.println("Currently scanning for SSID.");
            break;
        default:
            nss.println();
            break;
        }
    }
    return -1;
}

void sendAtCommand()
{
    nss.println("Sending command to the XBee");

    // send the command
    xbee.send(atRequest);

    // wait up to 5 seconds for the status response
    if (xbee.readPacket(5000)) {
        // got a response!

        // should be an AT command response
        if (xbee.getResponse().getApiId() == AT_COMMAND_RESPONSE) {
            xbee.getResponse().getAtCommandResponse(atResponse);

            if (atResponse.isOk()) {
                nss.print("Command [");
                nss.write(atResponse.getCommand()[0]);
                nss.write(atResponse.getCommand()[1]);
                nss.println("] was successful!");

                if (atResponse.getValueLength() > 0) {
                    nss.print("Command value length is ");
                    nss.println(atResponse.getValueLength(), DEC);

                    nss.print("Command value: ");

                    for (int i = 0; i < atResponse.getValueLength(); i++) {
                        if (atResponse.getValueLength() >= 7) {
                            nss.write(atResponse.getValue()[i]);
                        } else {
                            nss.print(atResponse.getValue()[i], HEX);
                            nss.print(" ");
                        }
                    }
                    nss.println("");
                }
            } else {
                nss.print("Command return error code: ");
                nss.println(atResponse.getStatus(), HEX);
            }
        } else {
            nss.print("Expected AT response but got ");
            nss.print(xbee.getResponse().getApiId(), HEX);
        }
    } else {
        // at command failed
        if (xbee.getResponse().isError()) {
            nss.print("Error reading packet.  Error code: ");
            nss.println(xbee.getResponse().getErrorCode());
        } else {
            nss.print("No response from radio");
        }
    }
}





