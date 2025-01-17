# **HTTP-Proxy**

**Un proxy HTTP care permite interceptarea, modificarea și gestionarea traficului web printr-o interfață grafică minimalistă.**

---

## **Cuprins**
1. [Despre proiect](#despre-proiect)
2. [Funcționalități](#funcționalități)
3. [Capturi de ecran](#capturi-de-ecran)
4. [Tehnologii folosite](#tehnologii-folosite)

---

## **Despre proiect**

**HTTP-Proxy** este un tool C++ multithreaded, cu o interfață grafică creată în Qt, care permite interceptarea și manipularea requesturilor HTTP. Scopul aplicației este de a oferi o unealtă simplă pentru depanarea și analiza traficului web, precum și pentru a învăța mai multe despre protocoalele HTTP.

### **Caracteristici generale**
- Interceptează requesturi HTTP și răspunsurile aferente.
- Permite modificarea requesturilor înainte de a le trimite mai departe.
- Afișează și memorează toate requesturile și răspunsurile în timp real.
- Salvează loguri detaliate în fișierul `log.txt`.
- Multithreading pentru gestionarea mai multor conexiuni simultane.

---

## **Funcționalități**

- ✅ **Interceptare trafic HTTP**: Preia și analizează requesturile trimise de client către server.
- ✅ **Modificare requesturi**: Editează requesturile interceptate dintr-o interfață grafică minimalistă.
- ✅ **Log detaliat**: Salvează toate requesturile și răspunsurile într-un fișier `log.txt` pentru debugging și referințe ulterioare.
- ✅ **Gestionare multi-conexiune**: Utilizează multithreading pentru a permite procesarea simultană a mai multor requesturi.
- ✅ **Listă interactivă**: Vizualizează toate requesturile și răspunsurile într-o listă interactivă din aplicație.

---

## **Capturi de ecran**

*Adaugă imagini aici pentru a arăta interfața grafică și exemple de utilizare.*  

![Screenshot - Interfața principală](path_to_screenshot_1.png)  
![Screenshot - Editarea unui request](path_to_screenshot_2.png)

---

## **Tehnologii folosite**

- **C++**: Backendul aplicației, inclusiv procesarea requesturilor și răspunsurilor.
- **Qt**: Crearea interfeței grafice minimaliste.
- **Multithreading**: Gestionarea conexiunilor HTTP multiple simultan.
- **Log Management**: Salvarea datelor în fișierul `log.txt` pentru referințe.
