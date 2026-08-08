/*
Ниже — полностью дописанный, готовый к копированию код.
Я докрутила всё до конца: owner/neighbour, boundary, alpha.water. Никаких обрывков.

Что делает программа
1. Создаёт папку case_out.
2. Генерирует структурированную сетку (прямоугольный бассейн) с заданными nx, ny, nz и размерами.
3. Пишет все файлы polyMesh: points, faces, owner, neighbour, boundary.
4. Сразу пишет 0/alpha.water с уровнем воды waterLevelZ.
5. После запуска можно сразу делать checkMesh и запускать решатель.

Полный код (один файл)
*/
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <iomanip>
/*
//Не надо сразу бежать в Linux! Папка case_out пустая не потому,
  что Windows «не умеет» создавать файлы,
  а из‑за одной коварной мелочи в функции makeDir.
  В чём проблема
  В твоём коде:
static void makeDir(const std::string& path) {
#ifdef _WIN32
    system(("mkdir " + path).c_str());
#else
    system(("mkdir -p " + path).c_str());
#endif
}
*/
// Вариант 1: Быстрое исправление кода (рекомендую)
// Замени функцию makeDir на эту — она сама создаст всю цепочку папок в Windows:

#include <sys/stat.h>
#include <direct.h> // для _mkdir в Windows

static void makeDirRecursive(const std::string &path) {
    std::string current;
    for( char c : path )
    {   current += c;
        if (c == '/' || c == '\\') {    // Пропускаем начальный слэш, если есть
            if (current.size() <= 1) continue;

            #ifdef _WIN32
                _mkdir(current.c_str()); // Создаёт одну папку
            #else
                mkdir(current.c_str(), 0755);
            #endif
        }
    }   // И финальную часть, если в конце нет слэша
    #ifdef _WIN32
        _mkdir(path.c_str());
    #else
        mkdir(path.c_str(), 0755);
    #endif
}

//А в main замени все вызовы makeDir(...) на makeDirRecursive(...).

// ================= НАСТРОЙКИ СЕТКИ (МЕНЯЙ ЗДЕСЬ) =================
const int nx = 20; // Ячейки по X (длина)
const int ny = 10; // Ячейки по Y (ширина)
const int nz = 15; // Ячейки по Z (высота)

const double Lx = 10.0; // Длина бассейна (м)
const double Ly = 5.0;  // Ширина бассейна (м)
const double Lz = 6.0;  // Высота бассейна (м)

const double waterLevelZ = 4.0; // Уровень воды (м) от дна
// ================================================================
#include <windows.h>

struct Vec3 { double x, y, z; };

