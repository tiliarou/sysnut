Homebrew network title installer.

# Installation

 - Create the directory /switch/deez/ on your switch's SD card.
 
 - Copy deez.nro to /switch/deez/deez.nro .
 
 - Obtain or generate a keys.txt file and place it in /switch/deez/keys.txt .  keys.txt is a text file containing various switch encryption keys.  If you plan to generate it yourself, you can find instructions here:  https://gbatemp.net/threads/how-to-get-switch-keys-for-hactool-xci-decrypting.506978/
 
 - Copy locations.conf to /switch/deez/locations.conf .  You should edit this file, it is only an example, and points to the various local and network locations hosting your switch content.

# Disclaimer

Use at your own risk, and always have a NAND backup.

# Credits

Tinfoil source code was reverse-engineered with plenty of help from Adubbz answering questions:
https://github.com/Adubbz/Tinfoil

icons lifted from:
https://github.com/NightlyFox/switchIconLibrary

UI Code started from:
https://github.com/XorTroll/CustomUI/

HACTOOL source code was reverse-engineered, with small bits of code lifted here and there:
https://github.com/SciresM/hactool

Random JSON parser:
https://github.com/nlohmann/json
