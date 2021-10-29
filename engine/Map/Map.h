////////////////////////////////////////////////////////////////////////////////
// Filename:	Map.h
// Class:		Map
//
// Description: Stores all data relevant to the map and provides member
//				functions for adding, removing and retrieving
// information about 				meta-rooms and rooms. It also
// provides
// for line of sight 				testing; agent/creature
// collision testing; agent/creature location testing; agent/creature physics;
// serialization. (See the Map.cpp file for function headers)
//
// Author:		Robert Dickson
////////////////////////////////////////////////////////////////////////////////
#ifndef MAP_H
#define MAP_H

#ifdef _MSC_VER
#pragma warning(disable : 4786 4503)
#endif

#include <set>
#include <algorithm>
#include <list>
#include <string>
#include <float.h>
#include "../PersistentObject.h"
#include "CARates.h"
#include "../Agents/AgentHandle.h"

//
// Public constants
//
const int CA_PROPERTY_COUNT = 20;
const int ROOM_TYPE_COUNT = 16;
const int IMPERMIABLE = 0;
const int PERMIABLE = 100;
const int DIRECTION_LEFT = 0;
const int DIRECTION_RIGHT = 1;
const int DIRECTION_CEILING = 2;
const int DIRECTION_UP = 2;
const int DIRECTION_FLOOR = 3;
const int DIRECTION_DOWN = 3;
const int DIRECTION_ALL = 4;

enum CaRoomSearchCriteria {
  JUST_LEFT_AND_RIGHT,
  ALL_DOORS,
  ALL_DOORS_AND_LINKS
};

enum CaRoomCategoryFound {
  NONE_FOUND,
  NEIGHBOUR_FOUND,
  LINK_FOUND,
  CURRENT_ROOM_FOUND
};

enum MapDirection {
  GO_NOWHERE,
  GO_IN,
  GO_OUT,
  GO_UP,
  GO_DOWN,
  GO_LEFT,
  GO_RIGHT,
  GO_WALK_LEFT,
  GO_WALK_RIGHT
};

//
// Forward Declarations
//

class Door;
class Link;
class Room;
class MetaRoom;

//
// Types
//
typedef std::list<int> IntegerCollection;
typedef std::list<int>::iterator IntegerIterator;
typedef std::list<int>::const_iterator ConstantIntegerIterator;
typedef std::set<std::string> StringCollection;
typedef std::set<std::string>::iterator StringIterator;
typedef std::set<std::string>::const_iterator ConstantStringIterator;
typedef std::list<Door *> DoorCollection;
typedef std::list<Door *>::iterator DoorIterator;
typedef std::list<Door *>::const_iterator ConstantDoorIterator;
typedef std::list<Link *> LinkCollection;
typedef std::list<Link *>::iterator LinkIterator;
typedef std::list<Link *>::const_iterator ConstantLinkIterator;

//
// Serialization Helper Functions
//

static void WriteIntegerCollection(CreaturesArchive &ar,
                                   const IntegerCollection &integerCollection);
static void WriteDoorCollection(CreaturesArchive &ar,
                                const DoorCollection &doorCollection);
static void WriteStringCollection(CreaturesArchive &ar,
                                  const StringCollection &stringCollection);
static void ReadIntegerCollection(CreaturesArchive &ar,
                                  IntegerCollection &integerCollection);
static void ReadDoorCollection(CreaturesArchive &ar,
                               DoorCollection &doorCollection);
static void ReadStringCollection(CreaturesArchive &ar,
                                 StringCollection &stringCollection);

