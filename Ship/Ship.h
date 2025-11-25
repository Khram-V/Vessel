
#include <StdIO.h>
#include "..\Window\ConIO.h"
#include "..\Storm\Flex.h"
#include "..\Type.h"

typedef enum { fv100,fv110,fv120,fv130,fv140, fv150,fv160,fv165,fv170,fv180,
               fv190,fv191,fv195,fv198,fv200, fv201,fv210,fv220,fv230,fv240,
               fv250,fv261,fv270,fv280,fv290, fv295,fv296,fv297,fv298,fv300,
               fv302,fv303,fv305,fv309,fv310, fv313,fv314,fv317,fv327,fv332,
               fv335,fv421,fv430,fv462,fv500, fv510 } FileVersion; // всего=46
typedef enum { fvBodyplan, fvProfile, fvPlan, fvPerspective } ViewType;
typedef enum { fpLow,fpMedium,fpHigh,fpVeryHigh } PrecisionType; // ship-model
typedef enum { mvPort,mvBoth } BoardView; // Show half the hull or entire hull
typedef enum { fuMetric,fuImperial } UnitType; // Switch metric^imperial units
typedef enum { fcProjectSettings,fcActualData } HydrostaticCoefficient; // ??
typedef enum { svRegular, svCrease, svDart, svCorner } VertexType;
     // Different types of sub-division-vertices
typedef enum { fiFree,fiStation,fiButtock,fiWaterline,fiDiagonal } IntersectionType;
     // intersection lines: free,stations,buttocks,waterlines and diagonal type
struct Plane { Real a,b,c,d; }; // Description of 3D plane: a*x+b*y+c*z-d=0.0;
union Color{ unsigned C; byte c[4]; };

extern Vector Min,Max; // Экстремумы исходного графического изображения
extern byte  UnderWaterColorAlpha;
extern Color UnderWaterColor;
extern Real  Beam,     // ширина
             Draft,    // осадка
             Length;   // длина
struct Project{        // четыре строки описание корабля
char *Name,            // название проекта
     *Designer,        // автор проекта
     *Comment,         // расширенное описание
     *CreatedBy;       // изготовитель цифровой модели
bool MainparticularsHasBeenset, // Flag to check if the main particulars have been set before hydrostatic calculationss are being performed
                          // Флаг, позволяющий проверить, были ли заданы основные параметры перед выполнением гидростатических расчетов.
     DisableModelCheck,   // Disable the automatic checking of the surface
                          // Отключить автоматическую проверку поверхности
     EnableModelAutoMove, // Unable the automatic moving model along Z
                          // Невозможно автоматическое перемещение модели по оси Z
     EnableBonjeanSAC;    // Unable calculation and save in file Bonjean scale and SAC
                          // Невозможно рассчитать и сохранить в файле шкалу Бонжана и SAC
Real AppendageCoefficient; // коэффициент остаточного сопротивления: волн и пр.
Real WaterDensity,         // плотность воды
//   WaterTemper,          // и температура
     SplitSectionLocation; // положение мидельшпангоута
bool UseDefaultSplitSectionLocation; // If set to true, the midship/mainframe location is set to 0.5*project length, if false then value in FProjectMainframeLocation is used
     // Если установлено значение true, местоположение мидель/главного шпангоута
     // устанавливается равным 0,5*длина проекта, если false,
     // то используется значение в FProjectMainframeLocation.
bool ShadeUnderwaterShip,
     SavePreview;
UnitType Units;
PrecisionType Precision;
bool SimplifyIntersections;

HydrostaticCoefficient
     HydrostaticCoefficients; // General hydrostatics calculation settings
Real StartDraft,EndDraft,DraftStep, // cross-curves settings
     Trim;
 int NoDisplacements;
Real MinimumDisplacement,
     MaximumDisplacement,
     DisplIncrement;
Real* Displacements;
bool UseDisplIncrements;
 int NoAngles; Real* Angles;
 int NoTrims;  Real* Trims;
bool FreeTrim;
Real FVCG;          // Vertical Centre of Gravity (m above B.L.)
                    // аппликата центра тяжести
};
struct Visibility
{//  FFreeShip: TFreeShip;
bool ControlNet,
     Objects,
     InteriorEdges, // Show the surface edges
     Stations,      // Show the calculated stations
     Buttocks,      // Show the calculated Buttocks
     Waterlines,    // Show the calculated Waterlines
     Diagonals;     // Show the calculated Diagonals
BoardView ModelView;// Show half or entire ship
bool Normals,       // Show normals of selected surface patches
     Grid,          // Show the grid of intersections in the plan,profile and bodyplan view
     BothSides,     // show both sides
     Markers,
     ControlCurves,
     Curvature,
     HydrostaticData,
     HydrostDisplacement,
     HydrostLateralArea,
     HydrostSectionalAreas,
     HydrostMetacentricHeight,
     HydrostLCF,
     lFlowline;
Real CurvatureScale, // Scalefactor used to increase or decrease the size of the curvature plot
     CursorIncrement;
};
//struct Layers
//{ char *Description;
//    int ID;
//    unsigned int Color;
//    bool Visible,
//         Symmetric,
//         Developable,
//         UseforIntersection, // fc>=180
//         UswinHydrostatic;
//    Real MaterialDensity,    // fv>=191
//         Thickness;
//    bool ShoeinLineSpan;     // fv>=201
//    byte AlphaBlend;         // fv>=261
//};
class Surface
{ bool isLoad;              //-- признак успешной загрузки
  int  NoLayers,NoCoPoint,NoEdges,NoCurves,NoFaces; // размеры кривых  массивов
  struct Layers
  { char *Description;
    int ID;
//  unsigned int Color;
    Color LClr;
    bool Visible,
         Symmetric,
         Developable,
         UseforIntersection, // fc>=180
         UswinHydrostatic;
    Real MaterialDensity,    // fv>=191
         Thickness;
    bool ShowInLineSpan;     // fv>=201
    byte AlphaBlend;         // fv>=261
  } *L, ActiveLayer;         //! NoLayers

