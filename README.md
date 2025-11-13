# **UDP c++ Grupi 30**
### Pjesëtarët e Grupit 
---
* __Albison Bekaj__
* __Ali Shoshi__
* __Leona Zullufi__
* __Floridë Suka__ 

### Përshkrimi i Projektit
Programi përbëhet nga një server UDP dhe një klient i thjeshtë që realizon një sistem të shpërndarjes dhe menaxhimit të skedarëve në rrjet lokal. Serveri pranon lidhje nga 4 deri në 6 klientë njëkohësisht dhe kërkon që të paktën 4 klientë të jenë të lidhur përpara se të lejojë ekzekutimin e komandave (përveç PING-ut). Vetëm klienti që ekzekutohet në të njëjtin kompjuter me serverin (pra me të njëjtën IP adresë) fiton automatikisht privilegjet e administratorit; të gjithë të tjerët kanë qasje vetëm për lexim. Serveri ofron monitorim në kohë reale të trafikut dhe të gjendjes së lidhjeve, regjistron statistika çdo 5 sekonda në skedarin server_stats.txt dhe zbaton timeout 30 sekondash për klientët joaktivë.

---
### Privilegjet dhe komandat e mundshme:
* Klientët (client): 
    * __/list__ → shfaq listën e skedarëve në server
    * __/read__ → Shfaq përmbajtjen e file-it  
    * __/download__ 'emri' → lexon dhe shfaq përmbajtjen e një skedari
    * __/search__ 'fjal' → kërkon skedarë që përmbajnë fjalën në emër
    * __/info__ 'emri' → tregon madhësinë dhe datën e modifikimit të skedarit
    * __exit__ Ndal lidhjen
* Administratori:
    * __/upload__ 'emri' 'përmbajtja' → ngarkon një skedar të ri në server
    * __/delete__ 'emri' → fshin një skedar nga serveri
    * __/stats__ → shfaq të gjitha statistikat aktuale dhe historike të serverit direkt në klient

----