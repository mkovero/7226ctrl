# Settings:
# by Pete 2018/05

NODEJS_VERSION="8.11.2";

#_____________________________________________________

# Check if running as root/sudo		
if [ "$EUID" -ne 0 ]
  then echo "Please run this script as root/sudo !!!"
  exit
fi

echo '' ;

echo 'Updating packages...';
sudo apt-get update && sudo apt-get upgrade -y;

echo 'Setting keyboard layout to finnish...';
sudo sed -i 's/XKBLAYOUT="gb"/XKBLAYOUT="fi"/g' /etc/default/keyboard;

echo 'Improving WLAN range (setting country to Bulgaria)...';
sudo sed -i "\$acountry=BG" /etc/wpa_supplicant/wpa_supplicant.conf;

echo 'Disabling IPv6 (for GPS compatibility)...';
sudo sed -i "\$anet.ipv6.conf.all.disable_ipv6 = 1" /etc/sysctl.conf;
sudo sed -i "\$anet.ipv6.conf.default.disable_ipv6 = 1" /etc/sysctl.conf;
sudo sed -i "\$anet.ipv6.conf.lo.disable_ipv6 = 1" /etc/sysctl.conf;

echo 'Installing GPSd packages...';
sudo apt-get install gpsd gpsd-clients python-gps -y;

echo 'Configuring GPSd...';
sudo sed -i "\$achmod a+rw $DEVNAME" /lib/udev/gpsd.hotplug;
sudo sed -i "\$agpsdctl $ACTION $DEVNAME" /lib/udev/gpsd.hotplug;
sudo sed -i 's/DEVICES=""/DEVICES="/dev/ttyUSB0"/g' /etc/default/gpsd;
sudo sed -i "\$aGPSD_SOCKET="/var/run/gpsd.sock"" /etc/default/gpsd;

echo 'Enabling GPSd...';
sudo systemctl enable gpsd.socket;

echo 'Starting GPSd...';
sudo systemctl start gpsd.socket;

echo 'Installing NodeJS...';
cd;
wget https://nodejs.org/dist/v$NODEJS_VERSION/node-v$NODEJS_VERSION-linux-armv6l.tar.xz;
tar -xvf node-v$NODEJS_VERSION-linux-armv6l.tar.xz;
cd node-v$NODEJS_VERSION-linux-armv6l;
sudo cp -R * /usr/local/;
cd;
rm -R node-v$NODEJS_VERSION-linux-armv6l;

echo 'Installing Desktop Environment...';
sudo apt-get install raspberrypi-ui-mods -y;

echo 'Disabling Raspberry Pi logos at boot...';
sudo sed -i 's/splash/splash logo.nologo/g' /boot/cmdline.txt;

echo 'Changing bootsplash image...';
sudo mv /usr/share/plymouth/themes/pix/splash.png /usr/share/plymouth/themes/pix/splash_old.png ;
sudo wget https://dump.re/users/mersu/mb-wallpaper.png -O /usr/share/plymouth/themes/pix/splash.png ;

echo 'Changing wallpaper image...';
sudo mv /usr/share/rpd-wallpaper/road.jpg /usr/share/rpd-wallpaper/road_old.jpg
sudo wget https://dump.re/users/mersu/mb-wallpaper.jpg -O /usr/share/rpd-wallpaper/road.jpg;

echo 'Hiding menubar...';
sudo sed -i 's/autohide=0/autohide=1/g' /home/pi/.config/lxpanel/LXDE-pi/panels/panel;
sudo sed -i 's/heightwhenhidden=2/heightwhenhidden=0/g' /home/pi/.config/lxpanel/LXDE-pi/panels/panel;

echo 'Enabling auto-login...';
sudo sed -i 's/#autologin-user=/autologin-user=pi/g' /etc/lightdm/lightdm.conf;
sudo sed -i 's/#autologin-user-timeout=0/autologin-user-timeout=0/g' /etc/lightdm/lightdm.conf;

echo 'Disabling mouse/touch cursor...';
sudo sed -i 's/#xserver-command=X/xserver-command=X -nocursor/g' /etc/lightdm/lightdm.conf;

echo 'Disabling screen sleep/blank...';
sudo sed -i "\$a@xset s noblank" /home/pi/.config/lxsession/LXDE-pi/autostart;
sudo sed -i "\$a@xset s off" /home/pi/.config/lxsession/LXDE-pi/autostart;
sudo sed -i "\$a@xset -dpms" /home/pi/.config/lxsession/LXDE-pi/autostart;

echo 'Disabling under-voltage warning icon...';
sudo sed -i "\$aavoid_warnings=1" /boot/config.txt;

# TODO NOT SURE IF REQUIRED IN DEBIAN STRECH...
#echo 'Disabling splash...';
#sudo sed -i "\$adisable_splash=1" /boot/config.txt;


#_____________

echo '';
echo 'WLAN config example (sudo nano /etc/wpa_supplicant/wpa_supplicant.conf)';
echo '';
echo 'network={';
echo '	    ssid="network1"';
echo '	    psk="pass1"';
echo '}';
echo '';

echo 'All done!';
echo 'Please reboot now: sudo reboot';
echo '';
