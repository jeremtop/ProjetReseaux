# ProjetReseaux
Nous sommes dans le futur, et toutes les machines utilisent ipv6. Toutes ? Non ! Un groupe d'emmerdeurs résiste encore et toujours à l'envahisseur v6 et subsiste encore quelques machines en IPv4 seulement. 
Le but de ce projet est de permettre des communications entre îlots IPv4 au sein d'un monde IPv6. Dans une première partie, on utilisera des tunnels simples au-dessus de IPv6. 

Pour mettre en place le tunnel entre VM1 et VM3 il suffit d'executer :

sudo ./extremite config16.txt sur VM1-6

sudo ./extremite config36.txt sur VM3-6

A partir de maintenant le tunnel est en place et les routes sont configurés (par le programme sur VM1-6 et 3-6, par le salt pour les autres) et on peut ping VM3 depuis VM1 et inversement.
