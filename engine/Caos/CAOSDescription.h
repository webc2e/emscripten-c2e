// -------------------------------------------------------------------------
// Filename:    CAOSDescription.h
// Class:       CAOSDescription
// Purpose:     Store for CAOS commands, keywords etc...
// Description:
//
//
// Usage:
//
//
// History:
// 30Nov98  BenC	Initial version
// -------------------------------------------------------------------------

#ifndef CAOSDESCRIPTION_H
#define CAOSDESCRIPTION_H

#ifdef _MSC_VER
#pragma warning(disable : 4786 4503)
#endif
#include "../../modules/ModuleAPI.h"
#include "OpSpec.h"
#include <vector>

typedef unsigned short int OpType;

// magic numbers for ops which need special processing during compilation.
enum {
  specialNOTVERY = 0, // a special, non-special code
  specialDOIF,
  specialELIF,
  specialELSE,
  specialENDI,
  specialENUM,
  specialESEE,
  specialETCH,
  specialNEXT,
  specialREPS,
  specialREPE,
  specialLOOP,
  specialUNTL,
  specialEVER,
  specialSUBR,
  specialEPAS,
  specialECON,
};

class C2E_MODULE_API CAOSDescription {
public:
  static const int nFormat;

  CAOSDescription();

  // ---------------------------------------------------------------------
  // Method:      FindCommand
  // Arguments:   name - name of command to search for
  // Returns:     OpSpec for the command or NULL if not found
  // Description: Finds a description for a top-level command
  // ---------------------------------------------------------------------
  const OpSpec *FindCommand(const char *name);

  // ---------------------------------------------------------------------
  // Method:      FindSubCommand
  // Arguments:   name - name of command to search for
  //				parentop - op which preceeded this sub command
  // Returns:     OpSpec for the subcommand or NULL if not found
  // Description: Finds a description for a subcommand
  // ---------------------------------------------------------------------
  const OpSpec *FindSubCommand(const char *name, const OpSpec *parentop = NULL);

  // ---------------------------------------------------------------------
  // Method:      FindVariable
  // Arguments:   name - name of variable
  // Returns:     OpSpec for the var or NULL if not found
  // Description: Finds a description for a var
  // ---------------------------------------------------------------------
  const OpSpec *FindVariable(const char *name);

  // ---------------------------------------------------------------------
  // Method:      FindFloatRV
  // Arguments:   name - name of rvalue
  // Returns:     OpSpec for the rval or NULL if not found
  // Description: Finds a description for a float rval
  // ---------------------------------------------------------------------
  const OpSpec *FindFloatRV(const char *name);

  // ---------------------------------------------------------------------
  // Method:      FindIntegerRV
  // Arguments:   name - name of rvalue
  // Returns:     OpSpec for the rval or NULL if not found
  // Description: Finds a description for a rval
  // ---------------------------------------------------------------------
  const OpSpec *FindIntegerRV(const char *name);

  // ---------------------------------------------------------------------
  // Method:      FindStringRV
  // Arguments:   name - name of string rvalue
  // Returns:     OpSpec for the stringrval or NULL if not found
  // Description: Finds a description for a string rval
  // ---------------------------------------------------------------------
  const OpSpec *FindStringRV(const char *name);

  // ---------------------------------------------------------------------
  // Method:      FindAgentRV
  // Arguments:   name - name of agent rvalue
  // Returns:     OpSpec for the agentrval or NULL if not found
  // Description: Finds a description for a agent rval
  // ---------------------------------------------------------------------
  const OpSpec *FindAgentRV(const char *name);

  // opcodes of special instructions/values
  enum { cmdGOTO = 16, cmdSTOP = 17 };
  enum { varVA00 = 0, varOV00 = 100, varMV00 = 200 };

  // special opcodes for comparison operators
  enum {
    compNULL = 0,
    compEQ = 1,
    compNE = 2,
    compGT = 3,
    compLT = 4,
    compGE = 5,
    compLE = 6
  };

  // special opcodes for logical operators
  enum { logicalNULL = 0, logicalAND = 1, logicalOR = 2 };

  enum {
    argIntegerConstant = 0,
    argFloatConstant = 1,
    argStringConstant = 2,
    argVariable = 3,
    argIntegerRV = 4,
    argStringRV = 5,
    argAgentRV = 6,
    argFloatRV = 7
  };

#ifdef _DEBUG
  bool IsTableValid(int table, int &id);
  void SanityCheck();
#endif // _DEBUG

  const std::vector<OpSpec> &GetTable(int table) const;
  std::vector<OpSpec> &GetTable(int table);

  bool SaveSyntax(const std::string &filename) const;
  bool LoadSyntax(const std::string &filename);

  void PushTable(int tableNumber, OpSpec *start, int count,
                 OpSpec *superCommand = NULL);
  void PushTable(HandlerFunction superHandler, int superTableNumber,
                 const std::string &superCommandName, OpSpec *start, int count);
  void SetEngineVersion(const std::string &engineVersion);
  void SetCategoryText(const std::vector<std::string> &categoryText);
  void SetScriptNames(const std::vector<std::string> &scriptNames);

  void LoadDefaultTables();

  // Help stuff
  void MakeGrandTable(std::vector<OpSpec> &grand_table);
  void StreamHelpAsHTML(std::ostream &out, bool bAlphabetic);
  static void HyperlinkAndAnchorise(std::string &text);
  std::string GetTypeAsText(char param);
  std::string HelpOnOneCommand(std::string command);
  std::string ListAllCommands();
  std::string Apropos(std::string command);
  std::string GetHelpOnOpspec(OpSpec &op);

  std::vector<std::string> &GetScriptNames() { return myScriptNames; }

  // This is a bit of a mess, but just shows the possibilities of
  // modular extensions to C2e.  You can statically register a function
  // which adds commands to the command tables, so making them
  // part of the game.  This could be done from a DLL.

  // To do it properly, more thought should be taken to category
  // names and id numbers (which need to be global in CAOSDescription
  // at the moment).  OpSpec also knows about these global ids (ugh).

  // Also, the subcommand mechanism feels a bit dated now - have a
  // look at NetHandlers to see how silly the extra function layer
  // seems.  It is needed by the ORDR/SWAY/URGE/STIM commands, however.

  // If you do this modular extension, then add a DeclareModuleName
  // function, and print the name of all the modules loaded at
  // the top of the help page - Creatures Engine 2.200 (net module).

  // Also a function to check for duplicate command names would be useful
  // - now they aren't all in one file, so can't be searched for.
  typedef void (*TableRegisterFunction)(CAOSDescription &);
  static int AddTableRegisterFunction(TableRegisterFunction function);

private:
  static std::vector<TableRegisterFunction> &GetTableRegisterFunctions();

  // general purpose search routine
  OpSpec *FindOp(const char *name, int table);

#ifdef _MSC_VER
#pragma warning(disable : 4251)
#endif
  std::vector<std::vector<OpSpec>> myTables;
  std::string myCAOSEngineVersion; // engine that syntax was read from
  std::vector<std::string> myCategoryText;

  std::vector<std::string> myScriptNames;
#ifdef _MSC_VER
#pragma warning(default : 4251)
#endif
};

extern CAOSDescription theCAOSDescription;

#endif // CAOSDESCRIPTION_H
