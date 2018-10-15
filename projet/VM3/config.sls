# Configuration eth1
# RAPPEL: eth0 est à vagrant, ne pas y toucher

## Désactivation de network-manager
NetworkManager:
  service:
    - dead
    - enable: False
    
## Suppression de la passerelle par défaut
ip route del default:
  cmd:
    - run

##Configuration de VM3
eth1:
  network.managed:
    - enabled: True
    - type: eth
    - proto: none
    - ipaddr: 172.16.2.163
    - netmask: 28

eth2:
  network.managed:
    - enabled: True
    - type: eth
    - proto: none
    - ipaddr: 172.16.2.183
    - netmask: 28

## Configuration de la route vers LAN1 via VM2
routes:
  network.routes:
    - name: eth1
    - routes:
      - name: LAN1
        ipaddr: 172.16.2.128/28
        gateway: 172.16.2.162

## Configuration de la route vers LAN2-6 via VM3-6

    - name: eth2
    - routes:
      - name: LAN2-6
        ipv6ipaddr: fc00:1234:2::/64
        gateway : 172.16.2.186

## But enable ipv4 forwarding
net.ipv4.ip_forward:
  sysctl:
    - present
    - value: 1

