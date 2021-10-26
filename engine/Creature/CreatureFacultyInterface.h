#ifndef CreatureFacultyInterface_H
#define CreatureFacultyInterface_H

#ifdef _MSC_VER
#pragma warning(disable : 4786 4503)
#endif

#include "../../common/Vector2D.h"
#include "../Agents/AgentHandle.h"


class Brain;
class Biochemistry;
class LifeFaculty;
class LinguisticFaculty;
class SensoryFaculty;
class MotorFaculty;
class ReproductiveFaculty;
class MusicFaculty;

class CreatureFacultyInterface {
public:
  virtual void SetItAgent(AgentHandle &a) = 0;
  virtual AgentHandle GetItAgent() const = 0;
  virtual void SetIntrospective(bool b) = 0;
  virtual bool IsIntrospective() = 0;
  virtual float GetDriveLevel(int i) = 0;
  virtual std::string GetMoniker() = 0;
  virtual std::string GetShortMoniker() = 0;
  virtual std::string GetMotherMoniker() const = 0;
  virtual std::string GetFatherMoniker() const = 0;
  virtual float *GetInvalidLocusAddress() = 0;
  virtual float GetSensoryDistanceFromAgent(AgentHandle a) = 0;

  virtual float *GetLocusAddress(int type, int tissue, int organ,
                                 int locus) = 0;
  virtual bool ReadFromGenome() = 0;

  virtual Brain *GetBrain() = 0;
  virtual Biochemistry *GetBiochemistry() = 0;
  virtual LifeFaculty *Life() = 0;
  virtual LinguisticFaculty *Linguistic() = 0;
  virtual SensoryFaculty *Sensory() = 0;
  virtual MotorFaculty *Motor() = 0;
  virtual ReproductiveFaculty *Reproductive() = 0;
  virtual MusicFaculty *Music() = 0;

  virtual void ResetAnimationString() = 0;
  virtual bool PrepareForAgeing(int age) = 0;
  virtual void SetEyeState(int i) = 0;
  virtual void SetFacialExpression(int headset, int earset = -1) = 0;
  virtual int GetFacialExpression() = 0;
  virtual Vector2D GetNosePosition() = 0;
};
#endif // CreatureFacultyInterface_H
