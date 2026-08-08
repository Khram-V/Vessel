/*
Понял задачу! Вы хотите именно C++ программу, 
которая генерирует файлы граничных условий OpenFOAM, 
а не сами конфигурационные файлы. 
Вот пример такой программы:
*/
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>

// Класс для генерации файлов OpenFOAM
class OpenFOAMCaseGenerator {
private:
    double lengthX = 2000.0;   // Длина бассейна (м)
    double widthY = 40.0;      // Ширина бассейна (м)  
    double heightZ = 20.0;     // Высота бассейна (м)
    double waterLevel = 15.0;  // Уровень воды (м)
    double inletVelocity = 0.5;// Скорость на входе (м/с)
    
public:
    void generateBlockMeshDict() {
        std::ofstream file("blockMeshDict");
        
        file << "/*--------------------------------*- C++ -*----------------------------------*\\\n";
        file << "| =========                 |                                                 |\n";
        file << "| \\\\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox           |\n";
        file << "|  \\\\    /   O peration     | Version:  v2312                                 |\n";
        file << "|   \\\\  /    A nd           | Website:  www.openfoam.com                      |\n";
        file << "|    \\\\/     M anipulation  |                                                 |\n";
        file << "\\\\*---------------------------------------------------------------------------*/\n\n";
        
        file << "FoamFile\n{\n";
        file << "    version     2.0;\n";
        file << "    format      ascii;\n";
        file << "    class       dictionary;\n";
        file << "    object      blockMeshDict;\n";
        file << "}\n\n";
        
        file << "convertToMeters 1;\n\n";
        
        file << "vertices\n(\n";
        file << "    (0 0 0)\n";
        file << "    (" << lengthX << " 0 0)\n";
        file << "    (" << lengthX << " " << widthY << " 0)\n";
        file << "    (0 " << widthY << " 0)\n";
        file << "    (0 0 " << heightZ << ")\n";
        file << "    (" << lengthX << " 0 " << heightZ << ")\n";
        file << "    (" << lengthX << " " << widthY << " " << heightZ << ")\n";
        file << "    (0 " << widthY << " " << heightZ << ")\n";
        file << ");\n\n";
        
        int nx = static_cast<int>(lengthX / 2.5);
        int ny = static_cast<int>(widthY / 2.5);
        int nz = static_cast<int>(heightZ / 0.25);
        
        file << "blocks\n(\n";
        file << "    hex (0 1 2 3 4 5 6 7) (" << nx << " " << ny << " " << nz << ") simpleGrading (1 1 1)\n";
        file << ");\n\n";
        
        file << "boundary\n(\n";
        file << "    inlet { type patch; faces ((0 3 7 4)); }\n";
        file << "    outlet { type patch; faces ((1 2 6 5)); }\n";
        file << "    bottom { type wall; faces ((0 1 5 4)); }\n";
        file << "    topAtm { type symmetryPlane; faces ((3 2 6 7)); }\n";
        file << "    sides { type wall; faces ((0 1 2 3) (4 5 6 7)); }\n";
        file << ");\n";
        
        file.close();
        std::cout << "Файл blockMeshDict сгенерирован успешно!\n";
    }
    
    void generateUField() {
        std::ofstream file("0/U");
        
        file << "FoamFile\n{\n";
        file << "    version     2.0;\n";
        file << "    format      ascii;\n";
        file << "    class       volVectorField;\n";
        file << "    location    \"0\";\n";
        file << "    object      U;\n";
        file << "}\n\n";
        
        file << "dimensions      [0 1 -1 0 0 0 0];\n\n";
        file << "internalField   uniform (" << inletVelocity << " 0 0);\n\n";
        
        file << "boundaryField\n{\n";
        file << "    inlet { type fixedValue; value uniform (" << inletVelocity << " 0 0); }\n";
        file << "    outlet { type zeroGradient; }\n";
        file << "    bottom { type noSlip; }\n";
        file << "    topAtm { type symmetryPlane; }\n";
        file << "    sides { type noSlip; }\n";
        file << "}\n";
        
        file.close();
        std::cout << "Файл 0/U сгенерирован успешно!\n";
    }
    