class Room : public PersistentObject {
  CREATURES_DECLARE_SERIAL(Room)
public:
  int roomID;
  int metaRoomID;
  Vector2D startFloor;
  Vector2D endFloor;
  Vector2D deltaFloor;
  Vector2D startCeiling;
  Vector2D endCeiling;
  Vector2D deltaCeiling;
  Vector2D centre;
  Vector2D positionMin;
  Vector2D positionMax;
  DoorCollection doorCollection;
  DoorCollection floorCollection;
  DoorCollection ceilingCollection;
  DoorCollection leftCollection;
  DoorCollection rightCollection;
  IntegerCollection neighbourIDCollections[4];
  IntegerCollection neighbourIDCollection;
  int permiability;
  int type;
  float caValues[CA_PROPERTY_COUNT];
  float caOldValues[CA_PROPERTY_COUNT];
  float caOlderValues[CA_PROPERTY_COUNT];
  float caTempValue;
  float caInput;
  float caTotalDoorage;
  float perimeterLength;
  std::string track;
  int leftFloorRoomID;
  int rightFloorRoomID;
  Door *leftNavigableDoor;
  Door *rightNavigableDoor;
  std::list<Link *> linkCollection;

  int visitedOnTock; // new for Sea Monkeys (not serialised)
  float amountToAdd; // temp var for SM CAs

  virtual bool Write(CreaturesArchive &ar) const;
  virtual bool Read(CreaturesArchive &ar);
};

class Map : public PersistentObject {
  CREATURES_DECLARE_SERIAL(Map)
private:
  //
  // Private structures
  //

  struct OverlapInformation {
    Vector2D start;
    Vector2D end;
    bool shared;
  };

  typedef std::list<OverlapInformation> OverlapCollection;
  typedef std::list<OverlapInformation>::iterator OverlapIterator;

public:
  //
  // Public member functions
  //

  static inline int FastFloatToInteger(float f) { return (int)f; }

  Map(void);
  ~Map(void);

  void Reset(void);

  bool SearchForAdjacentRoom(const Vector2D &position, const int direction,
                             int &roomID);

  bool FindSafeAgentLocation(const Vector2D &positionPreferred,
                             const float width, const float height,
                             const int minDoorPermiability,
                             Vector2D &positionSafe);

  bool FindSafeCreatureLocation(const Vector2D &footLeft,
                                const Vector2D &footRight, const float yMin,
                                const int currentDownFoot,
                                const int minDoorPermiability,
                                Vector2D &downFootSafe);

  bool CanPointSeePoint(const Vector2D &start, const Vector2D &end,
                        const int minDoorPermiability, bool &canSee);

  bool IsAgentLocationValidInRoomSystem(const Vector2D &position,
                                        const float width, const float height,
                                        const int minDoorPermiability);

  bool IsAgentLocationValidInRectangle(const Vector2D &position,
                                       const float width, const float height,
                                       const float left, const float right,
                                       const float top, const float bottom);

  bool IsCreatureLocationValidInRoomSystem(const Vector2D &footLeft,
                                           const Vector2D &footRight,
                                           const float yMin,
                                           const int minDoorPermiability);

  bool IsCreatureLocationValidInRectangle(const Vector2D &footLeft,
                                          const Vector2D &footRight,
                                          const float yMin, const float left,
                                          const float right, const float top,
                                          const float bottom);

  bool TestForAgentCollisionInRectangle(
      const Vector2D &position, const float width, const float height,
      const int direction, const float distanceTest, const float left,
      const float right, const float top, const float bottom, bool &collision,
      float &distanceCollision);

  bool TestForCreatureCollisionInRectangle(
      const Vector2D &footLeft, const Vector2D &footRight, const float yMin,
      const int direction, const float distanceTest, const float left,
      const float right, const float top, const float bottom, bool &collision,
      float &distanceCollision);

  bool TestForAgentCollisionInRoomSystem(
      const Vector2D &position, const float width, const float height,
      const int direction, const float distanceTest,
      const int minDoorPermiability, bool &collision, float &distanceCollision);

  bool TestForCreatureCollisionInRoomSystem(
      const Vector2D &footLeft, const Vector2D &footRight, const float yMin,
      const int direction, const float distanceTest,
      const int minDoorPermiability, bool &collision, float &distanceCollision);

  void MoveAgentInsideRectangle(
      const float width, const float height, const bool applyPhysics,
      const float collisionFactor, const float aeroDynamicFactor,
      const float frictionFactor, const float gravity, const float left,
      const float right, const float top, const float bottom,
      Vector2D &position, Vector2D &velocity, bool &collision, int &wall,
      bool &stopped, Vector2D &velocityCollision);

