# 🎵 Riddl Tune – Music Rebus

Joc de rebus muzical în C++ (proiect POO). Primești o listă de melodii, ghicești artistul fiecărui rând, iar **coloana evidențiată** (A → B) formează un **cuvânt bonus** ascuns. Ghicești cuvântul și literele lui apar în grilă. 🎶

Jocul are **două interfețe** peste aceeași logică: o versiune în **consolă** (🖥️) și una **grafică SFML** (🎨).

---

## ✨ Cum se joacă

1. Fiecare rând arată titlul unei melodii și casuțe goale `_` pentru litera fiecărui caracter din numele artistului.
2. Ghicești artistul → rândul se completează și primești puncte.
3. Coloana `[ ]` (de la **A** la **B**) ascunde un cuvânt vertical. Ghicește-l pentru bonus. 🎁
4. Ai nevoie de ajutor? Ceri un **indiciu** (costă puncte), dar o ghicire greșită scade și ea scorul.
5. Runda se termină când toate rândurile sunt rezolvate. ✅

| Eveniment | Efect asupra scorului |
|---|---|
| Rând ghicit | + puncte pentru fiecare literă ascunsă |
| Cuvânt bonus ghicit | + bonus |
| Indiciu | − penalizare |
| Răspuns greșit | − penalizare (scorul nu scade sub 0) |

Valorile se configurează în `assets/config.json`.

---

## 🧱 Structura proiectului

```
proiect_poo/
├── include/Riddle.h        # interfața publică a bibliotecii
├── src/Riddle.cpp          # logica jocului (bibliotecă)
├── app/
│   ├── main.cpp            # aplicația în consolă (scenariul cu tastatura.txt)
│   ├── gui_main.cpp        # punctul de intrare al aplicației grafice
│   ├── RiddleGUI.h / .cpp  # interfața grafică SFML
├── assets/
│   ├── tracks.json         # melodiile și artiștii
│   ├── config.json         # cuvinte bonus + scor
│   └── font.ttf            # fontul interfeței grafice
├── tests/exemplu_test.cpp  # teste cu assert
└── tastatura.txt           # intrările pentru scenariul din consolă
```

### 🧩 Clase principale

- **`SongTrack`**: o melodie (titlu + artist), curăță diacriticele și compară răspunsurile.
- **`RebusRow`**: un rând din grilă (litere vizibile/ascunse, poziția literei bonus).
- **`RebusBoard`**: tabla completă; o construiește recursiv (backtracking), respectă **Rule of Three** și afișează grila cu `operator<<`.
- **`GameSession`**: sesiunea de joc (catalog, scor, configurare, runde).
- **`RiddleGUI`**: interfața grafică, care folosește doar metodele publice de mai sus.

---

## 🛠️ Compilare

Ai nevoie de **CMake ≥ 3.28** și de un compilator C++23. Dependențele (SFML, nlohmann/json) se descarcă automat la prima configurare, deci ai nevoie de internet. 🌐

```bash
cmake -S . -B build
cmake --build build
```

> 🐧 **Linux:** pentru SFML instalează înainte pachetele de dezvoltare (X11, OpenGL, freetype, udev).
> 🪟 **Windows / CLion:** *Tools → CMake → Reset Cache and Reload Project*.

---

## ▶️ Rulare

Rulează din folderul `build` (acolo se copiază `assets/` și `tastatura.txt`).

**🖥️ Consolă (scenariul cerut de temă, intrări din fișier):**

```bash
./oop_main < tastatura.txt
```

Pe Windows, în `cmd`: `oop_main.exe < tastatura.txt`. Poți rula și interactiv, fără fișier, cu `./oop_main`.

**🎨 Interfață grafică:**

```bash
./oop_gui
```

### ⌨️ Controale (interfața grafică)

| Acțiune | Tastă / buton |
|---|---|
| Alege rândul | ↑ / ↓ sau click pe rând |
| Trimite răspunsul | **Enter** |
| 💡 Indiciu | **Ctrl+H** |
| 🎁 Cuvânt bonus | **Ctrl+B** sau **Tab** |
| 👀 Arată soluția | **Ctrl+R** |
| 🔄 Rundă nouă | **Ctrl+N** |
| Ieșire | **Esc** |

### 🔤 Meniul din consolă

`1` ghicește rând · `2` indiciu · `3` cuvânt bonus (A-B) · `4` arată soluția · `5` rundă nouă · `0` ieșire

---

## ⚙️ Configurare

În `assets/config.json`:

```json
{
  "keywords": ["CONCERT", "FESTIVAL", "MELODIE", "PLAYLIST", "MUZICA", "REFREN", "HITURI"],
  "shuffle": true,
  "pointsPerLetter": 10,
  "keywordBonus": 50,
  "hintPenalty": 5,
  "wrongPenalty": 3
}
```

Poți adăuga cuvinte bonus și melodii noi în `assets/tracks.json`, **fără să recompilezi**. 🎼

---

## 🧪 Teste

```bash
cmake --build build --target oop_test
./build/tests/oop_test
```

Testele verifică normalizarea textului, potrivirea artistului, generarea tablei și completarea cuvântului bonus.

---

## 📚 Resurse

- [nlohmann/json](https://github.com/nlohmann/json) – citirea fișierelor JSON
- [SFML 3.0.0](https://www.sfml-dev.org/) – interfața grafică
