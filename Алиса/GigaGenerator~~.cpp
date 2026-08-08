/*
Вот пример кода на C++ для OpenFOAM, который задает граничные условия и параметры сетки для прямоугольного бассейна с проточной водой. В коде подробно прокомментированы все ключевые моменты.


1. blockMeshDict — описание геометрии расчетной области

/*--------------------------------*- C++ -*----------------------------------*\
| =========                 |                                                 |
| \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox           |
|  \\    /   O peration     | Version:  v2312                                 |
|   \\  /    A nd           | Website:  www.openfoam.com                      |
|    \\/     M anipulation  |                                                 |
\----------------------------------------------------------------------------*/
FoamFile
{
    version     2.0;
    format      ascii;
    class       dictionary;
    object      blockMeshDict;
}
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

convertToMeters 1; // Размеры в метрах

vertices
(
    (0 0 0)          // 0 - нижний левый передний угол
    (2000 0 0)       // 1 - правый по X
    (2000 40 0)      // 2 - дальний по Y
    (0 40 0)         // 3 - ближний по Y
    (0 0 20)         // 4 - верхний Z=20м
    (2000 0 20)      // 5
    (2000 40 20)     // 6
    (0 40 20)        // 7
);

blocks
(
    hex (0 1 2 3 4 5 6 7) (800 16 80) simpleGrading (1 1 1)
    // Дискретизация:
    // По длине (X): 800 ячеек (~2.5 м каждая)
    // По ширине (Y): 16 ячеек (по 2.5 м)
    // По высоте (Z): 80 ячеек (по 0.25 м)
);

edges
(
);

boundary
(
    inlet
    {
        type patch;
        faces
        (
            (0 3 7 4)
        );
    }
    outlet
    {
        type patch;
        faces
        (
            (1 2 6 5)
        );
    }
    bottom
    {
        type wall;
        faces
        (
            (0 1 5 4)
        );
    }
    topAtm
    {
        type symmetryPlane; // Симметрия вместо атмосферы
        faces
        (
            (3 2 6 7)
        );
    }
    sides
    {
        type wall;
        faces
        (
            (0 1 2 3)
            (4 5 6 7)
        );
    }
);

mergePatchPairs
(
);

/* ************************************************************************ 

Пояснения:  

Бассейн длиной 2 км ( $2000$ м), шириной $40$ м, высотой $20$ м.
Верхняя граница задана как симметричная плоскость (symmetryPlane) — имитирует открытую поверхность без волн.
Вход (inlet) и выход (outlet) реализованы как патчи для задания притока/оттока воды.
Сетка достаточно подробная для инженерных расчетов.


2. U — файл скорости (пример boundary conditions)

/*--------------------------------*- C++ -*----------------------------------*\
| =========                 |                                                 |
| \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox           |
|  \\    /   O peration     | Version:  v2312                                 |
|   \\  /    A nd           | Website:  www.openfoam.com                      |
|    \\/     M anipulation  |                                                 |
\----------------------------------------------------------------------------*/
FoamFile
{
    version     2.0;
    format      ascii;
    class       volVectorField;
    location    "0";
    object      U;
}
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

dimensions      [0 1 -1 0 0 0 0]; // м/с

internalField   uniform (0.5 0 0); // Начальная скорость потока, например, 0.5 м/с вдоль X

boundaryField
{
    inlet
    {
        type fixedValue;
        value uniform (0.5 0 0); // Постоянный приток воды со скоростью 0.5 м/с
    }
    outlet
    {
        type zeroGradient; // Свободный отток
    }
    bottom
    {
        type noSlip; // Прилипание к дну
    }
    topAtm
    {
        type symmetryPlane; // Симметрия сверху
    }
    sides
    {
        type noSlip; // Прилипание к боковым стенкам
    }
}