  void MoveCreatureInsideRectangle(
      const Vector2D &footLeft, const Vector2D &footRight, const float yMin,
      const bool applyPhysics, const float collisionFactor,
      const float aeroDynamicFactor, const float gravity, const float left,
      const float right, const float top, const float bottom,
      Vector2D &downFootPosition, Vector2D &velocity, bool &collision,
      int &wall, bool &stopped, int &downFoot, Vector2D &velocityCollision);

  void MoveAgentInsideRoomSystem(
      const float width, const float height, const bool applyPhysics,
      const int minDoorPermiability, const float collisionFactor,
      const float aeroDynamicFactor, const float frictionFactor,
      const float gravity, Vector2D &position, Vector2D &velocity,
      bool &collision, int &wall, bool &stopped, Vector2D &velocityCollision);

  void MoveCreatureInsideRoomSystem(
      const Vector2D &footLeft, const Vector2D &footRight, const float yMin,
      const bool applyPhysics, const int minDoorPermiability,
      const float collisionFactor, const float aeroDynamicFactor,
      const float gravity, Vector2D &downFootPosition, Vector2D &velocity,
      bool &collision, int &wall, bool &stopped, int &downFoot,
      Vector2D &velocityCollision);

  void TestNewUpFootInRectangle(const Vector2D &downFootCurrent,
                                const Vector2D &upFootCurrent,
                                const float xMinCurrent,
                                const float xMaxCurrent, const float left,
                                const float right, const float top,
                                const float bottom, Vector2D &downFootNew,
                                bool &collision, int &wall, bool &footChange);

  void CalculateGradients(const bool fall, const bool right, Door *door,
                          float &gradientDownhill, float &gradientUphill);

  void TestNewUpFootInRoomSystem(
      const Vector2D &downFootCurrent, const Vector2D &upFootPrevious,
      const Vector2D &upFootCurrent, const float xMinCurrent,
      const float xMaxCurrent, const float yMinPrevious,
      const float yMinCurrent, const int minDoorPermiability,
      const bool directionRight, Vector2D &downFootNew, float &gradientDownhill,
      float &gradientUphill, bool &collision, int &wall, bool &footChange,
      bool &fall);

  void ShiftCreatureAlongFloor(const Vector2D &downFoot, const Vector2D &upFoot,
                               const float xMin, const float xMax,
                               const float yMin, const int minDoorPermiability,
                               const float xShift, Vector2D &downFootNew,
                               bool &collision, int &wall, bool &footChange,
                               bool &fall);

  bool GetRoomCentre(const int roomID, Vector2D &centre);

  bool GetMetaRoomTrack(const int metaRoomID, std::string &track);

  bool SetMetaRoomTrack(const int metaRoomID, std::string &track);

  bool SetRoomTrack(const int roomID, std::string &track);

  bool GetRoomTrack(const int roomID, std::string &track);

  bool AddMetaRoom(const int positionX, const int positionY, const int width,
                   const int height, std::string background, int &metaRoomID);

  bool RemoveMetaRoom(const int metaRoomID);

  bool AddRoom(const int metaRoomID, const int xLeft, const int xRight,
               const int yLeftCeiling, const int yRightCeiling,
               const int yLeftFloor, const int yRightFloor, int &roomID);

  bool RemoveRoom(const int roomID);

  int GetMetaRoomMaxID(void);

  bool SetCurrentMetaRoom(const int metaRoomID);

  int GetCurrentMetaRoom(void);

  bool GetMetaRoomDefaultCameraLocation(const int metaRoomID, long &positionX,
                                        long &positionY);

  bool SetMetaRoomDefaultCameraLocation(const int metaRoomID,
                                        const Position &position);

  DoorCollection &GetDoorCollection(void);

  int GetMetaRoomIDCollection(IntegerCollection &metaRoomIDCollection);

  int GetRoomIDCollection(IntegerCollection &roomIDCollection);

