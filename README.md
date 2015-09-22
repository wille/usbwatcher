# usbwatcher

## Features

- Perform selected actions when disallowed USB activity is detected
- Select which scripts should be ran when triggered or write your own
- Whitelist USB device by serial, mount point or keyfile
- Needs ```secure-delete``` installed if you want to securely wipe swap and RAM
- Change check interval

## Dependencies

- [quickd](https://github.com/redpois0n/quickd)

## Arguments

| Argument    		| Description                           |
| --------    		| -----------                           |
| --list, -l	   	| Lists blockable devices		|
| --help, -h		| Prints usage				|
| --daemon		| Run as daemon (Linux)			|
| --genkey [path]	| Generate keyfile			| 
| --pid [path]		| Write process id to file		|

## Credits

- [redpois0n](https://redpois0n.com)
- [chloe](http://chloe.re)
- [Iterating disks in Linux](https://stackoverflow.com/questions/7243988/how-to-list-the-harddisks-attached-to-a-linux-machine-using-c)
