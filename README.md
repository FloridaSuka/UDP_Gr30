# **UDP c++ Grupi 30**
### Çka është UDP?

UDP (User Datagram Protocol) është një protokoll komunikimi i rrjetit që dërgon mesazhe shumë shpejt dhe pa krijuar lidhje të qëndrueshme si TCP. Në UDP, klientët dhe serveri thjesht dërgojnë “paketa” mes vete, pa kontrolluar nëse paketa ka ardhur, është humbur apo është marrë në rregull.

#### Pse përdorim UDP në këtë projekt?
* UDP është shumë i shpejtë
* Nuk ka nevojë për “connection” permanente (si TCP)
* Çdo klient mund t’i dërgojë serverit mesazhe në çdo moment
* Serveri mund t’u përgjigjet klientëve pa hapur lidhje të veçanta
* Perfekt për situata kur na duhen:
* Shumë klientë njëkohësisht
* Komunikim i lehtë
* Trafik i vogël, mesazhe të shpejta

### Përshkrimi i Projektit
Programi përbëhet nga një server UDP dhe një klient i thjeshtë që realizon një sistem të shpërndarjes dhe menaxhimit të skedarëve në rrjet lokal. Serveri pranon lidhje nga 4 deri në 6 klientë njëkohësisht dhe kërkon që të paktën 4 klientë të jenë të lidhur përpara se të lejojë ekzekutimin e komandave (përveç PING-ut). Vetëm klienti që ekzekutohet në të njëjtin kompjuter me serverin (pra me të njëjtën IP adresë) fiton automatikisht privilegjet e administratorit; të gjithë të tjerët kanë qasje vetëm për lexim. Serveri ofron monitorim në kohë reale të trafikut dhe të gjendjes së lidhjeve, regjistron statistika çdo 5 sekonda në skedarin server_stats.txt dhe zbaton timeout 30 sekondash për klientët joaktivë.



---
### Privilegjet dhe komandat e mundshme:
* Klientët (client): 
    * __/list__ → shfaq listën e skedarëve në server
    * __/read__ → Shfaq përmbajtjen e file-it  
    * __exit__ Ndal lidhjen
* Administratori:
    * __/upload__ 'emri' 'përmbajtja' → ngarkon një skedar të ri në server
    * __/download__ 'emri' → lexon dhe shfaq përmbajtjen e një skedari
    * __/search__ 'fjal' → kërkon skedarë që përmbajnë fjalën në emër
    * __/info__ 'emri' → tregon madhësinë dhe datën e modifikimit të skedarit
    * __/delete__ 'emri' → fshin një skedar nga serveri
    * __/stats__ → shfaq të gjitha statistikat aktuale dhe historike të serverit direkt në klient
* Serveri:
    Ruanë të gjitha mesazhet në chat_log.txt
    Ruanë statistika çdo 5 sekonda në server_stats.txt
    Tregon sa klientë janë aktivë
    Sa byte kanë dërguar/marrë
    Kush është admin
    Kush është aktiv
    Bën timeout të klientëve joaktivë  
----
### Startimi i programit:
    Starto serverin
 
server.exe
```
Serveri nis dhe pret klientë.
    
     Starto klientin


client.exe
```
Klienti tregon:
IP-n lokale
nëse je ADMIN apo KLIENT i thjeshtë

### Pjesëtarët e Grupit 
---
* __Albison Bekaj__
* __Ali Shoshi__
* __Leona Zullufi__
* __Floridë Suka__ 