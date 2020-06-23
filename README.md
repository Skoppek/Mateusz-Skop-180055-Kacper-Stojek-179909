# Mateusz-Skop-180055-Kacper-Stojek-179909
Projekt 4 - Techniki programowania

Mateusz Skop 180055 ACiR1, Kacper Stojek 179909 ACiR1

1. Informacje wstępne

  - Projekt został wykonany na bazie przykładowego programu zamieszczonego na platformie eNauczanie.
  - Po uruchomieniu programu należy rozpocząć pracę od kliknięcia przycisku RESET.
  
2. Opis przycisków

  - "1/200, 1/100, 1/50, 1/25, 1/1" - Zmiana prędkości. Największa prędkość 1/1, najmniejsza prędkość 1/200. Prędkość domyślna 1/1.
  - "NAGRYWAJ" - Rozpoczęcie nagrywania ruchu robota przy sterowaniu manualnym.
  - "ODTWÓRZ" - Odtworzenie nagranych ruchów.
  - "AUTOMAT" - Uruchomienie sortowania pudełek.
  - "RESET" - Ustawienie programu do stanu wyjściowego.
  - "<-- , -->" - Kontrola czerwonego ramienia.
  - "<- , ->" - Kontrola niebieskiego ramienia.
  - "STOP" - Zatrzymanie robota. (tylko przy kontroli manualnej)
  - "CHWYĆ/PUŚĆ" - Próba chwycenia lub puszczenia pudełka.
  - "+ , -" - Zmiana wag pudełek. (od lewej: żółtego, zielonego, niebieskiego, czerwonego)
  
3. Praca manualna

  Jednorazowe wciśnięcie jednego z przycisków "<--, -->, <-, ->".
  Rozpoczyna ruch wybranego ramienia w wybranym kierunku.
  Przycisk "STOP" zatrzymuje poruszające sie ramię.
  Niemożliwe jest ustawienie końcówki ramienia poniżej górnej części leżacych pudełek oraz obrócenie ramion tak, aby przez siebie         przeszły pokrywając się.
  W trakcie ruchu robota i poza nim można zmieniać prędkość ruchu ramienia opisanymi wyżej przyciskami.
  Zamiana prędkości jest natychmiastowa i nie wymaga zatrzymywania robota.
  Aby chwycić pudełko należy zbliżyć końcówkę ramienia robota do środka wybranego pudełka i kliknąć przycisk "CHWYĆ"
  Po udanym chwycie napis na przycisku powinien zmienić się na "PUŚĆ".
  Po kliknięciu przycisku "PUŚĆ" pudełko zostanie odczepione i spadnie na ziemię.
  
4. Nagrywanie i odtwarzanie ruchu

  Po kliknięciu przycisku "NAGRYWAJ" ramię robota natychmiast ustawi się w pozycji wyjściowej.
  Od tej chwili każdy ruch będzie zapisywany w kolejce ruchów 'record_queue'.
  Aby zakończyć nagrywanie należy ponownie wcisnąć przycisk "NAGRYWAJ" lub wcisnąć przycisk "ODTWÓRZ" co jednocześnie ustawi robota do pozycji wyjściowej i zacznie odtwarzać nagrany ruch.
  
5. Praca automatyczna / sortowanie

  Po kliknięciu przycisku "AUTOMAT" robot ustawi się na pozycji wyjściowej i zacznie przekładać pudełka na prawą stronę w kolejności (od lewej) od najlżejszego do nacięższego.
  Wagi pudełek należy ustawić przed uruchomieniem trybu automatycznego.
  
*W czasie odtwarzania ruchu i pracy automatycznej przyciski kontroli manualnej są zablokowane.