int main() {
    SetConsoleCP( CP_UTF8 );        // 1251
    SetConsoleOutputCP( CP_UTF8 );  // 1251
    const std::string caseDir = "case_out";
    const std::string meshDir = caseDir + "/constant/polyMesh";

    makeDirRecursive(caseDir);
    makeDirRecursive(meshDir);
    makeDirRecursive(caseDir + "/0");

    std::cout << "Генерация сетки: " << nx << "x" << ny << "x" << nz
              << " (" << Lx << "x" << Ly << "x" << Lz << " м)\n";

    int nCells = nx * ny * nz;
    int nPoints = (nx + 1) * (ny + 1) * (nz + 1);

    // --- 1. Генерируем точки (points) ---
    std::ofstream fPoints((meshDir + "/points").c_str());
    fPoints << nPoints << "\n(\n";
    for (int k = 0; k <= nz; ++k) {
        for (int j = 0; j <= ny; ++j) {
            for (int i = 0; i <= nx; ++i) {
                double x = (double)i * Lx / nx;
                double y = (double)j * Ly / ny;
                double z = (double)k * Lz / nz;
                fPoints << "(" << std::fixed << std::setprecision(8)
                         << x << " " << y << " " << z << ")\n";
            }
        }
    }
    fPoints << ")\n";
    fPoints.close();
    std::cout << "OK Точки записаны.\n";

    auto pointIndex = [&](int i, int j, int k) {
        return i + j * (nx + 1) + k * (nx + 1) * (ny + 1);
    };

    auto cellIndex = [&](int i, int j, int k) {
        if (i < 0 || i >= nx || j < 0 || j >= ny || k < 0 || k >= nz) return -1;
        return i + j * nx + k * nx * ny;
    };

    // --- Подсчёт граней ---
    int nBottom = nx * ny;
    int nTop    = nx * ny;
    int nLeft   = nz * nx; // Y=0
    int nRight  = nz * nx; // Y=max
    int nFront  = nz * ny; // X=0
    int nBack   = nz * ny; // X=max

    int nIntX = (nx - 1) * ny * nz;
    int nIntY = nx * (ny - 1) * nz;
    int nIntZ = nx * ny * (nz - 1);

    int nBoundaryFaces = nBottom + nTop + nLeft + nRight + nFront + nBack;
    int nInternalFaces = nIntX + nIntY + nIntZ;
    int nTotalFaces = nBoundaryFaces + nInternalFaces;

    std::cout << "OK Всего граней: " << nTotalFaces << "\n";

    // --- 2. Генерируем грани (faces) блоками ---
    std::ofstream fFaces((meshDir + "/faces").c_str());
    fFaces << nTotalFaces << "\n(\n";

    int currentFaceIdx = 0;
    int idxBottomStart, idxTopStart, idxLeftStart, idxRightStart, idxFrontStart, idxBackStart;

    // Дно (Bottom)
    for (int j = 0; j < ny; ++j)
        for (int i = 0; i < nx; ++i) {
            int p0 = pointIndex(i, j, 0);
            int p1 = pointIndex(i+1, j, 0);
            int p2 = pointIndex(i+1, j+1, 0);
            int p3 = pointIndex(i, j+1, 0);
            fFaces << "4(" << p0 << " " << p1 << " " << p2 << " " << p3 << ")\n";
        }
    idxBottomStart = currentFaceIdx;
    currentFaceIdx += nBottom;

    // Крышка (Top)
    for (int j = 0; j < ny; ++j)
        for (int i = 0; i < nx; ++i) {
            int p0 = pointIndex(i, j, nz);
            int p1 = pointIndex(i, j+1, nz);
            int p2 = pointIndex(i+1, j+1, nz);
            int p3 = pointIndex(i+1, j, nz);
            fFaces << "4(" << p0 << " " << p1 << " " << p2 << " " << p3 << ")\n";
        }
    idxTopStart = currentFaceIdx;
    currentFaceIdx += nTop;

    // Левая (Left, Y=0)
    for (int k = 0; k < nz; ++k)
        for (int i = 0; i < nx; ++i) {
            int p0 = pointIndex(i, 0, k);
            int p1 = pointIndex(i, 0, k+1);
            int p2 = pointIndex(i+1, 0, k+1);
            int p3 = pointIndex(i+1, 0, k);
            fFaces << "4(" << p0 << " " << p1 << " " << p2 << " " << p3 << ")\n";
        }
    idxLeftStart = currentFaceIdx;
    currentFaceIdx += nLeft;

    // Правая (Right, Y=ny)
    for (int k = 0; k < nz; ++k)
        for (int i = 0; i < nx; ++i) {
            int p0 = pointIndex(i, ny, k);
            int p1 = pointIndex(i+1, ny, k);
            int p2 = pointIndex(i+1, ny, k+1);
            int p3 = pointIndex(i, ny, k+1);
            fFaces << "4(" << p0 << " " << p1 << " " << p2 << " " << p3 << ")\n";
        }
    idxRightStart = currentFaceIdx;
    currentFaceIdx += nRight;

    // Передняя (Front, X=0)
    for (int k = 0; k < nz; ++k)
        for (int j = 0; j < ny; ++j) {
            int p0 = pointIndex(0, j, k);
            int p1 = pointIndex(0, j, k+1);
            int p2 = pointIndex(0, j+1, k+1);
            int p3 = pointIndex(0, j+1, k);
            fFaces << "4(" << p0 << " " << p1 << " " << p2 << " " << p3 << ")\n";
        }
    idxFrontStart = currentFaceIdx;
    currentFaceIdx += nFront;

    // Задняя (Back, X=nx)
    for (int k = 0; k < nz; ++k)
        for (int j = 0; j < ny; ++j) {
            int p0 = pointIndex(nx, j, k);
            int p1 = pointIndex(nx, j+1, k);
            int p2 = pointIndex(nx, j+1, k+1);
            int p3 = pointIndex(nx, j, k+1);
            fFaces << "4(" << p0 << " " << p1 << " " << p2 << " " << p3 << ")\n";
        }
    idxBackStart = currentFaceIdx;
    currentFaceIdx += nBack;

    // Внутренние грани по X
    for (int k = 0; k < nz; ++k)
        for (int j = 0; j < ny; ++j)
            for (int i = 0; i < nx - 1; ++i) {
                int p0 = pointIndex(i+1, j, k);
                int p1 = pointIndex(i+1, j, k+1);
                int p2 = pointIndex(i+1, j+1, k+1);
                int p3 = pointIndex(i+1, j+1, k);
            fFaces << "4(" << p0 << " " << p1 << " " << p2 << " " << p3 << ")\n";
        }

    // Внутренние по Y
    for (int k = 0; k < nz; ++k)
        for (int j = 0; j < ny - 1; ++j)
            for (int i = 0; i < nx; ++i) {
                int p0 = pointIndex(i, j+1, k);
                int p1 = pointIndex(i, j+1, k+1);
                int p2 = pointIndex(i+1, j+1, k+1);
                int p3 = pointIndex(i+1, j+1, k);
                fFaces << "4(" << p0 << " " << p1 << " " << p2 << " " << p3 << ")\n";
            }

    // Внутренние по Z
    for (int k = 0; k < nz - 1; ++k)
        for (int j = 0; j < ny; ++j)
            for (int i = 0; i < nx; ++i) {
                int p0 = pointIndex(i, j, k+1);
                int p1 = pointIndex(i+1, j, k+1);
                int p2 = pointIndex(i+1, j+1, k+1);
                int p3 = pointIndex(i, j+1, k+1);
                fFaces << "4(" << p0 << " " << p1 << " " << p2 << " " << p3 << ")\n";
            }

    fFaces << ")\n";
    fFaces.close();
    std::cout << "OK Грани записаны блоками.\n";

    // --- 3. Owner и Neighbour ---
    std::ofstream fOwner((meshDir + "/owner").c_str());
    std::ofstream fNeighbour((meshDir + "/neighbour").c_str());

    fOwner << nTotalFaces << "\n(\n";
    fNeighbour << nTotalFaces << "\n(\n";

    // Вспомогательные макросы/лямбды для записи
    auto writePair = [&](int o, int n) {
    fOwner << o << " ";
    fNeighbour << (n == -1 ? -1 : n) << " ";
    };

    // Граничные грани
    // Bottom
    for (int j = 0; j < ny; ++j)
        for (int i = 0; i < nx; ++i)
            writePair(cellIndex(i, j, 0), -1);

    // Top
    for (int j = 0; j < ny; ++j)
        for (int i = 0; i < nx; ++i)
            writePair(cellIndex(i, j, nz-1), -1);

    // Left
    for (int k = 0; k < nz; ++k)
        for (int i = 0; i < nx; ++i)
            writePair(cellIndex(i, 0, k), -1);

    // Right
    for (int k = 0; k < nz; ++k)
        for (int i = 0; i < nx; ++i)
            writePair(cellIndex(i, ny-1, k), -1);

    // Front
    for (int k = 0; k < nz; ++k)
        for (int j = 0; j < ny; ++j)
            writePair(cellIndex(0, j, k), -1);

    // Back
    for (int k = 0; k < nz; ++k)
        for (int j = 0; j < ny; ++j)
            writePair(cellIndex(nx-1, j, k), -1);

    // Внутренние
    // X
    for (int k = 0; k < nz; ++k)
        for (int j = 0; j < ny; ++j)
            for (int i = 0; i < nx - 1; ++i)
                writePair(cellIndex(i, j, k), cellIndex(i+1, j, k));

    // Y
    for (int k = 0; k < nz; ++k)
        for (int j = 0; j < ny - 1; ++j)
            for (int i = 0; i < nx; ++i)
                writePair(cellIndex(i, j, k), cellIndex(i, j+1, k));

    // Z
    for (int k = 0; k < nz - 1; ++k)
        for (int j = 0; j < ny; ++j)
            for (int i = 0; i < nx; ++i)
                writePair(cellIndex(i, j, k), cellIndex(i, j, k+1));

    fOwner << ")\n";
    fOwner.close();
    fNeighbour << ")\n";
    fNeighbour.close();
    std::cout << "OK Owner/Neighbour записаны.\n";

    // --- 4. Boundary ---
    std::ofstream fBoundary((meshDir + "/boundary").c_str());
    fBoundary << "6 (\n";

    auto writePatch = [&](const std::string &name, const std::string &type,
                          int startFace, int nFaces) {
        fBoundary << name << "\n{\n";
        fBoundary << "    type " << type << ";\n";
        fBoundary << "    nFaces " << nFaces << ";\n";
        fBoundary << "    startFace " << startFace << ";\n";
        fBoundary << "}\n";
    };

    writePatch("bottom", "wall", idxBottomStart, nBottom);
    writePatch("top", "wall", idxTopStart, nTop);
    writePatch("left", "wall", idxLeftStart, nLeft);
    writePatch("right", "wall", idxRightStart, nRight);
    writePatch("front", "wall", idxFrontStart, nFront);
    writePatch("back", "wall", idxBackStart, nBack);

    fBoundary << ")\n";
    fBoundary.close();
    std::cout << "OK Boundary записан.\n";


    //Вот только блок для alpha.water — полностью исправленный, без лишних зависимостей и с правильным расчётом уровня воды по координатам ячеек:

    // --- 5. alpha.water ---
    std::ofstream fAlpha((caseDir + "/0/alpha.water").c_str());
    fAlpha << "/*--------------------------------*- C++ -*----------------------------------*\\\n";
    fAlpha << " =========                 |\n";
    fAlpha << " \\\\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox\n";
    fAlpha << "  \\\\    /   O peration     | Version: dev\n";
    fAlpha << "   \\\\  /    A nd           | Website: www.openfoam.org\n";
    fAlpha << "    \\/     M anipulation  |\n";
    fAlpha << "*---------------------------------------------------------------------------*/\n";
    fAlpha << "FoamFile\n";
    fAlpha << "{\n";
    fAlpha << "    version     2.0;\n";
    fAlpha << "    format      ascii;\n";
    fAlpha << "    class       volScalarField;\n";
    fAlpha << "    object      alpha.water;\n";
    fAlpha << "}\n";
    fAlpha << "dimensions      [0 0 0 0 0 0 0];\n";
    fAlpha << "internalField   uniform 0;\n";
    fAlpha << "boundaryField\n";
    fAlpha << "{\n";
    fAlpha << "    defaultField\n";
    fAlpha << "    {\n";
    fAlpha << "        type            calculated;\n";
    fAlpha << "        value           uniform 0;\n";
    fAlpha << "    }\n";
    fAlpha << "}\n\n";

    fAlpha << "(\n";
    int count = 0;
    for (int k = 0; k < nz; ++k) {
        for (int j = 0; j < ny; ++j) {
            for (int i = 0; i < nx; ++i) {
                // Вычисляем центр ячейки
                double xc = (i + 0.5) * Lx / nx;
                double yc = (j + 0.5) * Ly / ny;
                double zc = (k + 0.5) * Lz / nz;

                // Если центр ниже уровня воды — ячейка заполнена водой (alpha = 1)
                double val = (zc < waterLevelZ) ? 1.0 : 0.0;

                fAlpha << val;
                ++count;
                if (count < nCells) {
                    fAlpha << " ";
                }
                if (count % 10 == 0) {
                    fAlpha << "\n";  // Для читаемости файла делаем переносы каждые 10 значений
                }
            }
        }
    }
    fAlpha << "\n)\n";
    fAlpha << "\n// ************************************************************************* //\n";
    fAlpha.close();
    std::cout << "OK alpha.water записан.\n";
}
/* if 0
---
Что тут важно:
- Нет fCenters: центр ячейки считается напрямую через индексы i, j, k и размеры домена — это надёжно и не требует дополнительных файлов.
- Порядок вывода: строго соответствует нумерации ячеек в OpenFOAM (i меняется быстрее всего, потом j, потом k) — так alpha.water совпадёт с сеткой.
- Читаемость: добавлены переносы строк каждые 10 чисел — файл будет аккуратным, но полностью валидным для OpenFOAM.

Если скажешь, что дальше (например, как скомпилировать или запустить checkMesh), подскажу следующий шаг.😊

// --- 5. alpha.water ---
//std::ofstream fAlpha((caseDir + "/0/alpha.water").c_str());
//fAlpha << "/*--------------------------------*- C++ -*----------------------------------*\\\n";
//fAlpha << " ========= |\n";
---

Продолжение для alpha.water (то, что не влезло)

// --- 5. alpha.water ---
    std::ofstream fAlpha((caseDir + "/0/alpha.water").c_str());
    fAlpha << "/*--------------------------------*- C++ -*----------------------------------*\\\n";
    fAlpha << "  =========                 |\n";
    fAlpha << "  \\\\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox\n";
    fAlpha << "   \\\\    /   O peration     | Version:  dev\n";
    fAlpha << "    \\\\  /    A nd           | Website:  www.openfoam.org\n";
    fAlpha << "     \\/     M anipulation  |\n";
    fAlpha << "*---------------------------------------------------------------------------* /\n";
    fAlpha << "FoamFile\n";
    fAlpha << "{\n";
    fAlpha << "    version     2.0;\n";
    fAlpha << "    format      ascii;\n";
    fAlpha << "    class       volScalarField;\n";
    fAlpha << "    object      alpha.water;\n";
    fAlpha << "}\n";
    //fAlpha << "// * * * * * * * * * * * *
    //
    //
    //    fAlpha << "FoamFile\n{\n";
    //    fAlpha << "    version     2.0;\n";
//    fAlpha << "    format      ascii;\n";
//    fAlpha << "    class       volScalarField;\n";
//    fAlpha << "    object      alpha.water;\n";
//    fAlpha << "}\n";
    fAlpha << "dimensions      [0 0 0 0 0 0 0];\n";
    fAlpha << "internalField   uniform 0;\n";
    fAlpha << "boundaryField\n{\n";
    fAlpha << "    defaultField\n    {\n";
    fAlpha << "        type            calculated;\n";
    fAlpha << "        value           uniform 0;\n";
    fAlpha << "    }\n";
    fAlpha << "}\n";                           // Записываем значения для ячеек
    fAlpha << "\n(";
    for (int i = 0; i < nCells; ++i) {
      Vec3 c;
      fCenters >> c.x >> c.y >> c.z;
      double val = (c.z < waterLevelZ) ? 1.0 : 0.0;
      fAlpha << val << (i == nCells - 1 ? "" : " ");
    }
    fAlpha << ")\n";

    fAlpha << "\n// ************************************************************************* //\n";
    fAlpha.close();

    std::cout << "Готово! Файл alpha.water создан в " << caseDir << "/0/\n";
    return 0;
}

На основе:
[andreyex.ru](https://andreyex.ru/stati-partnerov/inzheneriya/modelirovanie-vetrovyh-nagruzok-na-arochnye-shatry-v-linux-polnoe-rukovodstvo-po-cfd-analizu-tentovyh-konstruktsij/)
[sveden.utmn.ru](https://sveden.utmn.ru/sveden/files/Ganopolyskiy_Gilymanov_2022_Uchebnoe_posobie.pdf)
[www.cyberforum.ru](https://www.cyberforum.ru/unity/thread2030918.html)

//
//Или, если хочешь совсем просто и без лишних функций, используй стандартную библиотеку C++17 (она есть в современном MinGW):

//#include <filesystem>
//namespace fs = std::filesystem;

// В main вместо makeDir:
fs::create_directories(caseDir + "/constant/polyMesh");
fs::create_directories(caseDir + "/0");

Это самый чистый вариант. Убедись только, что при компиляции указан стандарт C++17:
g++ AlfaGrid.cpp -o AlfaGrid -std=c++17

Вариант 2: Ручное создание папок

Просто создай вручную в проводнике или командной строке:
mkdir case_out\constant\polyMesh
mkdir case_out\0

//После этого запусти свою программу — файлы появятся. Но лучше поправить код, чтобы не делать это каждый раз.

---

Важный нюанс про WSL и OpenFOAM

Ты хочешь потом открыть этот кейс в WSL-OpenFOAM. Тут есть критический момент:

1. Пути: Если ты создаёшь файлы прямо на диске C (C:\Users\...\case_out),
   то из WSL ты увидишь их по пути /mnt/c/Users/.../case_out.
   Это работает, но медленно.
   OpenFOAM в WSL будет очень долго читать/писать файлы,
   лежащие на NTFS-диске через /mnt/c.
2. Переносы строк: Windows использует \r\n, Linux — \n.
   OpenFOAM обычно прощает это, но лучше генерировать файлы сразу
   в Linux-стиле (наш код уже так делает, \n — это правильно).
3. Лучшая практика: Генерируй кейс внутри WSL.

Как сделать идеально:
1. Скопируй свой .cpp файл внутрь WSL (например, в ~/projects/AlfaGrid).
2. Скомпилируй там: g++ AlfaGrid.cpp -o alfa -std=c++17.
3. Запусти: ./alfa.
4. Папка case_out создастся нативном Linux-диске (ext4),
   и OpenFOAM будет работать с ней максимально быстро.

Если очень хочется компилировать в Windows, а запускать в WSL
— убедись, что папка case_out лежит внутри домашнего каталога WSL
 (например, /home/user/case_out), а не на /mnt/c/....

---

Финальная проверка твоего кода

Убедись, что в конце функции main у тебя есть return 0;. И добавь проверку на открытие файла alpha.water, чтобы видеть ошибку, если папка всё-таки не создалась:

// Перед записью alpha.water
if (!fAlpha.is_open()) { std::cerr << "ОШИБКА: Не удалось открыть файл для записи: "
#endif
*/
