configreadermake: configreaderv2.c
     gcc -o configreader configreaderv2.c -I.
     gcc -o server server.c -I.