/* ************************************************************************

Пояснения:  

На входе фиксированная скорость течения.
На выходе свободный градиент (вода уходит свободно).
На дне и боковых стенках прилипание (noSlip).
Сверху — симметрия (нет обмена импульсом через свободную поверхность).


3. p_rgh — давление (для несжимаемой жидкости)

/*--------------------------------*- C++ -*----------------------------------*\
| =========                 |                                                 |
| \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox           |
|  \\    /   O peration     | Version:  v2312                                 |
|   \\  /    A nd           | Website:  www.openfoam.com                      |
|    \\/     M anipulation  |                                                 |
\----------------------------------------------------------------------------*/
FoamFile
{
    version     2.0;
    format      ascii;
    class       volScalarField;
    location    "0";
    object      p_rgh;
}
// * * * * * * * * * * * * * * * * * * * * blockMeshDict * * * * * * * * * * * * * * //

dimensions      [1 -1 -2 0 0 0 0]; // Па

internalField   uniform 0;

boundaryField
{
    inlet
    {
        type fixedFluxPressure;
        value uniform 0;
    }
    outlet
    {
        type fixedValue;
        value uniform 0; // Давление на выходе атмосферное (отсчитывается от гидростатики)
    }
    bottom
    {
        type fixedFluxPressure;
        value uniform 0;
    }
    topAtm
    {
        type totalPressure;
        p0 uniform 0;
        U U;
        phi phi;
        rho rho;
        psi none;
        gamma 1;
        value uniform 0;
    }
    sides
    {
        type fixedFluxPressure;
        value uniform 0;
    }
}

/* ************************************************************************ //

Пояснения:  

Используется переменная $p_{rgh}$ (давление минус гидростатика).
На выходе фиксируется нулевое избыточное давление.
На остальных границах применяется условие постоянного потока или давления.

4. alpha.water — объемная доля воды (VOF-модель свободной поверхности)

/*--------------------------------*- C++ -*----------------------------------*\
| =========                 |                                                 |
| \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox           |
|  \\    /   O peration     | Version:  v2312                                 |
|   \\  /    A nd           | Website:  www.openfoam.com                      |
|    \\/     M anipulation  |                                                 |
\----------------------------------------------------------------------------*/
FoamFile
{
    version     2.0;
    format      ascii;
    class       volScalarField;
    location    "0";
    object      alpha.water;
}
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

dimensions      [0 0 0 0 0 0 0];

internalField   uniform 0; // Изначально воздух

boundaryField
{
    inlet
    {
        type variableHeightFlowRateInletVelocity;
        lowerBound 0;
        upperBound 1;
        value uniform 1; // Вода поступает только до уровня 15м (см. setFields ниже)
    }
    outlet
    {
        type zeroGradient;
    }
    bottom
    {
        type zeroGradient;
    }
    topAtm
    {
        type inletOutlet;
        inletValue uniform 0;
        value uniform 0;
    }
    sides
    {
        type zeroGradient;
    }
}

/* ************************************************************************

Пояснения:  

$\alpha = 1$ — вода, $\alpha = 0$ — воздух.
Для корректного начального распределения используйте утилиту setFields, 
чтобы задать воду только до высоты $15$ м.


Кратко о запуске расчета

Создайте сетку командой blockMesh.
Задайте распределение фаз (setFields) — вода до $15$ м, выше — воздух.
Запустите решатель, например, interFoam для двухфазных течений.

Все файлы должны быть размещены в директории case вашего проекта OpenFOAM. 
Названия файлов и структура папок строго регламентированы документацией OpenFOAM.

Если потребуется шаблон файла setFieldsDict для заливки воды до $15$ м — могу привести его отдельно.[1]

Для ответа использовал актуальные интернет-источники:

habr.com: OpenFOAM на практике / Хабр
kpfu.ru: А.н. нуриев, о.н. зайцева
sveden.utmn.ru: Документ подписан простой электронной подписью
www.researchgate.net: (PDF) Моделирование гидродинамических процессов...
www.researchgate.net: (PDF) Введение в компьютерное моделирование...
*/