  bool GetRoomIDCollectionForMetaRoom(const int metaRoomID,
                                      IntegerCollection &roomIDCollection,
                                      int &roomCount);

  bool IsMetaRoomIDValid(const int metaRoomID);

  bool IsRoomIDValid(const int roomID);

  bool GetMetaRoomIDForPoint(const Vector2D &position, int &metaRoomID);

  bool GetRoomIDForPoint(const Vector2D &position, int &roomID);

  bool SetMapDimensions(const int width, const int height);

  void GetMapDimensions(int &width, int &height);

  bool SetGridSize(const int gridSquareSize);

  int GetGridSize(void);

  bool GetMetaRoomLocation(const int metaRoomID, int &positionX, int &positionY,
                           int &width, int &height);

  bool GetRoomLocation(const int roomID, int &metaRoomID, int &xLeft,
                       int &xRight, int &yLeftCeiling, int &yRightCeiling,
                       int &yLeftFloor, int &yRightFloor);

  bool SetBackgroundCollection(const int metaRoomID,
                               StringCollection &backgroundCollection);

  bool GetBackgroundCollection(const int metaRoomID,
                               StringCollection &backgroundCollection);

  bool AddBackground(const int metaRoomID, std::string background);

  bool RemoveBackground(const int metaRoomID, std::string background);

  bool SetCurrentBackground(const int metaRoomID, std::string background);

  bool GetCurrentBackground(const int metaRoomID, std::string &background);

  bool SetDoorPermiability(const int roomID1, const int roomID2,
                           const int permiability);

  bool GetDoorPermiability(const int roomID1, const int roomID2,
                           int &permiability);

  bool SetRoomType(int roomID, int type);

  bool GetRoomType(int roomID, int &type);

  bool SetRoomProperty(int roomID, int caIndex, float value);

  bool GetRoomProperty(int roomID, int caIndex, float &value);

  bool GetRoomPropertyMinusMyContribution(const AgentHandle &handle,
                                          float &value);

  bool GetRoomPropertyChange(int roomID, int caIndex, float &value);

  void RecalculateAllNavigationalCAs();

  bool SetCARates(int roomType, int caIndex, CARates const &rates);

  bool GetCARates(int roomType, int caIndex, CARates &rates);

  CARates const &GetCARates(int roomType, int caIndex);

  bool IncreaseCAProperty(int roomID, int caIndex, float value);

  bool IncreaseCAInput(int roomID, float value);

  bool WhichDirectionToFollowCA(int currentRoomID, int caIndex,
                                int &mapDirection, bool approachNotRetreat);

  bool GetLinkPermiability(const int roomID1, const int roomID2,
                           int &permiability);

  bool SetLinkPermiability(const int roomID1, const int roomID2,
                           const int permiability);

  void UpdateCurrentCAProperty();

  int GetCAIndex(void);

  CaRoomCategoryFound
  GetRoomIDWithHighestCA(int roomID, int caIndex,
                         CaRoomSearchCriteria includeUpDownOrLinks,
                         int &roomIDHighest);

  CaRoomCategoryFound
  GetRoomIDWithLowestCA(int roomID, int caIndex,
                        CaRoomSearchCriteria includeUpDownOrLinks,
                        int &roomIDLowest);

  template <class T>
  friend CaRoomCategoryFound
  GetRoomIDWithCA(T &t, Map &map, int roomID, int caIndex,
                  CaRoomSearchCriteria includeUpDownOrLinks, int &roomIdBest);

  bool SetIndexBases(const int metaRoomBase, const int roomBase);

  bool IsCANavigable(int index);
  bool IsCASuperNavigable(int index);

  void AlterCAEmission(int roomID, int caIndex, float difference);

  //
  // Serialization
  //

  virtual bool Write(CreaturesArchive &ar) const;
  virtual bool Read(CreaturesArchive &ar);

private:
  void AlterCAEmission(Room *room, int caIndex, float difference, int distance,
                       int roomIDFrom, Link *linkFrom);

  void AlterSuperCAEmission(Room *room, int caIndex, float difference,
                            int distance);