    void generateP_rghField() {
        std::ofstream file("0/p_rgh");
        
        file << "FoamFile\n{\n";
        file << "    version     2.0;\n";
        file << "    format      ascii;\n";
        file << "    class       volScalarField;\n";
        file << "    location    \"0\";\n";
        file << "    object      p_rgh;\n";
        file << "}\n\n";
        
        file << "dimensions      [1 -1 -2 0 0 0 0];\n\n";
        file << "internalField   uniform 0;\n\n";
        
        file << "boundaryField\n{\n";
        file << "    inlet { type fixedFluxPressure; value uniform 0; }\n";
        file << "    outlet { type fixedValue; value uniform 0; }\n";
        file << "    bottom { type fixedFluxPressure; value uniform 0; }\n";
        file << "    topAtm { type totalPressure; p0 uniform 0; U U; phi phi; rho rho; psi none; gamma 1; value uniform 0; }\n";
        file << "    sides { type fixedFluxPressure; value uniform 0; }\n";
        file << "}\n";
        
        file.close();
        std::cout << "Файл 0/p_rgh сгенерирован успешно!\n";
    }
    
    void generateAlphaWater() {
        std::ofstream file("0/alpha.water");
        
        file << "FoamFile\n{\n";
        file << "    version     2.0;\n";
        file << "    format      ascii;\n";
        file << "    class       volScalarField;\n";
        file << "    location    \"0\";\n";
        file << "    object      alpha.water;\n";
        file << "}\n\n";
        
        file << "dimensions      [0 0 0 0 0 0 0];\n\n";
        file << "internalField   uniform 0;\n\n";
        
        file << "boundaryField\n{\n";
        file << "    inlet { type variableHeightFlowRateInletVelocity; lowerBound 0; upperBound 1; value uniform 1; }\n";
        file << "    outlet { type zeroGradient; }\n";
        file << "    bottom { type zeroGradient; }\n";
        file << "    topAtm { type inletOutlet; inletValue uniform 0; value uniform 0; }\n";
        file << "    sides { type zeroGradient; }\n";
        file << "}\n";
        
        file.close();
        std::cout << "Файл 0/alpha.water сгенерирован успешно!\n";
    }
    
    void createDirectoryStructure() {
        system("mkdir -p 0 constant system");
        std::cout << "Структура директорий создана.\n";
    }
};

int main() {
    OpenFOAMCaseGenerator generator;
    
    std::cout << "Генерация кейса OpenFOAM для бассейна...\n";
    std::cout << "Размеры: " << generator.lengthX << " x " << generator.widthY 
              << " x " << generator.heightZ << " м\n";
    std::cout << "Уровень воды: " << generator.waterLevel << " м\n";
    
    generator.createDirectoryStructure();
    generator.generateBlockMeshDict();
    generator.generateUField();
    generator.generateP_rghField();
    generator.generateAlphaWater();
    
    std::cout << "\n✅ Генерация завершена!\n";
    std::cout << "📁 Проверьте созданную структуру каталогов и отредактируйте:\n";
    std::cout << "   - constant/transportProperties (свойства жидкости)\n";
    std::cout << "   - controlDict (параметры времени)\n";
    std::cout << "   - fvSchemes/fvSolution (численные схемы)\n";
    std::cout << "\n💧 Для заливки воды до уровня " << generator.waterLevel 
              << " м используйте setFields после mesh generation.\n";
    
    return 0;
}
/*
Особенности этой программы

Что она делает:

Генерирует все основные файлы конфигурации OpenFOAM программно
Использует параметры как переменные класса (легко менять геометрию)
Создает правильную структуру каталогов
Выводит понятные сообщения о ходе работы

Как компилировать и запускать:

g++ -std=c++11 -o generate_openfoam_case generate_openfoam_case.cpp
./generate_openfoam_case

После запуска вы получите:

case/
├── 0/
│   ├── U
│   ├── p_rgh
│   └── alpha.water
├── constant/
├── system/
└── blockMeshDict

Ключевые преимущества такого подхода:

Легко изменять параметры через код
Можно добавить циклы для параметрических исследований
Интеграция с другими инструментами автоматизации
Отсутствие ошибок при ручном копировании шаблонов

Хотите, я добавлю в эту программу автоматическую генерацию setFieldsDict для правильной заливки воды?

Для ответа использовал актуальные интернет-источники:

habr.com: OpenFOAM с точки зрения программиста-физика / Хабр
ru.wikipedia.org: CalculiX 1️⃣
ru.wikipedia.org: OpenFOAM 1️⃣

1️⃣ РКН: иностранный владелец ресурса нарушает закон&nbsp;РФ
*/