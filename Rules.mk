# This is a -*- Makefile -*- set of rules
# $Id: Rules.mk,v 1.15 2000/06/28 13:29:45 firving Exp $

# How to make a .o from a .asm
%.o: %.asm
	echo Assembling $<
	$(NASM) $(NASMFLAGS) -o $@ $<

# How to make a .o from a .cpp (.o == release mode no symbols)
%.o: %.cpp
	echo Release build $<
	$(CC) $(CXXFLAGS) $(NO_ASSERT) $(NO_SYMBOLS) $(OPTIMISE) -o $@ -c $<
# How to make a .od from a .cpp (.od == debug mode with symbols)
%.od: %.cpp
	echo Debug build $<
	$(CC) $(CXXFLAGS) $(ASSERT) $(SYMBOLS) $(NO_OPTIMISE) -o $@ -c $<
# How to make a .oh from a .cpp (.oh == release mode with symbols)
%.oh: %.cpp
	echo Hybrid build $<
	$(CC) $(CXXFLAGS) $(PROFILE) $(NO_ASSERT) $(SYMBOLS) $(OPTIMISE) -o $@ -c $<

# How to make a .so from a set of .o (.so == release mode no symbols shared object)
%.so:
	echo Creating Release mode Library $@
	$(CC) $(CXXFLAGS) $(NO_ASSERT) $(NO_SYMBOLS) $(SHAREDLINK) -o $@ $^
# How to make a .do from a set of .od (.do == debug mode with symbols shared object)
%.do:
	echo Creating Debug mode Library $@
	$(CC) $(CXXFLAGS) $(ASSERT) $(SYMBOLS) $(SHAREDLINK) -o $@ $^
# How to make a .ho from a set of .oh (.ho == hybrid mode - just symbols)
%.ho:
	echo Creating Hybrid mode Library $@
	$(CC) $(CXXFLAGS) $(PROFILE) $(NO_ASSERT) $(SYMBOLS) $(SHAREDLINK) -o $@ $^

# How to make a dependency file
%.d: %.cpp
#	echo Creating the dependency information for $<
	$(CCDEPENDENCY) $(CXXFLAGS) $(DEPENDENCYFLAGS) $< | sed -e 's@ /[^ ]*@@g' -e 's@^\(.*\)\.o:@$(dir $(<))\1.d $(dir $(<))\1.o $(dir $(<))\1.od $(dir $(<))\1.oh:@' > $@

# Special low optimise files
# - we need to turn off optimization for certain files, where gcc
# gets stuck and uses lots of memory.  kcc (kai) gets stuck as well.
engine/Message.o engine/App.o engine/Agents/AgentConstants.o engine/Caos/AutoDocumentationTable.o engine/Caos/CAOSDescription.o engine/Creature/Biochemistry/BiochemistryConstants.o engine/Stimulus.o engine/Caos/CAOSTables.o engine/Agents/QuoteFactory.o : %.o : %.cpp
	echo Release build low-optimise $<
	$(CC) $(CXXFLAGS) $(NO_ASSERT) $(NO_SYMBOLS) $(OPTIMISE_LOW) -o $@ -c $<

engine/Message.oh engine/Agents/AgentConstants.oh engine/Caos/AutoDocumentationTable.oh engine/Caos/CAOSDescription.oh engine/Creature/Biochemistry/BiochemistryConstants.oh engine/App.oh engine/Stimulus.oh engine/Caos/CAOSTables.oh engine/Agents/QuoteFactory.oh : %.oh : %.cpp
	echo Hybrid build low-optimise $<
	$(CC) $(CXXFLAGS) $(PROFILE) $(NO_ASSERT) $(NO_SYMBOLS) $(OPTIMISE_LOW) -o $@ -c $<

# We get a spurious warning in this file.  It is a useful warning generally,
# so we only want to turn it off for this file.
# 	engine/Display/ErrorMessageHandler.cpp:126: warning: cannot pass objects of
#  		type `string' through `...'
# This is great as it stops you passing a std::string directly into the
# variable argument parameters (you should always call .c_str()).  However,
# it also warns that the string is the hook for the first agrument, which I 
# don't think is a problem.
# We turn off all warnings, unfortunately, as you can't turn off just this one.
engine/Display/ErrorMessageHandler.od engine/Display/ErrorMessageHandler.oh engine/Display/ErrorMessageHandler.o : CXXFLAGS += -w 

# KCC optimised seg faults in Map::AddRoom
ifeq ($(COMPILER),kcc)
engine/Map/Map.o : OPTIMISE := +K0
endif

# PGCC crashes during compilation of this
ifeq ($(COMPILER), pgcc)
engine/Creature/CreatureAgent.o : engine/Creature/CreatureAgent.cpp
	echo PGCC reduced release build $<
	gcc.plain $(CXXFLAGS) $(NO_ASSERT) $(NO_SYMBOLS) -O3 -o $@ -c $<
endif

