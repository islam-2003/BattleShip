======
 Team
======

- Coia Pascal - 000540745
- Lo Cascio Rosario - RosarioLC - 000546687
- Malouch Nikita - Aerynnisss - 000545795
- Mekhiouba Islam - 000538266
- Mertens Bryan - Ayat0ooo - 000522960
- Sanchez Espinosa Carlos - carsanche - 000514371
- Speilers Capucine - Spl4sh9 - 000540555
- Truong Nha - Minti - 000576343
- Veyret Danae - Phantom - 000570552

==========
Librairies
==========

Afin de pouvoir compiler le programme GUI, il est nécessaire d'installer la librairie suivante :

  SFML : sudo apt-get install libsfml-dev

===========
Compilation
===========

``make terminal`` ou ``make gui`` pour créer le programme ``server`` ainsi que respectivement ``client_terminal`` ou ``client_gui``.

Il n'est pas possible de compiler les deux clients en même temps.
Il est recommandé de ``make clean`` avant de changer de client.

``make clean`` supprime les fichiers ``.o`` et ``.d``.

``make mrclean`` supprime les exécutables en plus des fichiers ``.o`` et ``.d``.
