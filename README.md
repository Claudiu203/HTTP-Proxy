1. Cererea si configurarea:
Folosim socket server TCP/IP-asculta pt conexiuni HTTP
Configurare pentru mai multe conexiuni: gestionarea fiecarei conexiuni intr-un thread separat (mai multe cereri paralel), astfel încât fiecare client să poată trimite cereri independent de ceilalți.
2. Gestionarea cereri de la clienti:
Clientul trimite cererea catre proxy.
Când o cerere este primită de la client, proxy-ul o parsează și, opțional, modifică header-ele.
Creează o conexiune către Google (sau alt server specificat) și trimite cererea clientului mai departe.
Proxy-ul primește răspunsul serverului de destinație, îl poate modifica (de exemplu, adăugând un header) și îl trimite înapoi către client.
3. Interfata Grafia:
Vrem sa folosim PyQt sau PySide.
Folosim PyQt sau PySide pentru a crea ferestre, butoane, și alte elemente vizuale.
Creăm un buton de pornire pentru serverul proxy care inițiază funcția start_proxy_server.
Afisam cererea si permitem modificarea ei (Adăugăm afișaj pentru a vedea cererile active și eventual să oferim opțiuni de configurare a comportamentului proxy-ului.)
4. Extensii proiect:
Configurare avansată în GUI: Adaugă opțiuni suplimentare pentru configurarea proxy-ului direct din interfața grafică, cum ar fi schimbarea serverului de destinație, portul de ascultare, sau moduri de autentificare.
Jurnalizare detaliată: Loghează cererile și răspunsurile procesate pentru a putea urmări traficul.
Modificări mai avansate ale cererilor/răspunsurilor: Oferă opțiuni pentru filtrarea cererilor bazate pe URL sau alte criterii (ex. blocarea anumitor site-uri).
Monitorizarea traficului: Vizualizarea cererilor active și informații despre dimensiunea traficului.