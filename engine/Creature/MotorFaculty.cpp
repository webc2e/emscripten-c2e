// MotorFaculty.cpp: implementation of the MotorFaculty class.
//
//////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "MotorFaculty.h"
#include "AgentFacultyInterface.h"
#include "CreatureFacultyInterface.h"
#include "Brain/Brain.h"
#include "SensoryFaculty.h"
#include "LinguisticFaculty.h"
#include "LifeFaculty.h"
#include "../App.h"
#include "Brain/BrainScriptFunctions.h"
#include "../AgentManager.h" // for smell->cat mapping
#include "../Agents/AgentConstants.h"
#include "../Maths.h"
#include "Brain/BrainConstants.h"


/* involuntary actions:
	0 - flinch
	1 - lay egg
	2 - sneeze
	3 - cough
	4 - shiver 
	5 - sleep
	6 - fainting
	7 - (unassigned)
	8 - die
*/



CREATURES_IMPLEMENT_SERIAL(MotorFaculty)






//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------
// Function:    (constructor)
// Class:       MotorFaculty
// Description: 
// ------------------------------------------------------------------------
MotorFaculty::MotorFaculty()
{
	myCurrentFocusOfAttention = -1;

	for	(int i=0; i<NUMINVOL; i++) {				
		myInvoluntaryActions[i].locus = 0.0f;
		myInvoluntaryActions[i].latency = 0;
	}

    myCurrentAction=-1;		// hack to make first action call a script
	myCurrentInvoluntaryAction = -1;					// not doing reflex

	myVoluntaryScriptOverrides.attentionScriptNo = -1;
	myVoluntaryScriptOverrides.decisionScriptNo = -1;
}

// ------------------------------------------------------------------------
// Function:    (destructor)
// Class:       MotorFaculty
// Description: 
// ------------------------------------------------------------------------
MotorFaculty::~MotorFaculty()
{

}






