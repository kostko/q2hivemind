# Core (infrastruktura) #
  * ~~Dodati razred `Dispatcher`, ki naj omogoča thread-safe komunikacijo dogodkov med posameznimi deli bota (predvsem med lokalnim in globalnim planerjem)~~ (**kostko**)
  * ~~Kako (iz seznama entitet?) ugotoviti kdo je nasprotnik in kdo team member.~~ (**kostko**)
  * ~~Stackanje stanj in zahtevanje prekinitve s strani stanja~~ (**kostko**)
  * ~~Odstraniti stackanje stanj in implementirati seznam trenutno primernih stanj na podlagi triggerjev~~ (**degi**)
  * ~~Dokončati podporo za pridobivanje inventoryja~~ (**kostko**)
  * ~~Popraviti handling stanj~~ (**degi**)
  * ~~Dodaj flag, ki izbere mode igranja; explore (ucenje) ali exploit mode (uporaba pridobljenega znanja).~~ (**zomb**)

# Map #
  * ~~Preimenovati `link_t` v `MapLink` in ga narediti bolj fleksibilnega za sprotne posodobitve različnih metapodatkov (npr. uteži)~~ (**kostko**)
  * ~~Implementirati A`*` iskanje za path finding~~ (**kostko**)
  * ~~Izboljšati iskanje naključne poti z uporabo metapodatkov o zadnji obiskanosti povezave oz. dela zemljevida~~ (**degi**)
    * ~~Mogoče  bi bilo bolje, da so ne dolgo nazaj obiskane poti samo deprioritizirane pri izbiri, saj se sicer bot lahko na kakšnem mestu ustavi za nekaj sekund, ker je vse poti iz trenutnega vozlišča že obiskal.~~
  * ~~Implementirati avtomatsko učenje topografije aka. hivemind dynamic mapping grid~~ (**kostko**)
    * ~~Vozlišča lahko vsebujejo tudi metapodatke (npr. da se tam nahaja določen item)~~
    * ~~Pametno bi bilo dodati tudi "medij" vozlišča (npr. voda, zrak, tla, dvigalo); potem je možno narediti metodo v smislu "poišči mi bližnjo točko, ki ni v vodi" in pot do nje~~

# Lokalni planer #
  * ~~V določenih točkah se bot "zatakne" tako da se začne vrteti okrog točke, vendar je nikoli ne doseže~~ (**kostko**)
  * ~~Določiti in implementirati posamezna stanja bota; prav tako določiti interakcije med stanji ter način preklopa~~ (**zomb**)
  * ~~Integracija reinforcement learning podsistema~~ (**zomb**)
    * Način deljenja znanja pridobljenega preko RL z drugimi boti preko dogodkov; znanje drugih je _nekaj_ manj pomembno/obteženo kot neposredno pridobljeno znanje lokalne instance
    * ~~Implementirati uvoz in izvoz "znanja"~~ (**zomb**)
  * ~~Implementirati `SwimState`, tako da zna bot vsaj splavati vn iz vode če pade notr~~ (**degi**)
  * ~~Implementirati `RespawnState`, v katerega bot preide ko je ubit~~ (**degi**)
  * ~~Implementirati `ShootState` v katerem bot strelja nasprotnika.~~ (**zomb**)
    * ~~Implementirati izbiro tarč~~
  * ~~Implementirati `GoToState` v katerem bot najde pot do zahtevane entitete (drug bot, ammo, health pack, weapon..)~~ (**zomb**)
    * ~~ter implementirati izbiro itema za pobrat~~
    * ~~evaluate funkcije, ki assignajo ustrezno vrednost itemom glede na trenutne potrebe~~(**zomb**)
  * Implementirati `DropWeaponState`, kjer bot dropne drugemu botu weapon za pobrat (**degi**)
    * ce je bot dovolj blizu in ima na voljo weapon za dropat
    * preko MOLD poslji respawnanemu botu msg da pridem dropat
    * ko dobim konfirmacijo od bota, odidem na tisto mesto in dropnem
    * ~~ugotoviti, katere weapone ima bot na voljo in ce lahko kaj dropne~~
    * ~~se odlociti, kater weapon bo dropnil in ga dropniti~~
  * Implementirati `CamperState`, kjer bot caka, da mu bo kdo kaj dropnil (**kostko**)
  * ~~Implementirati proper obstacle avoidance~~ (**kostko**)
  * ~~Bot naj ima v roki vedno najboljsi trenuten weapon iz inventoryja~~ (**degi**)

# Globalni planer #
  * ~~Integracija MOLD in globalnega planerja - samo globalni planer lahko neposredno uporablja funkcije MOLD-a, drugi deli bota dobijo/posredujejo ustrezne dogodke preko osrednjega dispečerja~~ (**kostko**)
  * ~~Dodati centralni imenik znanih botov z možnostjo vodenja nekaterih metapodatkov~~ (**kostko**)
  * Implementirati podatkovno strukturo za sestavljeni pogled na svet, ki naj bo uporabna lokalni instanci bota (**kostko**)
    * Tukaj gre za pogled na svet dinamičnih entitet (npr. drugih igralcev), statični podatki (npr. informacije o tem kje se spawnajo posamezni itemi ipd.) se hranijo v mapping gridu
    * Mogoče bi se tudi ti podatki hranili v gridu (seveda v ločenem kd-drevesu, ki bi se dosti spreminjalo)

**Opomba:** Vse podatkovne strukture, ki vključujejo podatke prejete od drugih instanc botov morajo implementirati časovno žigosanje svojih elementov zaradi potrebe sinhronizacije. Vse instance morajo imeti natančno sinhronizirano uro, najbolje kar preko `ntpd`. Za to naj se uporablja vgrajeni tip `timestamp_t` in metoda `Timing::getCurrentTimestamp()` iz `timing.h` headerja.

# MOLD (Message Oriented Lightweight Distributor) #
  * ~~Če se _slučajno_ izkaže da je CAST prevelik crap za uporabljat obstaja možnost da se napiše svoj enostaven message bus z uporabo Boost ASIO ter Google ProtoBuf~~ (**kostko**)