cp -rf $(pwd)/dhcpd.conf /etc/
cp -rf $(pwd)/dhcpd.conf /etc/dhcp/

/sbin/ifconfig $1 192.168.7.1
sleep 1
#For Fedora machines
#/sbin/service dhcpd restart

# For Dell (Ubunutu machines)
/usr/sbin/service isc-dhcp-server restart
