#kako uporabiti RL v botih?

# Q-learning #
Zdi se mi, da je za začetek OK, če implementiramo "Q-learning" algoritem, ker je zelo osnoven in se ga da potem "nadgraditi" v SARSA algoritem (če bo čas :P).

## Osnovna ideja RL ##
Okolje agenta ima končno število stanj S in končno število akcij A(s), za vsak s € S.
Za vsak prehod iz stanja s v novo stanje s' preko akcije a, dobi agent nagrado r = R(s, a, s').
Ideja je, da agent poskuša skozi čas optimizirati neko funkcijo R (kot Reward) na način da bo vsota nagrad skozi čas največja.

Q-learning vpelje funkcijo ![http://upload.wikimedia.org/math/b/b/4/bb439e462dcecbd54fb854e643ead16e.png](http://upload.wikimedia.org/math/b/b/4/bb439e462dcecbd54fb854e643ead16e.png), ki oceni kvaliteto akcije a pri danemu stanju s. Ideja je, da se vsakič, ko agent zamenja stanje popravi vrednost Q na podlagi nove informacije (npr. kakšne so sedaj možnosti, ko je napravil dano akcijo):
![http://upload.wikimedia.org/math/5/2/1/521487e03f689b5531b46684b56e4217.png](http://upload.wikimedia.org/math/5/2/1/521487e03f689b5531b46684b56e4217.png)

Začetne Q vrednosti se določijo vnaprej. Catch je določiti tudi smiselne rewarde.

# Boti #
Določiti moramo stanja okolja S, akcije za posamezno stanje in začetno kvaliteto Q.
Pomoje, da je dost primerno,da naredimo tako kot smo se pogovarjali;
naprimer, da bodo stanja za začetek tupli v smislu:
  * `(health v nekem intervalu, weapon, ammo v nekem intervalu, vidim nasprotnika, team state)` team state bi bil lahko recimo razlika v fragih med ekipama, da lahko lepo računamo rewarde.
  * akcije pa npr. za `A((50 <, blaster, inf, FALSE, -3)) = { find heath pack, find weapon, explore }` potem pa naprimer za `A((100, machine gun, 200, TRUE, 10)) = { attack, run away, find weapon }` ipd...

Malo bo treba eksperimentirati še z 'learning rate' faktorjem (0 pomeni, da se ne nauči nič, 1 pomeni, da upošteva le pravkar dobljeno informacijo) ter z 'discount' faktorjem (0 pomeni, da je bot oportunist, 1 pomeni, da želi dolgoročne nagrade).

Premisliti je potrebno še kako implementirati funkcijo Q. Baje je najbolje, če se Q vrednosti računa z ANN (kar je kul bonus za UI2) :), ampak za začetek pa mogoče z lookup tabelo; c++ koda kako se to da lepo implementirati: http://www.compapp.dcu.ie/~humphrys/Notes/RL/Code/index.html