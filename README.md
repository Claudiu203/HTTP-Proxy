# HTTP-Proxy
1. Cererea si configurarea:
Folosim socket server TCP/IP-asculta pt conexiuni HTTP
Configurare pentru mai multe conexiuni: gestionarea fiecarei conexiuni intr-un thread separat (mai multe cereri paralel), astfel încât fiecare client să poată trimite cereri independent de ceilalți.

2. Gestionarea cereri de la clienti:
Clientul trimite cererea catre proxy.
Când o cerere este primită de la client, proxy-ul o parsează și, opțional, modifică header-ele.
Creează o conexiune către Google (sau alt server specificat) și trimite cererea clientului mai departe.
Proxy-ul primește răspunsul serverului de destinație, îl poate modifica (de exemplu, adăugând un header) și îl trimite înapoi către client.

3. interfata Grafia:
Interfața Grafică în Qt C++
Vom folosi Qt pentru a crea ferestre, butoane și alte elemente vizuale necesare aplicației.
Crearea unei ferestre Principale:
        Proiectăm o fereastra principală care va conține toate elementele necesare pentru controlul serverului proxy.
Buton pentru Pornirea Serverului Proxy:
        Implementăm un buton dedicat pentru a porni serverul proxy, care va iniția funcția corespunzătoare pentru a lansa serverul.
Afișarea Cererilor:
        Adăugăm un widget text care va afișa cererile active și răspunsurile de la server.
        Permitem utilizatorului să modifice cererile, oferind astfel opțiuni pentru configurarea comportamentului serverului proxy.
Opțiuni de Configurare:
        Oferim opțiuni suplimentare în interfață pentru a ajusta comportamentul proxy-ului, cum ar fi filtrarea cererilor sau modificarea header-elor HTTP.

4. Extensii proiect:
	Jurnalizare detaliată: Loghează cererile și răspunsurile procesate pentru a putea urmări traficul.
	Modificări mai avansate ale cererilor/răspunsurilor: Oferă opțiuni pentru filtrarea cererilor bazate pe URL sau alte criterii (ex. blocarea anumitor site-uri).
	Monitorizarea traficului: Vizualizarea cererilor active și informații despre dimensiunea traficului.