// ------------------------------------------------------------------------
// Function:    Update
// Class:       MotorFaculty
// Description: Updates anything involving the creatures motor control as follows:
//
//				1. Checks whether the creature is conscious.
//
//				2. Sets the IT object based on SensoryFaculty's known agents
//				and the brain's attention lobe winner.
//
//				3. Kicks off an involuntary action if any of the biochemical
//				loci for these are set high enough.
//
//				4. Based on the brain's decision lobe winner, kicks off a 
//				creature script to make the creature approach the object and
//				interact with it.
//
//				5. If the IT object is a creature a special "creature-creature"
//				script is called instead if it exists.
// ------------------------------------------------------------------------
void MotorFaculty::Update()
{
	// shouldn't move at all?
	if (myCreature->Life()->GetWhetherUnconscious() || myCreature->Life()->GetWhetherZombie())
		return;


	AgentHandle oldIt = myCreature->GetItAgent();

	int winningAttentionId = (myVoluntaryScriptOverrides.attentionScriptNo>=0) ?
		myVoluntaryScriptOverrides.attentionScriptNo :
		myCreature->GetBrain()->GetWinningId("attn");
	AgentHandle winningAgent = myCreature->Sensory()->GetKnownAgent(winningAttentionId);

	if (winningAgent!=oldIt) {			// agent who won isn't IT at the moment:
		// if IT agent was involved in the script we might want to stop the script:
		if (!myCreature->IsIntrospective() && winningAttentionId!=myCurrentFocusOfAttention) {
			myCreatureAsAgent->StopAllScripts();
		}

		// check if brain still has signal set
		if(myCreature->GetBrain()->GetNeuronState("visn", winningAttentionId, STATE_VAR) == 0.0f)
		{
			winningAgent = NULLHANDLE;
		}

		// in any case, set the IT object:
        myCreature->SetItAgent(winningAgent);
		if (winningAgent.IsInvalid())
		{
			myCreature->SetIntrospective(true);		// can be overridden by approaching a smell
		}
		myCurrentFocusOfAttention = winningAttentionId;
    }

	AgentHandle newIt = myCreature->GetItAgent();



	// DEBUG command to highlight the IT object in red:
	if (theApp.ShouldHighlightAgentsKnownToCreature() && (newIt.IsValid())) {
		newIt.GetAgentFacultyInterfaceReference().SetWhetherHighlighted(true, 255, 40, 40);
	}



	// When any script is finished (an involuntary action or something
	// which interrupted that) make sure our current involuntary action 
	// is reset to nothing:
	if (myCreatureAsAgent->VmIsRunning()==false)
		myCurrentInvoluntaryAction = -1;
	// Don't start another action if an involuntary action is still in progress
	// (DONE cmd hasn't yet executed in its macro script)
	if	(myCurrentInvoluntaryAction>-1)
		return;



	// Maybe an involuntary action should override any voluntary decision?
	// Method: for each non-latent receptor, pick a random number from 0 to 255. 
	// If the receptor signal is greater than that value, the action is a candidate.
	// Pick the action with the best score. (This method allows lower-signal actions to
	// get a look-in, without earlier ones necessarily getting first choice).
	// Action can then set a randomised latency timer to prevent repeats, 
	// and/or can use its built-in stimulus to deplete the chemical
	// which caused the action (or set chemical directly in script)
	int bestInvoluntaryActionId = -1;
	float strongestSoFar = 0.0f;
    for (int i=0; i<NUMINVOL; i++) {					// scan every involuntary action locus
		InvoluntaryAction& a = myInvoluntaryActions[i];
		if (a.latency==0 &&				// if not latent...
			a.locus>RndFloat() &&		// probability of success depends on signal
			a.locus>strongestSoFar) {	// keep strongest so far
										// (n.b. will not pick up zero locus)
				strongestSoFar = myInvoluntaryActions[i].locus;
				bestInvoluntaryActionId = i;
		} else if(myInvoluntaryActions[i].latency > 0){
			myInvoluntaryActions[i].latency--;						// ...else count down to end of latency
		}
	}

	// If an involuntary action recommends itself do it:
	if	((strongestSoFar > 0)&&	
		 (myCreature->Life()->GetWhetherAlert()) &&			// NO INVOLS DURING SLEEP!! - CAUSES TROUBLE!
		 (bestInvoluntaryActionId != myCurrentInvoluntaryAction)) {		// not already doing that
		myCurrentInvoluntaryAction = bestInvoluntaryActionId;				// start doing it
		Classifier scriptClassifier = myCreatureAsAgent->GetClassifier();
		scriptClassifier.SetEvent(SCRIPTINVOLUNTARY0 + bestInvoluntaryActionId);

		// this creature is OWNR of macro event comes FROM this creature
		// and classifier is also this creature's:
		if (myCreatureAsAgent->ExecuteScriptForClassifier(scriptClassifier,myCreatureAsAgent->GetAsAgentHandle(), INTEGERZERO, INTEGERZERO) == EXECUTE_SCRIPT_STARTED) 
		{// event# is 64 upwards	
			myCurrentAction = -1;							// pretend conscious action is quiescent?????
			myCreature->SetIntrospective(true);
			return;									// if found a script, action has been started
		} 
		else 
		{
			myCurrentInvoluntaryAction = -1;		// can't be doing anything involuntary
		}											// else make a voluntary choice instead
	}


	if (!myCreature->Life()->GetWhetherAlert())
		return;


	int scriptAction = (myVoluntaryScriptOverrides.decisionScriptNo>=0) ?
		myVoluntaryScriptOverrides.decisionScriptNo :
		GetScriptOffsetFromNeuronId(myCreature->GetBrain()->GetWinningId("decn"));


	if (scriptAction!=myCurrentAction || 
		(newIt!=oldIt && !myCreature->IsIntrospective()) ||
		!myCreatureAsAgent->VmIsRunning()) {		// restart script if finished

        // initiate action sequence & store myCurrentAction:
		myCurrentAction = scriptAction;
		myCurrentInvoluntaryAction = -1;	// can't be doing anything involuntary

		// set whether introspective:
		myCreature->SetIntrospective(!DoesThisScriptRequireAnItObject(scriptAction));


		// set classifier of script
		Classifier scriptClassifier = myCreatureAsAgent->GetClassifier();
		scriptClassifier.SetEvent(scriptAction + (newIt.IsCreature() ? 32 : 16));

		// if there's no new IT object but the script we want to run is extraspective
		// and there's no smell for that IT object then stop the VM:
		if (newIt.IsInvalid() && !myCreature->IsIntrospective() && 
			theAgentManager.GetSmellIdFromCategoryId(myCurrentFocusOfAttention)==-1)
		{
			myCreature->ResetAnimationString();
			myCreatureAsAgent->StopAllScripts();
			return;
		}

		// try execute the script but if it's not there make sure you stop the current script anyway:
		if (myCreatureAsAgent->ExecuteScriptForClassifier(scriptClassifier, myCreatureAsAgent->GetAsAgentHandle(), INTEGERZERO, INTEGERZERO) != EXECUTE_SCRIPT_STARTED) 
		{
			// the regular scripts should be executed if there are no creature-creature ones available:
			scriptClassifier.SetEvent(scriptAction + 16);
			if (myCreatureAsAgent->ExecuteScriptForClassifier(scriptClassifier, myCreatureAsAgent->GetAsAgentHandle(), INTEGERZERO, INTEGERZERO) != EXECUTE_SCRIPT_STARTED) {
				myCreature->ResetAnimationString();
				myCreatureAsAgent->StopAllScripts();
				return;
			}
		}
	}
}





