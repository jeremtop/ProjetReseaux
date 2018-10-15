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

##Configuration de VM1-6
eth1:
  network.managed:
    - enabled: True
    - type: eth
    - proto: none
    - ipaddr: 172.16.2.156
    - netmask: 28
  
eth2:            
  network.managed:                                                              
    - enabled: True           
    - type: eth                                                        
    - proto: none                                                   
    - enable_ipv6: True                          
    - ipv6proto: auto    

## Configuration de la route vers LAN1 via VM1
routes:
  network.routes:
    - name: eth1
    - routes:
      - name: LAN1
        ipaddr: 172.16.2.128/28
        gateway: 172.16.2.151

## Configuration de la route vers LAN2-6 via VM2-6
    - name: eth2
    - routes:
      - name: LAN2-6
        ipv6ipaddr: fc00:1234:1::/64
        gateway: fc00:1234:1::26


