# **UDP c++ Grupi 30**
### Çka është UDP?

UDP (User Datagram Protocol) është një protokoll komunikimi i rrjetit që dërgon mesazhe shumë shpejt dhe pa krijuar lidhje të qëndrueshme si TCP. Në UDP, klientët dhe serveri thjesht dërgojnë “paketa” mes vete, pa kontrolluar nëse paketa ka ardhur, është humbur apo është marrë në rregull.

#### Avantazhet e UDP-së
* __Komunikim i shpejtë__ sepse nuk ka overhead për kontrollin e gabimeve, rregullimin e renditjes
* __Nuk kërkon lidhje të përhershme__ , ndryshe nga TCP, nuk ka nevojë për “connection” të hapur gjatë gjithë kohës.
* Çdo klient mund të dërgojë __mesazhe në çdo moment__ – mesazhet dërgohen direkt në server pa krijuar lidhje të veçanta.
* Mbështetje për __shumë klientë njëkohësisht__ – ideali për aplikacione me përdorues të shumtë që dërgojnë paketa të shpejta.


### Përshkrimi i Projektit
Projekti përbëhet nga 2 file-a
* __Server.cpp__
    * Serveri pranon lidhje nga 4 deri në 6 klientë njëkohësisht dhe kërkon që të paktën 4 klientë të jenë të lidhur përpara se të lejojë ekzekutimin e komandave 
    * Serveri monitoron në kohë reale trafikun dhe gjendjen e lidhjeve, dhe ndërpret lidhjet e klientëve që nuk janë aktivë pas 30 sekondash.
* __Client.cpp__
    * Klientët e thjeshtë kanë qasje vetëm për lexim
    * Vetëm klienti që ekzekutohet në të njëjtin kompjuter me serverin (pra me të njëjtën IP adresë) fiton automatikisht privilegjet e administratorit që realizon një sistem të shpërndarjes dhe menaxhimit të skedarëve në rrjet lokal.



---
### Privilegjet dhe komandat e mundshme:
* Klientët (client): 
    * __/list__ → shfaq listën e skedarëve në server
    * __/read__ 'FileName' Shfaq përmbajtjen e file-it  
    * __exit__ Ndal lidhjen
* Administratori:
    * __/list__ → shfaq listën e skedarëve në server    
    * __/read__ 'FileName' Shfaq përmbajtjen e file-it 
    * __/upload__ 'FileName' 'përmbajtja' → ngarkon një skedar të ri në server
    * __/download__ 'FileName' → lexon dhe shfaq përmbajtjen e një skedari
    * __/search__ 'word' → kërkon skedarë që përmbajnë fjalën në emër
    * __/info__ 'FileName' → tregon madhësinë dhe datën e modifikimit të skedarit
    * __/delete__ 'FileName' → fshin një skedar nga serveri
    * __/stats__ shfaq të gjitha statistikat aktuale dhe historike të serverit direkt në klient
    * __exit__ Ndal lidhjen
* Serveri:
    * chat_log.txt
        * Çdo mesazh që nuk është PING regjistrohet me funksionin log_message tek chat_log.txt. Çdo hyrje në log përmban timestamp, IP-në dhe portin e klientit, si dhe mesazhin e dërguar.
        ```
        [Sat Nov 16 12:34:56 2025] 192.168.178.50:51234 -> /read file1.txt
        ```
    * server_stats.txt
        * Funksioni stats_thread shkruan çdo 5 sekonda gjendjen aktuale të klientëve dhe trafikun duke i ruajtur tek server_stats.txt. Ai tregon 
            * Sa klientë janë aktivë
            * Sa byte kanë dërguar/marrë
            * Kush është admin
            * Kush është aktiv
        ```
        === STATS Sat Nov 16 12:35:00 2025===
        Kliente aktive: 2 (min: 2 | max: 3)
        192.168.178.50:51234 | Admin:JO | Msg:5 | Recv:124B | Sent:87B
        192.168.178.36:8080 | Admin:PO | Msg:2 | Recv:64B | Sent:45B
        TOTAL - Recv: 188B | Sent:132B
        ```
 
----
# Perdorimi i Programit
### Parakushtet:
* Sigurohu të instalosh Visual Studio apo ndonjë kompajller tjetër për c++
* Sigurohu të vendosësh IPv4 addres-ën aktuale të serverit tek variabla " const string SERVER_IP = X " tek file-i i server-it dhe client-it
### Kompajllimi i file-ave:
Hapni dy dritare të “Developer Command Prompt for VS”: njëra përserverin  dhe tjetra për administratorin.<br>
Sigurohu që të jesh në direktorinë ku ndodhen skedarët server.cpp dhe client.cpp. Dhe ekzekuto këto komanda në njërën prej dritareve.
```
    cl client.cpp /EHsc /std:c++17 Ws2_32.lib
    cl server.cpp /EHsc /std:c++17 Ws2_32.lib
```
### Startimi i programit:
Tani, në njërën dritare ekzekuto server.exe, ndërsa në tjetrën ekzekuto client.exe.<br>
__Server.exe__ 
``` 
C:\UDP_Server>server.exe
==================================================
 UDP SERVER AKTIV - 192.168.178.36:8080
 Admin = Vetem IP: 192.168.178.36
 Minimumi 4 kliente, maksimumi 6
==================================================
```
__Client.exe__
```
C:\UDP_Server>client.exe
UDP SERVER - 192.168.178.36:8080
IP-ja jote: 192.168.178.36 -> ADMIN
ADMIN: /list, /read, /stats, /search, /download, /upload, /delete, exit + chat
>
```
### Puna me program
Nga këtu duhen edhe 3 pajisje të tjera (1 administrator dhe 3 klientë të thjeshtë) për të mundësuar pranimin e komandave.

---

### Pjesëtarët e Grupit 
---
* __Albison Bekaj__
* __Ali Shoshi__
* __Leona Zullufi__
* __Floridë Suka__ 