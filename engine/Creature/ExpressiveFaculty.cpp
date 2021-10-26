// ExpressiveFaculty.cpp: implementation of the ExpressiveFaculty class.
//
//////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "ExpressiveFaculty.h"
#include "Genome.h"
#include "CreatureFacultyInterface.h"
#include "LifeFaculty.h"
#include "../Maths.h"

const int SCRIPT_ILL_FACE = 199;


CREATURES_IMPLEMENT_SERIAL(ExpressiveFaculty)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------
// Function:    (constructor)
// Class:       ExpressiveFaculty
// Description: 
// ------------------------------------------------------------------------
ExpressiveFaculty::ExpressiveFaculty()
{
	for (int e=0; e<EXPR_COUNT; e++) {
		for (int d=0; d<NUMDRIVES; d++) {
			myDriveWeightingsForExpressions[e][d] = 0.0f;
		}
		myExpressionWeightings[e] = 0.0f;
	}
}

// ------------------------------------------------------------------------
// Function:    (destructor)
// Class:       ExpressiveFaculty
// Description: 
// ------------------------------------------------------------------------
ExpressiveFaculty::~ExpressiveFaculty()
{

}


// ------------------------------------------------------------------------
// Function:    ReadFromGenome
// Class:       ExpressiveFaculty
// Description: 
// Arguments:   Genome& g = 
// ------------------------------------------------------------------------
void ExpressiveFaculty::ReadFromGenome(Genome& g) {

	// facial expression genes:
	g.Reset();
	while ((g.GetGeneType(CREATUREGENE,G_EXPRESSION,NUMCREATURESUBTYPES))!=false) {
		int expressionId = g.GetCodonLessThan(EXPR_COUNT);
		g.GetByte();						// spare
		myExpressionWeightings[expressionId] = g.GetFloat();

		// reset drive expression weightings:
		for (int d=0; d<NUMDRIVES; d++)
			myDriveWeightingsForExpressions[expressionId][d] = 0.0f;

		// get drive weightings for this expression:
		int noOfDrivesWeightedUp = 0;
		for (int i=0; i<4; i++) {
			int driveId = g.GetByte();
			float driveWeight = g.GetSignedFloat();

			if (driveId<0 || driveId>=NUMDRIVES)
				continue;
			noOfDrivesWeightedUp++;
			myDriveWeightingsForExpressions[expressionId][driveId] = driveWeight;
		}

		// adjust general weight depending on the number of drives weighted up:
		if (noOfDrivesWeightedUp>0)
			myExpressionWeightings[expressionId] /= noOfDrivesWeightedUp;
	}
}


const int BLINKRATE = 16;			// probability (rnd(n) that a creature
									// will blink this frame

// ------------------------------------------------------------------------
// Function:    Update
// Class:       ExpressiveFaculty
// Description: 
// ------------------------------------------------------------------------
void ExpressiveFaculty::Update() {
	// Blinking, smiling etc:
    int iBlinkRate = (BLINKRATE + 1) - 
		FastFloatToInteger((myCreature->GetDriveLevel(SLEEPINESS) * 16.0f));

    // Set eyes closed if sleeping or blinking or dead, else eyes are open:
	int newEyeState = 
		!(myCreature->Life()->GetWhetherAlert() || myCreature->Life()->GetWhetherZombie()) || 
		!Rnd(iBlinkRate) ? 0 : 1;
    ASSERT(newEyeState==0 || newEyeState==1);
	myCreature->SetEyeState(newEyeState);


	// Change expressions (was updated only slowly):
	int newExpressionId = CalculateExpressionFromDrives();
    if (newExpressionId != myCreature->GetFacialExpression()) {

		// change expressions via Normal:
		newExpressionId = 
			myCreature->GetFacialExpression()!=EXPR_NORMAL ?
            EXPR_NORMAL : newExpressionId;

        myCreature->SetFacialExpression(newExpressionId);
/*		if (newExpressionId==EXPR_ILL)
			myCreature->ExecuteScriptForEvent(SCRIPT_ILL_FACE, myCreature,
			INTEGERZERO, INTEGERZERO);*/
    }
}


// ------------------------------------------------------------------------
// Function:    CalculateExpressionFromDrives
// Class:       ExpressiveFaculty
// Description: 
// Returns:     int = 
// ------------------------------------------------------------------------
int ExpressiveFaculty::CalculateExpressionFromDrives() 
{
	int bestExpressionIdSoFar = EXPR_NORMAL;
	float bestRecommendationSoFar = 0.1f;

//	OutputDebugString("\n");
	for (int e=EXPR_NORMAL; e<EXPR_COUNT; e++) 
	{
		float recommendationForThisExpression = 0.0f;
		for (int d=0; d<NUMDRIVES; d++) 
		{
			if (myDriveWeightingsForExpressions[e][d] >= 0.0f)
			{
				recommendationForThisExpression +=
					myDriveWeightingsForExpressions[e][d] *
					myCreature->GetDriveLevel(d);
			}
			else
			{
				recommendationForThisExpression +=
					-myDriveWeightingsForExpressions[e][d] *
					(1.0f - myCreature->GetDriveLevel(d));
			}
		}
		recommendationForThisExpression *= myExpressionWeightings[e];

//		OutputFormattedDebugString("Expression %d (%s) has %f weight.\n", e, 
//			e==0 ? "normal" : e==1 ? "happy" : e==2 ? "sad" : e==3 ? "angry" : e==4 ? "scared" : e==5 ? "sleepy" : "?", 
//			recommendationForThisExpression);

		if (recommendationForThisExpression>bestRecommendationSoFar) 
		{
			bestRecommendationSoFar = recommendationForThisExpression;
			bestExpressionIdSoFar = e;
		}
	}
    return bestExpressionIdSoFar;
}



// ------------------------------------------------------------------------
// Function:    Write
// Class:       ExpressiveFaculty
// Description: 
// Arguments:   CreaturesArchive &archive = 
// Returns:     bool = 
// ------------------------------------------------------------------------
bool ExpressiveFaculty::Write(CreaturesArchive &archive) const {
	base::Write( archive );
	for (int i=0; i<EXPR_COUNT; i++) {
		archive << myExpressionWeightings[i];
		for (int o=0; o<NUMDRIVES; o++) {
		    archive << myDriveWeightingsForExpressions[i][o];
		}
	}
	return true;
}

// ------------------------------------------------------------------------
// Function:    Read
// Class:       ExpressiveFaculty
// Description: 
// Arguments:   CreaturesArchive &archive = 
// Returns:     bool = 
// ------------------------------------------------------------------------
bool ExpressiveFaculty::Read(CreaturesArchive &archive) 
{
	int32 version = archive.GetFileVersion();

	if(version >= 3)
	{
		if(!base::Read( archive ))
			return false;

		for (int i=0; i<EXPR_COUNT; i++) {
			archive >> myExpressionWeightings[i];
			for (int o=0; o<NUMDRIVES; o++) {
				archive >> myDriveWeightingsForExpressions[i][o];
			}
		}
	}
	else
	{
		_ASSERT(false);
		return false;
	}

	return true;
}

