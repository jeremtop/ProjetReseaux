sudo ip address add $1 dev tun0
ip link set tun0 up
if [ $2 = 'fc00:1234:1::16' ]
then
	sudo ip route add 172.16.2.144/28 via 172.16.2.10 dev tun0
elif [ $2 = 'fc00:1234:2::36' ]
then
	sudo ip route add 172.16.2.176/28 via 172.16.2.10 dev tun0
else
	echo Pas de route configuree
fi