  struct CoPoint             /// Control Point
  { Vector V;                // координаты
    VertexType T;            // тип узла: svRegular, svCrease, svDart, svCorner
    bool Selected,           // -- выбор
         Locked;             // -- блокировка >=fv198
  } *P;                      //! NoCoPoint

  struct Edges               /// Control Edges
  { int StartIndex,EndIndex; // --> Points индексы
    bool Crease,             // -- ребро по сломанной поверхности
         Selected,           // -- выбор
         ControlEdge;        // == True на считывании ребра
    Vector StartPoint,EndPoint; //~~ излишнее переприсваивание -- безобидно
  } *G;                      //! NoEdges

  struct Curves              /// Control Curves FV >= 195
  { int *P,Capacity;         // --> Points индексные ссылки
    bool Selected;           // дополнительно: Edge[N-1] + Points[N] !!! грубо
  } *C;                      //! NoCurves

  struct Faces               /// Control Faces
  { int *P,Capacity;         // --> Points
    int LayerIndex;          // -- слой для всего набора площадок
    bool Selected;
  } *F;                      //! NoFaces

  void Extents();            // экстремумы по расчетным площадкам
public:
  Surface(){ memset( this,0,sizeof( Surface ) ); }
  void Read();
  void ReadFEF();
  void Drawing( BoardView=mvPort );
};
struct Spline{ Vector P; bool Knuckle; };
struct Items{ int NoSplines; Spline *S; };

struct InterSection //: Plane
{ IntersectionType IT;
  bool ShowCurvature,Build;
  Plane Pl;
  int NIt,NPt; Items *T;
  InterSection(){ memset( this,0,sizeof( InterSection ) ); }
  void Read();
  void ReConnect();              // перенастройка по первому фрагменту образцу
//void ReStation();              // упорядочение шпангоута по нижней точке в ДП
 Vector *ReButtocks( int &Id, int &Iu ); // штевни перестроенные в контур
  void Drawing( BoardView=mvPort );
};
struct Marker
{ bool Visible,Selected,ShowCurvature;
  Real CurvatureScale;
  int NoSplines; Spline *S;
};
struct Ship                      // Сборка корпуса в целом
{ WCHAR *FName; char *Name;      // Единожды представляемое имя числовой модели
  PrecisionType PT;              // Precision of the ship-model
  Visibility Visio;              // Show настройка графической визуализации
  Project      Set;              // характеристики и размерности корабля
  Surface    Shell;              // Shell оболочка поверхности обшивки корпуса

  int NoStations,NoButtocks,NoWaterlines,NoDiagonals,NoMarkers,NoFlowLines;
  InterSection *Stations,        // шпангоуты  LoadStation
               *Buttocks,        // батоксы    LoadButtocks
               *Waterlines,      // ватерлинии LoadWaterlines
               *Diagonals;       // рыбины     LoadDiagonals
  Marker *Marks;
  Flex* FlowLines;
  bool YesShip;                  // ... и это здесь не особо-то необходимо ?
  Ship();                        // очищающий конструктор
  bool LoadProject();            // быстрая выборка всего комплекса данных
                                 // в общие структуры в оперативной памяти
  bool LoadFEF();                // здесь Ship.fef == File Exchange Format
  bool LoadPart();               // или просто фрагмент цифровой модели
  void WriteVSL();               // запись теоретических контуров k Hull+Aurora
};
struct FreeShip: Ship,View       // , Matrix
{ FreeShip();
  virtual bool Draw();           // виртуальная процедура с настройкой сцены
  virtual bool KeyBoard( fixed );
};
