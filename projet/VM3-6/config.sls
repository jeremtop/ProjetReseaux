## Configuration eth1
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

##Configuration de VM1-6
eth1:
  network.managed:                                                        
    - enabled: True           
    - type: eth                                                        
    - proto: none                                                   
    - enable_ipv6: True                          
    - ipv6proto: auto   
  
eth2:            
  network.managed:
    - enabled: True
    - type: eth
    - proto: none
    - enable_ipv4: false
    - ipv6proto: static
    - enable_ipv6: true
    - ipv6_autoconf: no
    - ipv6ipaddr: fc00:1234:2::26
    - ipv6netmask: 64      

## Configuration de la route vers LAN1-6 via VM2-6
routes:
  network.routes:
    - name: eth1
    - routes:
      - name: LAN2
        ipaddr: 172.16.2.160/28
        gateway: 172.16.2.183

## Configuration de la route vers LAN2 via VM3
    - name: eth2
    - routes:
      - name: LAN1-6
        ipv6ipaddr: fc00:1234:1::/64
        gateway: fc00:1234:2::26

