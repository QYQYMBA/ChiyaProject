# Chiya
Is a open source alternative to default windows way of switching keyboard layouts. In Windows you can't temporary disable some layouts. With Chiya you can disable/enable languages you need or switch to them with hotkeys.

# Where to get
You can download Chiya using installer/directly or build it from sources. 

### Building from source
You can download source code from this repository and build it with Qt. Qt version 6.1.2 is used.

### Using installer
You can also use [installer](https://github.com/QYQYMBA/ChiyaProject/releases). You can use installer to get latest release.

### Download .exe
You can download latest [release](https://github.com/QYQYMBA/ChiyaProject/releases) here.

# Running Chiya as administrator
Some programs might not work without Chiya beeing runned as administrator. It is recommended to run Chiya as administrator. 

# Settings
This section will explain what different settings are doing
### General Settings
1) Run on startup - if this is checked, the programm will start on windows startup.
2) Run as administrator - if this is checked, the programm will run as administrator.
3) Auto update - if this is checked, the program will check if update is available and perform update in background.
4) Start in tray - if this is checked, the programm will start directly in tray.
5) Current Combination - you can select what key kombination you want to use. **This will affect key kombination in windows and not only in Chiya.**

### Layout Controller Settings
Click on ... button to go to the Layout Controller settings
#### General settings
1) Start with Chiya - decides if the module will start with main program (Chiya)
2) Disable windows default switching - if this function is enabled, default Windows switching will be disabled. Chiya will work faster, but in case of crush. You will need to run Chiya again or manualy set shortcut for switching layout in Windows settings. It is highly recomended to activate this if you are running Chiya not as administrator.
#### Layout specific settings
1) Select language in the list
2) Your layout will be switched to the selected
3) You can set hotkeys and set enabled/disabled as default for this language.
#### Exceptions
1) In text fild write your exceptions separated with space. Example: (Chiya Chrome Telegram)
2) If Whitelist is checked, Chiya will work only in apps mentioned in exceptions list