  //
  // Private member functions
  //

  void Initialise(void);
  void Cleanup(void);

  // Updates the CA between either doors or links:
  template <typename D> void UpdateDoorCABetweenRooms(const D &doorOrLink) {
    Room *room1 = myRoomCollection[doorOrLink.parent1];
    Room *room2 = myRoomCollection[doorOrLink.parent2];

    // Diffuses from proper share of temp values and adds them to new values:
    UpdateDoorCA(room1->caTempValue, myCARates[room1->type][myNextCAProperty],
                 doorOrLink.doorage1,

                 room2->caTempValue, myCARates[room2->type][myNextCAProperty],
                 doorOrLink.doorage2,

                 room1->caValues[myNextCAProperty],
                 room2->caValues[myNextCAProperty]);
  }

  template <typename D> void CalculateDoorDoorage(D &doorOrLink) {
    float normPerm = doorOrLink.permiability / 100.0f;
    float doorage = doorOrLink.length * normPerm * normPerm;
    if (doorOrLink.parent1 != -1)
      doorOrLink.doorage1 =
          doorage / myRoomCollection[doorOrLink.parent1]->perimeterLength;
    if (doorOrLink.parent2 != -1)
      doorOrLink.doorage2 =
          doorage / myRoomCollection[doorOrLink.parent2]->perimeterLength;
  }

  static inline int FastFloor(float f) {
    // Note that this only floors non-negative numbers
    return (f < 1.0f) ? 0 : FastFloatToInteger(f - 0.5f);
  }

  static inline float CalculateY(const Vector2D &position,
                                 const Vector2D &start, const Vector2D &delta) {
    _ASSERT(delta.x != 0.0f);
    _ASSERT((position.x >= start.x) && (position.x <= start.x + delta.x));
    return (start.y + (delta.y / delta.x) * (position.x - start.x));
  };

  static inline bool IsPointAboveLine(const Vector2D &position,
                                      const Vector2D &start,
                                      const Vector2D &delta, float &yAbove) {
    register float y = start.y + (delta.y / delta.x) * (position.x - start.x);
    yAbove = y - position.y;
    return (position.y < (y + 0.0001f));
  }

  static inline bool IsPointBelowLine(const Vector2D &position,
                                      const Vector2D &start,
                                      const Vector2D &delta, float &yBelow) {
    register float y = start.y + (delta.y / delta.x) * (position.x - start.x);
    yBelow = position.y - y;
    return (position.y > (y - 0.0001f));
  }

  bool DoesPathIntersectLine(const Vector2D &startPath, const Vector2D &endPath,
                             const Vector2D &deltaPath,
                             const Vector2D &startLine, const Vector2D &endLine,
                             const Vector2D &deltaLine,
                             Vector2D &intersectionPath,
                             Vector2D &intersectionLine);

  bool
  DoesPathCrossOrTouchLine(const Vector2D &startPath, const Vector2D &endPath,
                           const Vector2D &deltaPath, const Vector2D &startLine,
                           const Vector2D &endLine, const Vector2D &deltaLine,
                           Vector2D &intersection);

  void GetLineBoundingBox(const Vector2D &start, const Vector2D &end,
                          Vector2D &positionMin, Vector2D &positionMax);

  bool IsPointOnLine(const Vector2D &position, const Vector2D &start,
                     const Vector2D &end, const Vector2D &delta,
                     const float tolerance);

  bool IsPointBelowCeilingDoors(const Vector2D &position, Room *room);

  bool IsPointAboveFloorDoors(const Vector2D &position, Room *room);

  void GetGridIndexesForPoint(const Vector2D &position, int &squareX,
                              int &squareY);

  bool GetRoomInformationForPoint(const Vector2D &position, int roomIDList[4],
                                  Room *roomList[4], int &roomCount,
                                  int &roomID);

  bool IsPointInsideRoom(const Vector2D &position, Room *room);

  bool IsRegionInsideSameRoom(const Vector2D &positionMin,
                              const Vector2D &positionMax);

