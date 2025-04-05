#define ETH_TYPE                        ETH_PHY_RTL8201
#define ETH_ADDR                        0
#define ETH_CLK_MODE_0                  ETH_CLOCK_GPIO0_IN
#define ETH_RESET_PIN                   -1
#define ETH_MDC_PIN                     23
#define ETH_POWER_PIN                   12
#define ETH_MDIO_PIN                    18
#define SD_MISO_PIN                     34
#define SD_MOSI_PIN                     13
#define SD_SCLK_PIN                     14
#define SD_CS_PIN                       5


// ETHERNET CONFIGURATION
IPAddress ip(10, 255, 250, 150);                         //the Arduino's IP
IPAddress subnet(255, 255, 254, 0);                     //subnet mask
IPAddress gateway(10, 255, 250, 1);                     //gateway IP
IPAddress outIp(10, 255, 250, 129);                     //destination IP
const unsigned int inPort = 7001;                       //Arduino's Port
const unsigned int outPort = 7000;                      //destination Port
