# Chiya
Is a free alternative to default windows method to switch layout. My problem with windows was that i have some languages that i use really rare and it was annoying to add and delete them every time i need to use them. With Chiya you can temporarily disable/enable languages you need or switch to them using hotkeys.

# Where to get
You can download Chiya using installer/directly or build it. 

### Building from source
You can download source code from this repository and build it with Qt 6.1.2.

### Using installer
You can also use [installer](https://github.com/QYQYMBA/ChiyaProject/releases). You can use installer to get latest release. In future auto updates will be performed using installer.
Installer is built using Qt Installer Framework.

### Download .exe
You can download latest [release](https://github.com/QYQYMBA/ChiyaProject/releases) here.

# Modules
I am planning to add more modules in future. All modules will handle different tasks but they all will be somehow connected to switching languages.

### List of available modules
1) Layout Controller - module that is responcible for switching languages insted of windows.

# Running Chiya as administrator
Some programs might not work without Chiya beeing runned as administrator. It is recommended to run Chiya as administrator. 

# Settings
This section will explain what different settings are doing
### General Settings
1) Run on startup - if this is checked, the programm will start automatically with yout windows 
2) Run as administrator - if this is checked, the programm will run as administrator
3) Auto update - if this is checked, the program will check if update is available and perform update in background. Can be used only with administrator mode.
4) Start in tray - if this is checked, the programm will start directly in tray

### Layout Controller Settings
Click on ... button to go to the Layout Controller settings
#### General settings
1) Start with Chiya - decides if the module will start with main program (Chiya)
2) Disable windows default switching - if this function is enabled, default windows switching will be disabled. Chiya will work faster, but in case of crush. You will need to run Chiya again or manualy set shortcut for switching layout in windows settings. It is highly recomended to activate this if you are running Chiya not as administrator.
#### Layout specific settings
1) Select language in the list
2) Your layout will be switched to the selected
3) You can set hotkeys and set enabled/disabled as default for this language.
#### Exceptions
1) In text fild write your exceptions separated with space. Example: (Chiya Chrome Telegram)
2) If Whitelist is checked, Chiya will work only in apps mentioned in exceptions list


