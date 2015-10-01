/**
 * Configuration file
 */
#define CONFIG_FILE "usbwatcher.conf"

/**
 * Delay between each check
 */
#define DELAY 1000

/**
 * Size of keyfile in bits when generating
 */
#define KEYFILE_BITS 4096

/**
 * Path of keyfile on devices
 */
#define KEYFILE_PATH "/keyfile"

/**
 * Config file
 */
#define CFG_INTERVAL "interval:"
#define CFG_EXEC "execute:"
#define CFG_ALLOW "allow:"
#define CFG_KEYFILE "keyfile:"

/**
 * Program command line arguments
 */
#define OPT_LIST "--list"
#define OPT_LIST_SHORT "-l"
#define OPT_HELP "--help"
#define OPT_HELP_SHORT "-h"
#define OPT_DAEMON "--daemon"
#define OPT_GENKEY "--genkey"
#define OPT_PID "--pid"