  void GetRoomsOverlappingRegion(const int xSquareMin, const int ySquareMin,
                                 const int xSquareMax, const int ySquareMax);

  void CalculateSlideAccelerationAndSlideVelocity(const Vector2D &velocity,
                                                  const Vector2D &slope,
                                                  const Vector2D &gravity,
                                                  Vector2D &slideAcceleration,
                                                  Vector2D &slideVelocity);

  void CalculateCollisionVelocityAndElapsedTime(const Vector2D &velocity,
                                                const Vector2D &acceleration,
                                                const Vector2D &path,
                                                const Vector2D &deltaTravelled,
                                                Vector2D &velocityCollision,
                                                float &timeElapsed);

  int CalculateWallFromDoorAndPath(Door *door, const Vector2D &path);

  void GetReflectionFromVertex(const Vector2D &positionVertex,
                               const Vector2D &v, const bool useEdgeDelta,
                               const Vector2D &edgeDelta,
                               const int minDoorPermiability,
                               Vector2D &vReflection, Door &slidingDoor,
                               bool &spike);

  void DoesPathReachRoomBoundary(Room *room, const Vector2D &start,
                                 const Vector2D &end, const Vector2D &delta,
                                 const int minDoorPermiability, bool &reached,
                                 Vector2D &positionReached, Door *&doorReached,
                                 bool &blocked, bool &throughVertex);

  bool DoesPathReachSurroundingRoomBoundary(
      const Vector2D &start, const Vector2D &end, const Vector2D &delta,
      const int minDoorPermiability, bool &reached, Vector2D &positionReached,
      Door *&doorReached, int &roomIDReached, bool &blocked,
      bool &throughVertex);

  bool ArePointsInRectangle(Vector2D points[4], const int pointCount,
                            const float left, const float right,
                            const float top, const float bottom);

  bool AreLinesValidInRoomSystem(Vector2D starts[4], Vector2D ends[4],
                                 const Vector2D &positionMin,
                                 const Vector2D &positionMax,
                                 const int minDoorPermiability,
                                 const int lineCount);

  void GetCreaturePoints(const Vector2D &footLeft, const Vector2D &footRight,
                         const float yMin, Vector2D starts[3], Vector2D ends[3],
                         Vector2D points[4], Vector2D &positionMin,
                         Vector2D &positionMax);

  bool IsAgentPathBlockedByRectangle(
      const Vector2D &position, const float width, const float height,
      const Vector2D &path, const float left, const float right,
      const float top, const float bottom, Vector2D &positionCollision,
      Vector2D &deltaCollision, Vector2D &deltaWall, int &wall,
      float &minSquaredDistance);

  bool IsCreaturePathBlockedByRectangle(
      const Vector2D &footLeft, const Vector2D &footRight, const float yMin,
      const Vector2D &path, const float left, const float right,
      const float top, const float bottom, Vector2D &positionCollision,
      Vector2D &deltaCollision, Vector2D &deltaWall, int &wall,
      bool &floorCollision, int &foot, float &minSquaredDistance);

  bool IsAgentPathBlockedByRoom(
      Room *room, const Vector2D &pathBoxMin, const Vector2D &pathBoxMax,
      const Vector2D &start1, const Vector2D &end1, const Vector2D &start2,
      const Vector2D &end2, const Vector2D &start3, const Vector2D &end3,
      const Vector2D &edge1Delta, const Vector2D &edge2Delta,
      const Vector2D &path, const Vector2D &pathReversed,
      const int minDoorPermiability, Vector2D &positionCollision,
      Vector2D &deltaCollision, Door *&doorCollision, bool &hitVertex,
      Vector2D &positionVertex, bool &edge, Vector2D &edgeDelta,
      float &minSquaredDistance);

  bool IsCreaturePathBlockedByRoom(
      Room *room, const Vector2D &pathBoxMin, const Vector2D &pathBoxMax,
      const Vector2D &footLeft, const Vector2D &footRight, Vector2D points[4],
      Vector2D starts[3], Vector2D ends[3], Vector2D deltas[3],
      const Vector2D &path, const Vector2D &pathReversed,
      const int minDoorPermiability, Vector2D &positionCollision,
      Vector2D &deltaCollision, Door *&doorCollision, bool &hitVertex,
      Vector2D &positionVertex, bool &floorCollision, int &foot,
      float &minSquaredDistance);

