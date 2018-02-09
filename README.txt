Sortowanie współbieżne w pamięci dzielonej

Filip Binkiewicz
332069

1. Schemat rozwiązania

Rozwiązanie jest zgodnie ze specyfikacją podaną w treści zadania.
Jedyna część logiki nieopisana w treści dotyczy zakończenia wszystkich
procesów sortujących. Wykorzystałem następujące fakty:
    - Każdy z procesów A oraz B musi wykonać n kroków w pesymistycznym przypadku
    - Po n krokach procesów A i B tablica jest posortowana
    - Jeśli dla pewnego i, 1 <= i <= n, żaden z procesów A i B
      nie wykonał zamiany w tablicy, to tablica jest posortowana
      w kroku i

Po tych obserwacjach implementacja jest oczywista: w każdym kroku
procesy A i B dzielą informację, czy wykonały zamianę. Jeśli któryś
proces zauważy, że w pewnym kroku bezczynnych było 2 * n - 1 procesów,
może rozpropagować informację, że sortowanie zostało zakończone.

Oczywiście rozwiązanie to nie gwarantuje, że żaden proces nie wykona
niepotrzebnie pustych kroków. W praktyce jednak okazuje się, że
przy nieco dłuzszych posortowanych tablicach (długości rzędu 1000),
wszystkie procesy kończą po maksymalnie kilkudziesięciu krokach.
Wszystko zależy od początkowego stanu tablicy i złośliwości planisty

2. Wykorzystane zmienne

Wykorzystałem zaalokowane w (anonimowej) pamięci współdzielonej następujące zmienne:

    - shared_array - tablica do posortowania
    - idle_count - tablica długości n, mówiąca, ile procesów było bezczynnych
      w danym kroku
    - sorting_done - mówiąca, czy któryś z procesów ustalił, że
      tablica została posortowana

Do synchronizacji zostały wykorzystane nastepujące semafory:
    - mutex 
    - n semaforów A
    - n - 1 semaforów B

Zgodnie z nazwą, mutex służy do modyfikowania współdzielonych zmiennych,
a każdy z procesów A i B ma osobisty semafor, na którym czeka, aż procesy,
od których pracy zależy, wykonają swój krok.

