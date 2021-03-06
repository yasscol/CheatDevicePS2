Cheat Device for PS2
====================
Cheat Device is a game enhancer for PlayStation 2 games similar to Action 
Replay, GameShark, and CodeBreaker. It supports booting retail, disc based 
games as well as ELF files such as OpenPS2Loader or ESR.

See the [wiki](https://github.com/root670/CheatDevicePS2/wiki) to get started
using Cheat Device.

# Features
* Easy to navigate menu system similar to CodeBreaker
* Supports booting retail discs and loader ELFs
* Fast loading of large cheat lists
* Save manager for backing up and restoring game saves to/from a flash drive
* Powered by ps2rd's powerful cheat engine

# Compiling
If you have Docker installed, it's easiest to compile Cheat Device using my
docker image:
```bash
./docker-make
```
See the [compile](https://github.com/root670/CheatDevicePS2/wiki/Compiling) 
page in the wiki for instructions to build without Docker.

# License
Cheat Device is not licensed, sponsored, or endorsed by Sony Computer 
Entertainment, Inc. This program comes with ABSOLUTELY NO WARRANTY. Cheat 
Device is licensed under GNU GPL-3. See LICENSE for details. The cheat 
engine is from ps2rd by Mathias Lafedlt. The bootstrap is based on EE_CORE 
from OpenPS2Loader.