  bool IsAgentPathBlockedByRoomSystem(
      const Vector2D &position, const float width, const float height,
      const Vector2D &path, const int minDoorPermiability,
      Vector2D &positionCollision, Vector2D &deltaCollision,
      Door *&doorCollision, bool &hitVertex, Vector2D &positionVertex,
      bool &edge, Vector2D &edgeDelta, float &minSquaredDistance);

  bool IsCreaturePathBlockedByRoomSystem(
      const Vector2D &footLeft, const Vector2D &footRight, const float yMin,
      const Vector2D &path, const int minDoorPermiability,
      Vector2D &positionCollision, Vector2D &deltaCollision,
      Door *&doorCollision, bool &hitVertex, Vector2D &positionVertex,
      bool &floorCollision, int &foot, float &minSquaredDistance);

  void GetNearbyFloorInformation(const Vector2D &position, Room **floorRooms,
                                 int &floorRoomCount);

  bool IsRegionBlockedByVerticalWall(Room **floorRooms, int floorRoomCount,
                                     const int minDoorPermiability,
                                     const float xMinTest, const float xMaxTest,
                                     const float yMinTest, const float yMaxTest,
                                     const bool right, float &xDisplacement);

  void SnapPointsToFloor(const Vector2D &position1, const Vector2D &position2,
                         Room **floorRooms, const int floorRoomCount,
                         const int minDoorPermiability, const bool right,
                         bool &snapped1, bool &snapped2,
                         Vector2D &positionSnap1, Vector2D &positionSnap2,
                         Door *&door1, Door *&door2, float &distance1,
                         float &distance2);

  void SnapFeetToFloor(const Vector2D &downFoot, const Vector2D &upFoot,
                       Room **floorRooms, const int floorRoomCount,
                       const int minDoorPermiability, const bool right,
                       Vector2D &downFootNew, Door *&door, bool &footChange,
                       bool &fall);

  void CalculateRoomPermiabilityAndDoorage(Room *room);
  void CalculateFloorsForRoom(Room *room);
  void CalculateCeilingsForRoom(Room *room);
  void CalculateLeftWallsForRoom(Room *room);
  void CalculateRightWallsForRoom(Room *room);
  void CalculateNeighbourInformationForRoom(Room *room);

  bool GetNeighbours(const int roomID, const int direction,
                     IntegerCollection &neighbourIDCollection);

  bool GetValidMetaRoomPointer(const int metaRoomID, MetaRoom *&metaRoom);

  bool GetValidRoomPointer(const int roomID, Room *&room);

  void AddRoomToGrid(Room *room);

  void RemoveRoomFromGrid(Room *room);

  void BuildCache(void);

  int GetAlternativeParent(Door *door, const int knownID);

  int GetOverlapInformation(Door *door, const Vector2D &startEdge,
                            const Vector2D &endEdge, const Vector2D &deltaEdge,
                            OverlapInformation overlapInformation[3],
                            int &sharedIndex);

  void AmalgamateDoors(Room *room, Door *oldSharedDoor);

  void AddRoomEdge(IntegerCollection &siblingIDs, Room *room,
                   const Vector2D &start, const Vector2D &end,
                   const int edgeType);

  bool GetDoorBetweenRooms(const int id1, const int id2, Door *&door);

  bool GetLinkBetweenRooms(const int roomID1, const int roomID2, Link *&link);

  std::set<int> const &GetNavigableCAIndices();
  std::set<int> const &GetSuperNavigableCAIndices();

  //
  // Private constants
  //

#define PI 3.14159265435f
#define HALFPI 1.570796327f
#define TOLERANCE 0.1f

  enum {
    DOOR_LEFT_RIGHT = 0,
    DOOR_CEILING_FLOOR = 1,
    MAX_ROOMS = 2000,
    MAX_META_ROOMS = 200
  };