// ------------------------------------------------------------------------
// Function:    GetLocusAddress
// Class:       MotorFaculty
// Description: Allows the biochemistry to attach to the involuntary action loci
//				in order to trigger them when needs be.
// Arguments:   int type = 
//              int organ = 
//              int tissue = 
//              int locus = 
// Returns:     float* = 
// ------------------------------------------------------------------------
float* MotorFaculty::GetLocusAddress(int type, int organ, int tissue, int locus) {
	if (organ!=ORGAN_CREATURE)
		return NULL;

	if (tissue==TISSUE_SENSORIMOTOR) {
		if (type==RECEPTOR) {
			if (locus>=LOC_INVOLUNTARY0 && locus<=LOC_INVOLUNTARY7)
				return &myInvoluntaryActions[locus-LOC_INVOLUNTARY0].locus;
		}
		else if (type==EMITTER) {
			if (locus>=LOC_E_INVOLUNTARY0 && locus<=LOC_E_INVOLUNTARY7)
				return &myInvoluntaryActions[locus-LOC_E_INVOLUNTARY0].locus;
		}
	}
	return NULL;
}


// ------------------------------------------------------------------------
// Function:    Write
// Class:       MotorFaculty
// Description: Serialisation.
// Arguments:   CreaturesArchive &archive = 
// Returns:     bool = 
// ------------------------------------------------------------------------
bool MotorFaculty::Write(CreaturesArchive &archive) const {
	base::Write( archive );
	archive << myCurrentFocusOfAttention;
	archive << myCurrentAction;
	archive << myCurrentInvoluntaryAction;

	archive << myVoluntaryScriptOverrides.attentionScriptNo;
	archive << myVoluntaryScriptOverrides.decisionScriptNo;

	for (int i=0; i<NUMINVOL; i++) {
		archive << myInvoluntaryActions[i].latency;
		archive.WriteFloatRefTarget( myInvoluntaryActions[i].locus );
	}

	archive << myActiveFlag;
	return true;
}

// ------------------------------------------------------------------------
// Function:    Read
// Class:       MotorFaculty
// Description: Serialisation.
// Arguments:   CreaturesArchive &archive = 
// Returns:     bool = 
// ------------------------------------------------------------------------
bool MotorFaculty::Read(CreaturesArchive &archive) 
{
	int32 version = archive.GetFileVersion();

	if(version >= 3)
	{

		if(!base::Read( archive ))
			return false;

		archive >> myCurrentFocusOfAttention;
		archive >> myCurrentAction;
		archive >> myCurrentInvoluntaryAction;

		archive >> myVoluntaryScriptOverrides.attentionScriptNo;
		archive >> myVoluntaryScriptOverrides.decisionScriptNo;

		for (int i=0; i<NUMINVOL; i++) {
			archive >> myInvoluntaryActions[i].latency;
			archive.ReadFloatRefTarget( myInvoluntaryActions[i].locus );
		}

		if (version >= 37)
		{
			archive >> myActiveFlag;
		}
	}
	else
	{
		_ASSERT(false);
		return false;
	}
	return true;
}

