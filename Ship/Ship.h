
#include <StdIO.h>
#include "..\Window\ConIO.h"
#include "..\Window\Window.h"

typedef enum { fv100,fv110,fv120,fv130,fv140, fv150,fv160,fv165,fv170,fv180,
               fv190,fv191,fv195,fv198,fv200, fv201,fv210,fv220,fv230,fv240,
               fv250,fv261,fv270,fv280,fv290, fv295,fv296,fv297,fv298,fv300,
               fv302,fv303,fv305,fv309,fv310, fv313,fv314,fv317,fv327,fv332,
               fv335,fv421,fv430,fv462,fv500 } FileVersion;       // всего = 45
typedef enum { fvBodyplan, fvProfile, fvPlan, fvPerspective } ViewType;
typedef enum { fpLow,fpMedium,fpHigh,fpVeryHigh } PrecisionType;  // ship-model
typedef enum { mvPort,mvBoth } BoardView;  // Show half the hull or entire hull
typedef enum { fuMetric,fuImperial } UnitType;  // Switch metric^imperial units
typedef enum { fcProjectSettings,fcActualData } HydrostaticCoefficient;
typedef enum { svRegular, svCrease, svDart, svCorner } VertexType; // Different types of subdivisionvertices
typedef enum { fiFree,fiStation,fiButtock,fiWaterline,fiDiagonal } IntersectionType;
     // types of intersection lines, stations, buttocks, waterlines and lines orientated in random planes

struct Point { Real x,y,z; };
struct Plane { Real a,b,c,d; };  // Description of a 3D plane: a*x + b*y + c*z -d = 0.0;

struct Project
{//  FreeShip: TFreeShip;
bool MainparticularsHasBeenset, // Flag to check if the main particulars have been set before hydrostatic calculationss are being performed
     DisableModelCheck,         // Disable the automatic checking of the surface
     EnableModelAutoMove,       // Unable the automatic moving model along Z
     EnableBonjeanSAC;          // Unable calculation and save in file Bonjean scale and SAC
Real AppendageCoefficient,
     Beam,
     Draft,
     Length;
byte UnderWaterColorAlpha;
Real WaterDensity,
//   WaterTemper,
     SplitSectionLocation;
bool UseDefaultSplitSectionLocation; // If set to true, the midship/mainframe location is set to 0.5*project length, if false then value in FProjectMainframeLocation is used
char *Name,
     *Designer,
     *Comment,
     *CreatedBy;
bool ShadeUnderwaterShip,
     SavePreview;
unsigned int UnderWaterColor;
UnitType     Units;
PrecisionType Precision;
bool SimplifyIntersections;
HydrostaticCoefficient
     HydrostaticCoefficients;      // General hydrostatics calculation settings
Real StartDraft,
     EndDraft,
     DraftStep,
     Trim;                                   // crosscurves settings
Real* Displacements;
int NoDisplacements;
Real MinimumDisplacement,
     MaximumDisplacement,
     DisplIncrement;
bool UseDisplIncrements;
int NoAngles;
Real* Angles;
int NoTrims;
Real* Trims;
bool FreeTrim;
Real FVCG;
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
BoardView
     ModelView;     // Show half or entire ship
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
     Flowlines;
Real CurvatureScale, // Scalefactor used to increase or decrease the size of the curvature plot
     CursorIncrement;
};
struct Surface
{ bool isLoad;
  int  NoLayers,NoCoPoint,NoEdges,NoCurves,NoFaces;
  struct Laeyrs
  { char *Description;
    int ID;
    unsigned int Color;
    bool Visible,
         Symmetric,
         Developable,
         UseforIntersection, // fc>=180
         UswinHydrostatic;
    Real MaterialDensity,    // fv>=191
         Thickness;
    bool ShoeinLineSpan;     // fv>=201
    byte AlphaBlend;         // fv>=261
  } *L, ActiveLayer;
  struct CoPoint:Point       /// Control Point
  { VertexType T;
    bool Selected,Locked;
  } *P;
  struct Edges               /// Control Edges
  { int StartIndex,EndIndex;  // --> Points
//  Point StartPoint,EndPoint;
    bool Crease,Selected;
  } *G;
  struct Curves              /// Control Curves FV >= 195
  { int *P,Capacity;          // --> Points
    bool Selected;
  } *C;
  struct Faces               /// Control Faces
  { int *P,Capacity;          // --> Points
    int LayerIndex;
    bool Selected;

  } *F;

  Surface(){ memset( this,0,sizeof( Surface ) ); }
};

struct InterSection:Plane
{ IntersectionType IT;
  bool ShowCurvature,
       Build;
  int  NoItems;
  struct Items
  { int NoSplines;
    struct Spline
    { Point P; bool Knuckle;
    } *S;
  } *T;
  void Read();
};
struct Marker
{ bool Visible,
       Selected,
       ShowCurvature;
  Real CurvatureScale;
  int NoSplines;
  struct Spline
  { Point P; bool Knuckle;
  } *S;
};


struct Ship
{ PrecisionType PT;                     // Precision of the ship-model
  Visibility  Show;                     // настройка графической визуализации
  Project      Set;                     // характеристики и размерности корабля
  Surface      Shl;                     // оболочка поверхности обшивки корпуса

  int NoStations,NoButtocks,NoWaterlines,NoDiagonals,NoMarkers;
  InterSection *Stations,       ///  LoadStation
               *Buttocks,       ///  LoadButtocks
               *Waterlines,     ///  LoadWaterlines
               *Diagonals;      ///  LoadDiagonals
  Marker       *Marks;

  Ship();

  bool ReadFile( char *FileName );

};