  //
  // Private member variables
  //

  int myRoomCount;
  int myMaxRoomID;
  Room *myRoomCollection[MAX_ROOMS];
  Room *myTempRoomCollection[MAX_ROOMS];
  int myTempRoomCount;
  IntegerCollection myRoomIDCollection;
  int myMetaRoomCount;
  int myMetaRoomMaxID;
  int myMetaRoomID;
  MetaRoom *myMetaRoomCollection[MAX_META_ROOMS];
  IntegerCollection myMetaRoomIDCollection;
  LinkCollection myLinkCollection;
  DoorCollection myConstructedDoorCollection;
  IntegerCollection *myIntegerCollectionGrid;
  DoorCollection myDoorCollection;
  DoorCollection myNavigableDoorCollection; // not used but kept for
                                            // serialisation compatibility
  float myMapWidth;
  float myMapHeight;
  float myGridSquareSize;
  Vector2D myMetaRoomMaxCoordinates;
  int mySquareCountX;
  int mySquareCountY;
  CARates myCARates[ROOM_TYPE_COUNT][CA_PROPERTY_COUNT];
  int myNextCAProperty;
  int myMetaRoomIndexBase;
  int myRoomIndexBase;
  unsigned int myAlreadyProcessedList[MAX_ROOMS];
  unsigned int myAlreadyProcessedTruthValue;
};

class MetaRoom : public PersistentObject {
  CREATURES_DECLARE_SERIAL(MetaRoom)
public:
  int metaRoomID;
  Vector2D positionMin;
  Vector2D positionMax;
  Vector2D positionDefault;
  float width;
  float height;
  IntegerCollection roomIDCollection;
  StringCollection backgroundCollection;
  std::string background;
  std::string track;

  virtual bool Write(CreaturesArchive &ar) const;
  virtual bool Read(CreaturesArchive &ar);
};
class Door : public PersistentObject {
  CREATURES_DECLARE_SERIAL(Door)
public:
  Door(){};
  Door(int doorType_, int permiability_, int parentCount_, int parent1_,
       int parent2_, Vector2D start_, Vector2D end_)
      : doorType(doorType_), permiability(permiability_),
        parentCount(parentCount_), parent1(parent1_), parent2(parent2_),
        start(start_), end(end_) {
    delta = end_ - start_;
    length = delta.Length();
    positionMin.x = start.x;
    positionMax.x = end.x;
    if (start.y < end.y) {
      positionMin.y = start.y;
      positionMax.y = end.y;
    } else {
      positionMin.y = end.y;
      positionMax.y = start.y;
    }
  }
  void SetDoorProperties(int doorType_, int permiability_, int parentCount_,
                         int parent1_, int parent2_, Vector2D start_,
                         Vector2D end_) {
    doorType = doorType_;
    permiability = permiability_;
    parentCount = parentCount_;
    parent1 = parent1_;
    parent2 = parent2_;
    start = start_;
    end = end_;
    delta = end_ - start_;
    length = delta.Length();
    positionMin.x = start.x;
    positionMax.x = end.x;
    if (start.y < end.y) {
      positionMin.y = start.y;
      positionMax.y = end.y;
    } else {
      positionMin.y = end.y;
      positionMax.y = start.y;
    }
  }
  int doorType;
  int permiability;
  int parentCount;
  int parent1;
  int parent2;
  Vector2D start;
  Vector2D end;
  Vector2D delta;
  float length;
  Vector2D positionMin;
  Vector2D positionMax;
  float doorage1;
  float doorage2;

  virtual bool Write(CreaturesArchive &ar) const;
  virtual bool Read(CreaturesArchive &ar);
};

class Link : public PersistentObject {
  CREATURES_DECLARE_SERIAL(Link)
public:
  int parent1;
  int parent2;
  int permiability;
  float length;
  float doorage1;
  float doorage2;

  virtual bool Write(CreaturesArchive &ar) const;
  virtual bool Read(CreaturesArchive &ar);
};

#endif // MAP_H
